#include "nodes/Program.hpp"
#include "Optimizer.hpp"
#include "visitors/CodegenVis.hpp"
#include "visitors/Visitor.hpp"
#include <cstddef>
#include <iostream>

void Program::accept(Visitor &visitor) {
    visitor.visitProgram(*this);
}

void Program::add(std::unique_ptr<ExternalDecl> edecl) {
    root.push_back(std::move(edecl));
}

void Program::printAST() {
    PrintVisitor printvisitor;
    this->accept(printvisitor);
}

int Program::semAnalyse() {
    SemanticVisitor semvisitor;
    this->accept(semvisitor);
    return semvisitor.numOfErrors;
}

void Program::opt() {
    llvm::Module *mod = (codegenvis.Module).get();

    Optimizer opt;
    opt.registerPasses();
    opt.run(*mod);

    std::error_code EC;
    llvm::raw_fd_ostream outFile("output.ll", EC);

    mod->print(outFile, nullptr);
}

void Program::codegen() {
    codegenvis.initModule(fileName);
    codegenvis.setTarget(target);

    for (size_t i = 0; i < root.size(); i++) {
        root[i]->codegen(codegenvis);
    }

    llvm::Module *mod = (codegenvis.Module).get();

    llvm::verifyModule(*mod, &llvm::errs());
}

void Program::emitIR() {
    std::error_code EC;
    llvm::raw_fd_ostream outFile("output.ll", EC);

    llvm::Module *mod = (codegenvis.Module).get();
    mod->print(outFile, nullptr);
}

void Program::emitObj(const std::string &fileName) {
    codegenvis.emitObj(fileName);
}

void Program::setFileName(const std::string &file_name) {
    fileName = file_name;
}

void Program::setTarget(const std::string &target) {
    this->target = target;
}
