#include "nodes/Function.hpp"

llvm::Function *Prototype::codegen(CodegenVis &codegenvis) {
    llvm::Module *Mod = (codegenvis.Module).get();
    std::vector<llvm::Type *> typeVec;

    for (size_t i = 0; i < paramList.size(); i++) {
        Parameter *param = (paramList[i]).get();
        typeVec.push_back(codegenvis.tkToType(param->type));
    }

    llvm::FunctionType *functype =
        llvm::FunctionType::get(codegenvis.tkToType(retType), typeVec, false);
    llvm::Function *func =
        llvm::Function::Create(functype, llvm::Function::ExternalLinkage, funcName, Mod);

    unsigned Idx = 0;
    for (auto &Arg : func->args()) {
        Parameter *param = (paramList[Idx++]).get();
        Arg.setName(param->name);
    }

    return func;
}

void FuncDef::codegen(CodegenVis &codegenvis) {
    llvm::LLVMContext *Cxt = (codegenvis.Context).get();
    llvm::Module *Mod = (codegenvis.Module).get();
    llvm::IRBuilder<> *Bldr = (codegenvis.Builder).get();

    llvm::Function *func = Mod->getFunction(prototype->funcName);

    if (!func) func = prototype->codegen(codegenvis);

    llvm::BasicBlock *BB = llvm::BasicBlock::Create(*Cxt, "entry", func);
    Bldr->SetInsertPoint(BB);

    codegenvis.pushScope();

    size_t i = 0;
    Prototype *proto = prototype.get();
    for (auto &Arg : func->args()) {
        Parameter *param = (proto->paramList[i++]).get();

        std::string paramAllocaStr = Arg.getName().str() + ".addr";

        llvm::AllocaInst *alloca = codegenvis.CreateEntryAlloca(func, paramAllocaStr, param->type);

        Bldr->CreateStore(&Arg, alloca);

        codegenvis.insertName(Arg.getName().str(), alloca);
    }

    funcBody->codegen(codegenvis);
    codegenvis.popScope();

    if (prototype->retType == getType("void")) {
        llvm::BasicBlock &lastBlock = func->back();
        if (lastBlock.getTerminator() == nullptr) Bldr->CreateRetVoid();
    }

    llvm::verifyFunction(*func);
}
