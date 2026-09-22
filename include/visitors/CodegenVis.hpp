#ifndef CODEGENVIS_H
#define CODEGENVIS_H

#include "utils/Scope.hpp"
#include <map>
#include <stack>
#include <memory>

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"

#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Type.h"

#include "llvm/Support/raw_ostream.h"
#include "llvm/TargetParser/Triple.h"

#include "llvm/IR/LegacyPassManager.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/CodeGen.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/TargetParser/Host.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"

enum class Operators;

struct OpConfig {
    llvm::Value *left;
    llvm::Value *right;
    Operators op;
    TypeKind *type;
};

class CodegenVis {
  public:
    std::unique_ptr<llvm::LLVMContext> Context;
    std::unique_ptr<llvm::IRBuilder<>> Builder;
    std::unique_ptr<llvm::Module> Module;

    std::string targetString = "";

    std::map<std::string, llvm::GlobalVariable *> globals;
    std::vector<std::map<std::string, llvm::AllocaInst *>> scopes;
    std::stack<std::pair<llvm::BasicBlock *, llvm::BasicBlock *>> loopStack;

    void initModule(const std::string &fileName);
    void setTarget(const std::string &target);

    llvm::Value *LogErrorV(const std::string &errMsg);
    llvm::Type *tkToType(TypeKind *typek);
    llvm::AllocaInst *CreateEntryAlloca(llvm::Function *function, const std::string &varname,
                                        TypeKind *tk);

    llvm::Value *handlePtrArith(OpConfig &opconfig);
    llvm::Value *handleBinOp(const OpConfig &opconfig);

    void pushScope();
    void popScope();

    void insertName(const std::string &name, llvm::AllocaInst *alloca);
    llvm::AllocaInst *lookup(const std::string &name);
    llvm::GlobalVariable *findGlobal(const std::string &name);

    void emitObj(const std::string &Filename);
};

#endif
