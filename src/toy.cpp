#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <vector>

using namespace llvm;

static LLVMContext context;
static Module *moduleObj = new Module("foo", context);

Function *createFunc(IRBuilder<> &builder, const std::string& name) {
    FunctionType *functionType = FunctionType::get(builder.getInt32Ty(), false);
    return Function::Create(functionType, Function::ExternalLinkage, name, moduleObj);
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

int main(int argc, char **argv) {
    static IRBuilder<> builder(context);

    GlobalVariable *globalVar = createGlob(builder, "x");

    Function *fooFunc = createFunc(builder, "foo");
    BasicBlock *entry = createBB(fooFunc, "entry");
    builder.SetInsertPoint(entry);

    builder.CreateRet(builder.getInt32(0));

    verifyFunction(*fooFunc);

    moduleObj->print(errs(), nullptr);
    return 0;
}
