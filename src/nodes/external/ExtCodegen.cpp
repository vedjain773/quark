#include "nodes/ExternalDecl.hpp"
#include "nodes/Expression.hpp"
#include "visitors/CodegenVis.hpp"

void GlobalDecl::codegen(CodegenVis &codegenvis) {
    llvm::Module &mod = *(codegenvis.Module);
    
    llvm::Constant *constant = nullptr;
    llvm::Type *globalType = codegenvis.tkToType(type);

    if (expression == nullptr) constant = llvm::Constant::getNullValue(globalType);     
    else constant = llvm::dyn_cast<llvm::Constant>(expression->codegen(codegenvis));
   
    codegenvis.globals[name] = new llvm::GlobalVariable(mod, globalType, false,
            llvm::GlobalValue::ExternalLinkage, constant, name); 
}
