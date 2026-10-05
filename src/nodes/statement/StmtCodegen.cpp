#include "nodes/Statement.hpp"

void ExprStmt::codegen(CodegenVis &codegenvis) {
    expression->codegen(codegenvis);
}

void BlockStmt::codegen(CodegenVis &codegenvis) {
    codegenvis.pushScope();
    for (size_t i = 0; i < statements.size(); i++) {
        statements[i]->codegen(codegenvis);

        if (statements[i]->isTerminator()) break;
    }
    codegenvis.popScope();
}

void IfStmt::codegen(CodegenVis &codegenvis) {
    llvm::IRBuilder<> *Bldr = (codegenvis.Builder).get();
    llvm::LLVMContext *Cxt = (codegenvis.Context).get();
    llvm::Value *cond = condition->codegen(codegenvis);

    if (!cond) {
        return;
    }

    llvm::Value *zero = llvm::ConstantInt::get(llvm::Type::getInt32Ty(*Cxt), 0);

    cond = Bldr->CreateICmpNE(cond, zero, "ifcond");

    llvm::Function *func = Bldr->GetInsertBlock()->getParent();

    llvm::BasicBlock *thenBB = llvm::BasicBlock::Create(*Cxt, "if.then", func);
    llvm::BasicBlock *mergeBB = llvm::BasicBlock::Create(*Cxt, "if.after");
    llvm::BasicBlock *elseBB = llvm::BasicBlock::Create(*Cxt, "if.else");

    if (elseStmt != nullptr) {
        Bldr->CreateCondBr(cond, thenBB, elseBB);
    } else {
        Bldr->CreateCondBr(cond, thenBB, mergeBB);
    }

    Bldr->SetInsertPoint(thenBB);

    codegenvis.pushScope();
    body->codegen(codegenvis);
    codegenvis.popScope();

    if (Bldr->GetInsertBlock()->getTerminator() == nullptr) {
        Bldr->CreateBr(mergeBB);
    }

    if (elseStmt != nullptr) {
        func->insert(func->end(), elseBB);
        Bldr->SetInsertPoint(elseBB);

        elseStmt->codegen(codegenvis);

        if (Bldr->GetInsertBlock()->getTerminator() == nullptr) {
            Bldr->CreateBr(mergeBB);
        }
    }

    func->insert(func->end(), mergeBB);
    Bldr->SetInsertPoint(mergeBB);

    llvm::verifyFunction(*func);
}

void ElseStmt::codegen(CodegenVis &codegenvis) {
    codegenvis.pushScope();
    body->codegen(codegenvis);
    codegenvis.popScope();
}

void WhileStmt::codegen(CodegenVis &codegenvis) {
    llvm::IRBuilder<> *Bldr = (codegenvis.Builder).get();
    llvm::LLVMContext *Cxt = (codegenvis.Context).get();

    llvm::Function *func = Bldr->GetInsertBlock()->getParent();

    llvm::BasicBlock *condBB = llvm::BasicBlock::Create(*Cxt, "while.cond", func);
    llvm::BasicBlock *bodyBB = llvm::BasicBlock::Create(*Cxt, "while.body");
    llvm::BasicBlock *afterBB = llvm::BasicBlock::Create(*Cxt, "while.after");

    codegenvis.loopStack.push(std::make_pair(afterBB, condBB));

    Bldr->CreateBr(condBB);
    Bldr->SetInsertPoint(condBB);

    llvm::Value *cond = condition->codegen(codegenvis);

    if (!cond) {
        return;
    }

    llvm::Value *zero = llvm::ConstantInt::get(llvm::Type::getInt32Ty(*Cxt), 0);
    cond = Bldr->CreateICmpNE(cond, zero, "whilecond");

    Bldr->CreateCondBr(cond, bodyBB, afterBB);

    func->insert(func->end(), bodyBB);
    Bldr->SetInsertPoint(bodyBB);

    codegenvis.pushScope();
    body->codegen(codegenvis);
    codegenvis.popScope();

    if (Bldr->GetInsertBlock()->getTerminator() == nullptr) {
        Bldr->CreateBr(condBB);
    }

    func->insert(func->end(), afterBB);
    Bldr->SetInsertPoint(afterBB);

    codegenvis.loopStack.pop();
    llvm::verifyFunction(*func);
}

