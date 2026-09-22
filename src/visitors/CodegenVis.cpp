#include "visitors/CodegenVis.hpp"
#include "nodes/Expression.hpp"
#include <iostream>

void CodegenVis::initModule(const std::string &fileName) {
    Context = std::make_unique<llvm::LLVMContext>();
    Module = std::make_unique<llvm::Module>(fileName, *Context);
    Builder = std::make_unique<llvm::IRBuilder<>>(*Context);
}

void CodegenVis::setTarget(const std::string &target) {
    targetString = target;
}

llvm::Value *CodegenVis::LogErrorV(const std::string &errMsg) {
    std::cerr << errMsg << "\n";
    return nullptr;
}

llvm::Type *CodegenVis::tkToType(TypeKind *typek) {

    if (typek == nullptr)
        return nullptr;

    TypeKind *intType = getType("int");
    TypeKind *charType = getType("char");
    TypeKind *uint8Type = getType("uint8_t");
    TypeKind *uint16Type = getType("uint16_t");
    TypeKind *voidType = getType("void");

    if (typek == intType)
        return llvm::Type::getInt32Ty(*Context);

    else if (typek == uint8Type || typek == charType)
        return llvm::Type::getInt8Ty(*Context);

    else if (typek == uint16Type)
        return llvm::Type::getInt16Ty(*Context);

    else if (typek == voidType)
        return llvm::Type::getVoidTy(*Context);

    else if (isPointerType(typek))
        return llvm::PointerType::get(*Context, 0);

    else if (isArrayType(typek))
        return llvm::ArrayType::get(tkToType(typek->to), getNumElements(typek));

    else if (isStructType(typek))
        return llvm::StructType::getTypeByName(*Context, typek->name.substr(7));

    else
        return nullptr;
}

llvm::AllocaInst *CodegenVis::CreateEntryAlloca(llvm::Function *function, 
                                                const std::string &varname, TypeKind *tk) {
    llvm::StringRef VarName(varname);
    llvm::IRBuilder<> tmpBldr(&function->getEntryBlock(), function->getEntryBlock().begin());
    return tmpBldr.CreateAlloca(tkToType(tk), nullptr, VarName);
}

llvm::Value *CodegenVis::handlePtrArith(OpConfig &opconfig) {
    auto &[left, right, Op, typek] = opconfig;
    llvm::IRBuilder<> *Bldr = (Builder).get();

    if (Op == Operators::MINUS) {
        right = Bldr->CreateNeg(right);
    }

    return Bldr->CreateGEP(tkToType(typek), left, right, "gep");
}

