#include "nodes/Expression.hpp"

llvm::Value *IntExpr::codegen(CodegenVis &codegenvis) {
    llvm::LLVMContext *Cxt = (codegenvis.Context).get();
    return llvm::ConstantInt::get(*Cxt, llvm::APInt(32, Val, true));
}

llvm::Value *CharExpr::codegen(CodegenVis &codegenvis) {
    llvm::LLVMContext *Cxt = (codegenvis.Context).get();
    return llvm::ConstantInt::get(*Cxt, llvm::APInt(8, character));
}

llvm::Value *VarExpr::codegen(CodegenVis &codegenvis) {
    llvm::IRBuilder<> *Bldr = (codegenvis.Builder).get();
    
    llvm::AllocaInst *alloca = codegenvis.lookup(Name);
    llvm::Type *type = nullptr;
    llvm::Value *src = nullptr;

    if (!alloca) {
        llvm::GlobalVariable *global = codegenvis.findGlobal(Name);
        if (global == nullptr) return codegenvis.LogErrorV("Use of undeclared variable: " + Name);
        
        type = global->getValueType();
        src = global;
    } else {
        type = alloca->getAllocatedType();
        src = alloca;
    }

    return Bldr->CreateLoad(type, src, Name.c_str());
}

llvm::Value *VarExpr::emitPtr(CodegenVis &codegenvis) {
    llvm::AllocaInst *alloca = codegenvis.lookup(Name);

    if (!alloca) {
        llvm::GlobalVariable *global = codegenvis.findGlobal(Name);
        if (global == nullptr) return codegenvis.LogErrorV("Use of undeclared variable: " + Name);
        
        return global;    
    }

    return alloca;
}

llvm::Value *DerefExpr::codegen(CodegenVis &codegenvis) {
    llvm::IRBuilder<> *Bldr = (codegenvis.Builder).get();

    llvm::Value *ptr = expr->emitPtr(codegenvis);

    if (isPointerType(expr->infType))
        ptr = expr->codegen(codegenvis);

    llvm::Type *type = nullptr;

    TypeKind *tk = expr->infType->to;
    type = codegenvis.tkToType(tk);

    return Bldr->CreateLoad(type, ptr, "deref");
}

llvm::Value *DerefExpr::emitPtr(CodegenVis &codegenvis) {

    if (isArrayType(expr->infType))
        return expr->emitPtr(codegenvis);

    return expr->codegen(codegenvis);
}

llvm::Value *AddressExpr::codegen(CodegenVis &codegenvis) {
    return expr->emitPtr(codegenvis);
}

llvm::Value *SizeOfExpr::codegen(CodegenVis &codegenvis) {
    llvm::LLVMContext *Cxt = (codegenvis.Context).get();
    return llvm::ConstantInt::get(*Cxt, llvm::APInt(32, argType->size, true));
}

llvm::Value *CastExpr::codegen(CodegenVis &codegenvis) {
    llvm::IRBuilder<> *Bldr = (codegenvis.Builder).get();
    llvm::Value *val = expr->codegen(codegenvis);

    if (from->size < to->size) {
        return Bldr->CreateZExt(val, codegenvis.tkToType(to), "castext");
    } else if (from->size > to->size) {
        return Bldr->CreateTrunc(val, codegenvis.tkToType(to), "casttrunc");
    } else {
        return nullptr;
    }
}

llvm::Value *UnaryExpr::codegen(CodegenVis &codegenvis) {
    llvm::LLVMContext *Cxt = (codegenvis.Context).get();
    llvm::IRBuilder<> *Bldr = (codegenvis.Builder).get();
    llvm::Value *val = Operand->codegen(codegenvis);

    switch (Op) {
        case Operators::MINUS: {
            return Bldr->CreateNeg(val);
        } break;

        case Operators::BANG: {
            llvm::Value *zero = llvm::ConstantInt::get(llvm::Type::getInt32Ty(*Cxt), 0);

            llvm::Value *cmp = Bldr->CreateICmpNE(val, zero, "compne");
            llvm::Value *exor = Bldr->CreateXor(cmp, true, "xor");

            return Bldr->CreateZExt(exor, llvm::Type::getInt32Ty(*Cxt));
        } break;

        default: {
            return val;
        }
    }
}

llvm::Value *BinaryExpr::codegen(CodegenVis &codegenvis) {
    llvm::Value *left = LHS->codegen(codegenvis);
    llvm::Value *right = RHS->codegen(codegenvis);

    if (!left || !right)
        return nullptr;

    if (isPointerType(LHS->infType)) {
        OpConfig opconf = {left, right, Op, LHS->infType->to};
        return codegenvis.handlePtrArith(opconf);
    }

    return codegenvis.handleBinOp({left, right, Op, infType});
}

