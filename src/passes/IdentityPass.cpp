#include "passes/IdentityPass.hpp"

using namespace llvm;

PreservedAnalyses IdentityPass::run(Function &F, FunctionAnalysisManager &) {

    IRBuilder<> Builder = IRBuilder<>(F.getContext());

    for (BasicBlock &BB : F) {
        for (auto it = BB.begin(); it != BB.end();) {
            Instruction &I = *it++;
            Builder.SetInsertPoint(&I);

            if (!I.isBinaryOp(*I.getOpcodeName())) continue;

            auto transform = [&I](ConstantInt *cst, Value *other, int iden) {
                if (cst) {
                    int val = cst->getSExtValue();

                    if (val == iden) {
                        I.replaceAllUsesWith(other);
                        I.eraseFromParent();
                    }
                }
            };

            const char *op = I.getOpcodeName();

            Value *op1 = I.getOperand(0);
            Value *op2 = I.getOperand(1);

            ConstantInt *cst1 = dyn_cast<ConstantInt>(op1);
            ConstantInt *cst2 = dyn_cast<ConstantInt>(op2);

            if (std::string_view(op) == "add") {
                transform(cst1, op2, 0);
                transform(cst2, op1, 0);
            } else if (std::string(op) == "mul") {
                transform(cst1, op2, 1);
                transform(cst2, op1, 1);
            } else if (std::string(op) == "sub") {
                transform(cst2, op1, 0);

                if (cst1) {
                    int val = cst1->getSExtValue();

                    if (val == 0) {
                        Value *val = Builder.CreateNeg(op2);

                        I.replaceAllUsesWith(val);
                        I.eraseFromParent();
                    }
                }
            }
        }
    }

    return PreservedAnalyses::none();
}
