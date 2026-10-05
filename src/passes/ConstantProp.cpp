#include "passes/ConstantProp.hpp"
#include "llvm/IR/CFG.h"

using namespace llvm;

PreservedAnalyses ConstantPropPass::run(Function &F, FunctionAnalysisManager &) {
    IRBuilder<> Builder = IRBuilder<>(F.getContext());

    for (BasicBlock &BB : F) {
        for (auto it = BB.begin(); it != BB.end();) {
            Instruction &I = *it++;

            Builder.SetInsertPoint(&I);

            ICmpInst *icmpinst = dyn_cast<ICmpInst>(&I);
            if (!I.isBinaryOp() && !icmpinst) continue;

            Value *op1 = I.getOperand(0);
            Value *op2 = I.getOperand(1);

            ConstantInt *cst1 = dyn_cast<ConstantInt>(op1);
            ConstantInt *cst2 = dyn_cast<ConstantInt>(op2);

            if (!cst1 || !cst2) continue;

            APInt val1 = cst1->getValue();
            APInt val2 = cst2->getValue();
            APInt res;

            bool folded = true;
            switch (I.getOpcode()) {
                case Instruction::Add:
                    res = val1 + val2;
                    break;

                case Instruction::Sub:
                    res = val1 - val2;
                    break;

                case Instruction::Mul:
                    res = val1 * val2;
                    break;

                case Instruction::SDiv:
                    res = val1.sdiv(val2);
                    break;

                case Instruction::SRem:
                    res = val1.srem(val2);
                    break;

                case Instruction::Shl:
                    res = val1.shl(val2);
                    break;

                default:
                    folded = false;
            }

            if (folded) {
                Type *type = I.getType();
                Value *newValue = ConstantInt::get(type, res);
                I.replaceAllUsesWith(newValue);
                continue;
            }

            folded = true;
            switch (icmpinst->getPredicate()) {
                case CmpInst::ICMP_EQ:
                    res = val1.eq(val2);
                    break;

                case CmpInst::ICMP_NE:
                    res = val1.ne(val2);
                    break;

                case CmpInst::ICMP_SGT:
                    res = val1.sgt(val2);
                    break;

                case CmpInst::ICMP_SGE:
                    res = val1.sge(val2);
                    break;

                case CmpInst::ICMP_SLT:
                    res = val1.slt(val2);
                    break;

                case CmpInst::ICMP_SLE:
                    res = val1.sle(val2);
                    break;

                default:
                    folded = false;
            }

            if (folded) {
                Type *type = I.getType();
                Value *newValue = ConstantInt::get(type, res);
                I.replaceAllUsesWith(newValue);
                continue;
            }
        }
    }

    return PreservedAnalyses::none();
}
