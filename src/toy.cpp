#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <vector>

using namespace llvm;

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

    builder.CreateRet(val);

    verifyFunction(*fooFunc);

    moduleObj->print(errs(), nullptr);
    return 0;
}
