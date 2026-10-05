#include "passes/DeadInstPass.hpp"

using namespace llvm;

PreservedAnalyses DeadInstPass::run(Function &F, FunctionAnalysisManager &) {

    IRBuilder<> Builder = IRBuilder<>(F.getContext());

    for (BasicBlock &BB : F) {
        for (auto it = BB.begin(); it != BB.end();) {
            Instruction &I = *it++;
            Builder.SetInsertPoint(&I);

            if (I.mayHaveSideEffects()) continue;

            if (I.isTerminator()) continue;

            if (I.use_empty() && I.isSafeToRemove()) I.eraseFromParent();
        }
    }

    return PreservedAnalyses::none();
}