void ForStmt::codegen(CodegenVis &codegenvis) {
    llvm::IRBuilder<> *Bldr = (codegenvis.Builder).get();
    llvm::LLVMContext *Cxt = (codegenvis.Context).get();

    llvm::Function *func = Bldr->GetInsertBlock()->getParent();

    llvm::BasicBlock *condBB = llvm::BasicBlock::Create(*Cxt, "for.cond", func);
    llvm::BasicBlock *iterBB = llvm::BasicBlock::Create(*Cxt, "for.iter");
    llvm::BasicBlock *bodyBB = llvm::BasicBlock::Create(*Cxt, "for.body");
    llvm::BasicBlock *afterBB = llvm::BasicBlock::Create(*Cxt, "for.after");

    init->codegen(codegenvis);
    codegenvis.loopStack.push(std::make_pair(afterBB, iterBB));

    Bldr->CreateBr(condBB);
    Bldr->SetInsertPoint(condBB);

    llvm::Value *cond = condn->codegen(codegenvis);

    if (!cond) {
        cond = llvm::ConstantInt::get(llvm::Type::getInt32Ty(*Cxt), 1);
    }

    llvm::Value *zero = llvm::ConstantInt::get(llvm::Type::getInt32Ty(*Cxt), 0);
    cond = Bldr->CreateICmpNE(cond, zero, "forcond");

    Bldr->CreateCondBr(cond, bodyBB, afterBB);

    func->insert(func->end(), bodyBB);

    func->insert(func->end(), iterBB);
    Bldr->SetInsertPoint(iterBB);

    iter->codegen(codegenvis);
    Bldr->CreateBr(condBB);

    Bldr->SetInsertPoint(bodyBB);

    codegenvis.pushScope();
    body->codegen(codegenvis);
    codegenvis.popScope();

    if (Bldr->GetInsertBlock()->getTerminator() == nullptr) {
        Bldr->CreateBr(iterBB);
    } else {
        if (iterBB->hasNPredecessors(0)) iterBB->eraseFromParent();
    }

    func->insert(func->end(), afterBB);
    Bldr->SetInsertPoint(afterBB);

    codegenvis.loopStack.pop();
    llvm::verifyFunction(*func);
}

void BreakStmt::codegen(CodegenVis &codegenvis) {
    llvm::IRBuilder<> *Bldr = (codegenvis.Builder).get();

    if (!codegenvis.loopStack.empty()) {
        auto [afterBB, condBB] = codegenvis.loopStack.top();
        Bldr->CreateBr(afterBB);
    }
}

void ContinueStmt::codegen(CodegenVis &codegenvis) {
    llvm::IRBuilder<> *Bldr = (codegenvis.Builder).get();

    if (!codegenvis.loopStack.empty()) {
        auto [afterBB, condBB] = codegenvis.loopStack.top();
        Bldr->CreateBr(condBB);
    }
}

void ReturnStmt::codegen(CodegenVis &codegenvis) {
    llvm::Value *retVal = retExpr->codegen(codegenvis);
    codegenvis.Builder->CreateRet(retVal);
}

void DeclStmt::codegen(CodegenVis &codegenvis) {
    llvm::IRBuilder<> *Bldr = (codegenvis.Builder).get();
    llvm::Function *func = Bldr->GetInsertBlock()->getParent();

    llvm::AllocaInst *alloca = codegenvis.CreateEntryAlloca(func, name, type);

    if (expression != nullptr) {
        llvm::Value *initVal = expression->codegen(codegenvis);

        Bldr->CreateStore(initVal, alloca);
    }

    codegenvis.insertName(name, alloca);
}

void StructDecl::codegen(CodegenVis &codegenvis) {
    llvm::LLVMContext *Cxt = (codegenvis.Context).get();

    std::vector<llvm::Type *> types;

    for (size_t i = 0; i < fields.size(); i++) {
        types.push_back(codegenvis.tkToType(fields[i]->type));
    }

    llvm::StructType::create(*Cxt, types, tag);
}