llvm::Value *CodegenVis::handleBinOp(const OpConfig &opconfig) {
    auto &[left, right, Op, infType] = opconfig;
    llvm::IRBuilder<> *Bldr = (Builder).get();

    switch (Op) {
        case Operators::PLUS: {
            return Bldr->CreateAdd(left, right, "add", false, true);
        } break;

        case Operators::MINUS: {
            return Bldr->CreateSub(left, right, "sub", false, true);
        } break;

        case Operators::MULT: {
            return Bldr->CreateMul(left, right, "mul", false, true);
        } break;

        case Operators::DIVIDE: {
            if (infType->isSigned) return Bldr->CreateSDiv(left, right, "sdiv", false);
            else return Bldr->CreateUDiv(left, right, "udiv", false); 
        } break;

        case Operators::MODULUS: {
            if (infType->isSigned) return Bldr->CreateSRem(left, right, "srem");
            else return Bldr->CreateURem(left, right, "urem");
        } break;

        case Operators::GREATER: {
            llvm::Value *gt = nullptr;

            if (infType->isSigned) gt = Bldr->CreateICmpSGT(left, right, "compSGT");
            else gt = Bldr->CreateICmpUGT(left, right, "compUGT");
            
            return Bldr->CreateZExt(gt, tkToType(infType), "ext");
        } break;

        case Operators::GREATER_EQUALS: {
            llvm::Value *ge = nullptr;

            if (infType->isSigned) ge = Bldr->CreateICmpSGE(left, right, "compSGE");
            else ge = Bldr->CreateICmpUGE(left, right, "compUGE");
            
            return Bldr->CreateZExt(ge, tkToType(infType), "ext");
        } break;

        case Operators::LESS: {
            llvm::Value *lt = nullptr;
            
            if (infType->isSigned) lt = Bldr->CreateICmpSLT(left, right, "compSLT");
            else lt = Bldr->CreateICmpULT(left, right, "compULT");

            return Bldr->CreateZExt(lt, tkToType(infType), "ext");
        } break;

        case Operators::LESS_EQUALS: {
            llvm::Value *le = nullptr;
            
            if (infType->isSigned) le = Bldr->CreateICmpSLE(left, right, "compSLE");
            else le = Bldr->CreateICmpULE(left, right, "compULE");

            return Bldr->CreateZExt(le, tkToType(infType), "ext");
        } break;

        case Operators::EQUALS: {
            llvm::Value *ee = Bldr->CreateICmpEQ(left, right, "compEE");
            return Bldr->CreateZExt(ee, tkToType(infType), "ext");
        } break;

        case Operators::NOT_EQUALS: {
            llvm::Value *ne = Bldr->CreateICmpNE(left, right, "compNE");
            return Bldr->CreateZExt(ne, tkToType(infType), "ext");
        } break;

        case Operators::AND: {
            llvm::Value *booland = Bldr->CreateAnd(left, right, "and");
            return Bldr->CreateZExt(booland, tkToType(infType), "ext");
        } break;

        case Operators::OR: {
            llvm::Value *boolor = Bldr->CreateOr(left, right, "or");
            return Bldr->CreateZExt(boolor, tkToType(infType), "ext");
        } break;

        default:
            return left;
    }
}

void CodegenVis::pushScope() {
    scopes.emplace_back();
}

void CodegenVis::popScope() {
    assert(!scopes.empty() && "Popping empty scope!");
    scopes.pop_back();
}

void CodegenVis::insertName(const std::string &name, llvm::AllocaInst *alloca) {
    scopes[scopes.size() - 1].insert({name, alloca});
}

llvm::AllocaInst *CodegenVis::lookup(const std::string &name) {
    for (int i = scopes.size() - 1; i >= 0; i--) {
        if (scopes[i].count(name)) {
            return scopes[i][name];
        }
    }

    return nullptr;
}

llvm::GlobalVariable *CodegenVis::findGlobal(const std::string &name) {
    if (globals.count(name) != 0) return globals[name];  

    return nullptr;
}

void CodegenVis::emitObj(const std::string &Filename) {
    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();

    std::string TargetTripleStr = targetString.empty() ? llvm::sys::getDefaultTargetTriple()
        : targetString;
    
    llvm::Triple TargetTriple(TargetTripleStr);
    Module->setTargetTriple(TargetTriple);

    std::string Error;
    auto Target = llvm::TargetRegistry::lookupTarget(TargetTriple, Error);

    if (!Target) {
        llvm::errs() << Error;
        return;
    }

    std::string CPU = "generic";
    std::string Features = "";

    llvm::TargetOptions opt{};
    auto TargetMachine = Target->createTargetMachine(
            TargetTriple, CPU, Features, opt, llvm::Reloc::PIC_);

    Module->setDataLayout(TargetMachine->createDataLayout());

    std::error_code EC;
    llvm::raw_fd_ostream dest(Filename, EC, llvm::sys::fs::OF_None);

    if (EC) {
        llvm::errs() << "Could not open file: " << EC.message();
        return;
    }

    llvm::legacy::PassManager pass;

    auto FileType = llvm::CodeGenFileType::ObjectFile;

    if (TargetMachine->addPassesToEmitFile(pass, dest, nullptr, FileType)) {
        llvm::errs() << "TheTargetMachine can't emit a file of this type";
        return;
    }

    pass.run(*Module);
    dest.flush();
}
