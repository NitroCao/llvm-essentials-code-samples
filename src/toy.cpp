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

int main(int argc, char **argv) {
    static IRBuilder<> builder(context);

    GlobalVariable *globalVar = createGlob(builder, "x");

    Function *fooFunc = createFunc(builder, "foo");
    setFuncArgs(fooFunc, FuncArgs);
    BasicBlock *entry = createBB(fooFunc, "entry");
    builder.SetInsertPoint(entry);

    Value *arg1 = fooFunc->arg_begin();
    Value *constant = builder.getInt32(16);
    Value *val = createArith(builder, arg1, constant);

    Value *val2 = builder.getInt32(100);
    Value *compare = builder.CreateICmpULT(val, val2, "cmp_tmp");
    Value *condition = builder.CreateICmpNE(compare, builder.getInt1(false), "if_cond");

    ValList valList;
    valList.emplace_back(condition);
    valList.emplace_back(arg1);

    BasicBlock *thenBB = createBB(fooFunc, "then");
    BasicBlock *elseBB = createBB(fooFunc, "else");
    BasicBlock *mergeBB = createBB(fooFunc, "if_continue");
    BBList list;
    list.emplace_back(thenBB);
    list.emplace_back(elseBB);
    list.emplace_back(mergeBB);

    Value *v = createIfElse(builder, list, valList);

    builder.CreateRet(v);

    verifyFunction(*fooFunc);

    moduleObj->print(errs(), nullptr);
    return 0;
}
