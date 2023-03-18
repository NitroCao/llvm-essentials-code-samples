#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <vector>

using namespace llvm;

typedef SmallVector<BasicBlock *, 16> BBList;
typedef SmallVector<Value *, 16> ValList;

static LLVMContext context;
static Module *moduleObj = new Module("foo", context);
static std::vector<std::string> FuncArgs;

Function *createFunc(IRBuilder<> &builder, const std::string& name) {
    FuncArgs.emplace_back("a");
    FuncArgs.emplace_back("b");
    std::vector<Type *> integers(FuncArgs.size(), Type::getInt32Ty(context));

    FunctionType *functionType = FunctionType::get(builder.getInt32Ty(), integers, false);
    return Function::Create(functionType, Function::ExternalLinkage, name, moduleObj);
}

void setFuncArgs(Function *func, std::vector<std::string> funcArgs) {
    unsigned idx = 0;
    Function::arg_iterator AI, AE;
    for (AI = func->arg_begin(), AE = func->arg_end(); AI != AE; AI++, idx++) {
        AI->setName(funcArgs[idx]);
    }
}

BasicBlock *createBB(Function *func, const std::string& name) {
    return BasicBlock::Create(context, name, func);
}

GlobalVariable *createGlob(IRBuilder<> &builder, const std::string& name) {
    moduleObj->getOrInsertGlobal(name, builder.getInt32Ty());
    GlobalVariable *globalVar = moduleObj->getNamedGlobal(name);

    globalVar->setLinkage(GlobalValue::CommonLinkage);
    globalVar->setAlignment(MaybeAlign(4));

    return globalVar;
}

Value *createArith(IRBuilder<> &builder, Value *L, Value *R) {
    return builder.CreateMul(L, R, "mul_tmp");
}

Value *createIfElse(IRBuilder<> &builder, BBList list, ValList valList) {
    Value *Condition = valList[0];
    Value *Arg1 = valList[1];
    BasicBlock *ThenBB = list[0];
    BasicBlock *ElseBB = list[1];
    BasicBlock *MergeBB = list[2];
    builder.CreateCondBr(Condition, ThenBB, ElseBB);

    builder.SetInsertPoint(ThenBB);
    Value *thenVal = builder.CreateAdd(Arg1, builder.getInt32(1), "then_add_tmp");
    builder.CreateBr(MergeBB);

    builder.SetInsertPoint(ElseBB);
    Value *elseVal = builder.CreateAdd(Arg1, builder.getInt32(2), "else_add_tmp");
    builder.CreateBr(MergeBB);

    unsigned phiBBSize = list.size() - 1;
    builder.SetInsertPoint(MergeBB);
    PHINode *phi = builder.CreatePHI(Type::getInt32Ty(context), phiBBSize, "if_tmp");
    phi->addIncoming(thenVal, ThenBB);
    phi->addIncoming(elseVal, ElseBB);

    return phi;
}

Value *createLoop(IRBuilder<> &builder, BBList list, ValList vl, Value *startVal, Value *endVal) {
    // preheader basic block initializes the value of variable i to 1.
    BasicBlock *preheaderBB = builder.GetInsertBlock();
    Value *val = vl[0];
    // loopBB basic block is the loop body.
    BasicBlock *loopBB = list[0];
    builder.CreateBr(loopBB);
    // now we're in the loop body.
    builder.SetInsertPoint(loopBB);

    // idxVar stands for the induction variable i.
    PHINode *idxVar = builder.CreatePHI(Type::getInt32Ty(context), 2, "i");
    idxVar->addIncoming(startVal, preheaderBB);
    // create the only statement inside the loop body.
    Value *add = builder.CreateAdd(val, ConstantInt::get(Type::getInt32Ty(context), 5), "add_tmp");
    // stepVal stands for the step value of variable i.
    Value *stepVal = builder.getInt32(1);
    // nextVal is the temporary variable which is incremented by i.
    Value *nextVal = builder.CreateAdd(idxVar, stepVal, "next_val");
    // endCond is the condition expression of the loop.
    Value *endCond = builder.CreateICmpULT(idxVar, endVal, "end_cond");
    endCond = builder.CreateICmpNE(endCond, builder.getInt1(false), "loop_cond");
    BasicBlock *loopEndBB = builder.GetInsertBlock();
    BasicBlock *afterBB = list[1];
    // if the condition expression is true, it'll execute loopBB, or execute afterBB.
    builder.CreateCondBr(endCond, loopBB, afterBB);
    builder.SetInsertPoint(afterBB);
    idxVar->addIncoming(nextVal, loopEndBB);

    return add;
}

int main(int argc, char **argv) {
    static IRBuilder<> builder(context);

    GlobalVariable *globalVar = createGlob(builder, "x");

    Function *fooFunc = createFunc(builder, "foo");
    setFuncArgs(fooFunc, FuncArgs);
    BasicBlock *entry = createBB(fooFunc, "entry");
    builder.SetInsertPoint(entry);

    Function::arg_iterator AI = fooFunc->arg_begin();
    Value *arg1 = AI++;
    Value *arg2 = AI;
    Value *constant = builder.getInt32(16);
    Value *val = createArith(builder, arg1, constant);
    ValList valList;
    valList.emplace_back(arg1);

    BBList list;
    BasicBlock *loopBB = createBB(fooFunc, "loop");
    BasicBlock *afterBB = createBB(fooFunc, "after_loop");
    list.emplace_back(loopBB);
    list.emplace_back(afterBB);

    Value *startVal = builder.getInt32(1);
    Value *result = createLoop(builder, list, valList, startVal, arg2);

    builder.CreateRet(result);

    verifyFunction(*fooFunc);

    moduleObj->print(errs(), nullptr);
    return 0;
}
