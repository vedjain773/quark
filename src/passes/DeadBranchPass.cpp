#include "passes/DeadBranchPass.hpp"
#include "llvm/IR/CFG.h"

using namespace llvm;

PreservedAnalyses DeadBranchPass::run(Function &F, FunctionAnalysisManager &) {
    IRBuilder<> Builder = IRBuilder<>(F.getContext());

    for (BasicBlock &BB : F) {
        Instruction *I = BB.getTerminator();

        if (!I) continue;

        Builder.SetInsertPoint(I);

        BranchInst *brinst = dyn_cast<BranchInst>(I);

        if (brinst && brinst->isConditional()) {
            Value *condn = brinst->getCondition();

            ConstantInt *cint = dyn_cast<ConstantInt>(condn);

            if (cint) {
                int val = cint->getSExtValue();

                BasicBlock *live = nullptr;
                BasicBlock *dead = nullptr;

                if (val == 0) {
                    live = brinst->getSuccessor(1);
                    dead = brinst->getSuccessor(0);
                } else {
                    live = brinst->getSuccessor(0);
                    dead = brinst->getSuccessor(1);
                }

                Builder.CreateBr(live);
                I->eraseFromParent();

                for (BasicBlock *succDead : successors(dead))
                    succDead->removePredecessor(dead);

                dead->removePredecessor(&BB);

                if (dead->hasNPredecessors(0)) dead->eraseFromParent();
            }
        }
    }

    for (auto it = F.begin(); it != F.end();) {
        BasicBlock &BB = *it++;
        if (&BB != &(BB.getParent()->getEntryBlock()) && BB.hasNPredecessors(0))
            BB.eraseFromParent();
    }

    return PreservedAnalyses::none();
}
