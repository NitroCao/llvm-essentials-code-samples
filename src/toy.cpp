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

int main(int argc, char **argv) {
    static IRBuilder<> builder(context);

    Function *fooFunc = createFunc(builder, "foo");
    verifyFunction(*fooFunc);

    moduleObj->print(errs(), nullptr);
    return 0;
}
