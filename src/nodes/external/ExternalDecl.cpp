#include "nodes/ExternalDecl.hpp"
#include "nodes/Expression.hpp"
#include "visitors/CodegenVis.hpp"
#include "visitors/Visitor.hpp"

GlobalDecl::GlobalDecl(TypeKind *type, const std::string &name, std::unique_ptr<Expression> expr,
        int line, int column)
    : line(line),
      column(column),
      type(type),
      name(name),
      expression(std::move(expr)) {}

void GlobalDecl::accept(Visitor &visitor) {
    visitor.visitGlobalDecl(*this);
}

void GlobalDecl::codegen(CodegenVis &codegenvis) {
    llvm::Module &mod = *(codegenvis.Module);
    
    llvm::Constant *constant = nullptr;
    llvm::Type *globalType = codegenvis.tkToType(type);

    if (expression == nullptr) constant = llvm::Constant::getNullValue(globalType);     
    else constant = llvm::dyn_cast<llvm::Constant>(expression->codegen(codegenvis));
   
    codegenvis.globals[name] = new llvm::GlobalVariable(mod, globalType, false,
            llvm::GlobalValue::ExternalLinkage, constant, name); 
}