llvm::Value *BinaryExpr::emitPtr(CodegenVis &codegenvis) {
    llvm::IRBuilder<> *Bldr = (codegenvis.Builder).get();

    llvm::Value *left = LHS->emitPtr(codegenvis);
    llvm::Value *right = RHS->codegen(codegenvis);

    if (isArrayType(LHS->infType))
        return Bldr->CreateInBoundsGEP(codegenvis.tkToType(LHS->infType), left,
                                       {Bldr->getInt32(0), right}, "inbgep");

    return nullptr;
}

llvm::Value *AssignExpr::codegen(CodegenVis &codegenvis) {
    llvm::IRBuilder<> *Bldr = (codegenvis.Builder).get();

    llvm::Value *addr = LHS->emitPtr(codegenvis);

    llvm::Value *exprVal = RHS->codegen(codegenvis);
    Bldr->CreateStore(exprVal, addr);
    return exprVal;
}

llvm::Value *CompAssignExpr::codegen(CodegenVis &codegenvis) {
    llvm::IRBuilder<> *Bldr = (codegenvis.Builder).get();

    std::string binOpStr = std::string(1, getOpStr(Op)[0]);
    Operators binOp = getOp(binOpStr);

    llvm::Value *addr = LHS->emitPtr(codegenvis);

    llvm::Value *left = LHS->codegen(codegenvis);
    llvm::Value *right = RHS->codegen(codegenvis);

    llvm::Value *exprVal = nullptr;

    if (isPointerType(LHS->infType)) {
        OpConfig opconfig = {left, right, binOp, LHS->infType->to};
        exprVal = codegenvis.handlePtrArith(opconfig);
    } else {
        exprVal = codegenvis.handleBinOp({left, right, binOp, infType});
    }

    Bldr->CreateStore(exprVal, addr);
    return exprVal;
}

llvm::Value *EmptyExpr::codegen(CodegenVis &codegenvis) {
    return nullptr;
}

llvm::Value *CallExpr::codegen(CodegenVis &codegenvis) {
    llvm::LLVMContext *Context = (codegenvis.Context).get();
    llvm::IRBuilder<> *Bldr = (codegenvis.Builder).get();
    llvm::Module *module = (codegenvis.Module).get();
    llvm::Function *calleefunc = module->getFunction(callee);

    if (!calleefunc)
        return codegenvis.LogErrorV("Unknown function referenced");

    if (calleefunc->arg_size() != args.size())
        return codegenvis.LogErrorV("Incorrect no. of arguments passed");

    std::vector<llvm::Value *> argsValue;

    for (unsigned i = 0, e = args.size(); i != e; ++i) {
        argsValue.push_back(args[i]->codegen(codegenvis));
        if (!argsValue.back())
            return nullptr;
    }

    if (calleefunc->getReturnType() != llvm::Type::getVoidTy(*Context))
        return Bldr->CreateCall(calleefunc, argsValue, "calltmp");
    else
        return Bldr->CreateCall(calleefunc, argsValue);
}

llvm::Value *MemberAccessExpr::codegen(CodegenVis &codegenvis) {
    llvm::IRBuilder<> *Bldr = (codegenvis.Builder).get();
    llvm::Function *func = Bldr->GetInsertBlock()->getParent();

    unsigned int idx = 0;

    for (size_t i = 0; i < base->infType->fields.size(); i++) {
        if (base->infType->fields[i].name == fName) {
            idx = i;
            break;
        }
    }

    llvm::Value *basePtr = nullptr;

    if (base->isLValue()) {
        basePtr = base->emitPtr(codegenvis);
    } else {
        llvm::AllocaInst *alloca = codegenvis.CreateEntryAlloca(func, "memtmp", base->infType);

        llvm::Value *memtmp = base->codegen(codegenvis);

        Bldr->CreateStore(memtmp, alloca);
        basePtr = alloca;
    }

    llvm::Value *memPtr =
        Bldr->CreateStructGEP(codegenvis.tkToType(base->infType), basePtr, idx, "memacc");

    std::string instName = base->infType->name;
    instName += '.';
    instName += fName;

    return Bldr->CreateLoad(codegenvis.tkToType(infType), memPtr, instName);
}

llvm::Value *MemberAccessExpr::emitPtr(CodegenVis &codegenvis) {
    llvm::IRBuilder<> *Bldr = (codegenvis.Builder).get();

    unsigned int idx = 0;

    for (size_t i = 0; i < base->infType->fields.size(); i++) {
        if (base->infType->fields[i].name == fName) {
            idx = i;
            break;
        }
    }

    llvm::Value *basePtr = base->emitPtr(codegenvis);

    llvm::Value *memPtr =
        Bldr->CreateStructGEP(codegenvis.tkToType(base->infType), basePtr, idx, "memacc");

    return memPtr;
}
