#include <llvm/IR/Module.h>
#include <llvm/IR/LLVMContext.h>

using namespace llvm;

static LLVMContext context;
static Module *moduleObj = new Module("foo", context);

int main(int argc, char **argv) {
    moduleObj->print(errs(), nullptr);
    return 0;
}