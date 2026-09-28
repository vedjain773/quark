#include "passes/Mem2Reg.hpp"
#include "llvm/IR/CFG.h"
#include "llvm/Transforms/Utils/PromoteMemToReg.h"
#include <algorithm>
#include <cstddef>

using namespace llvm;
using size_t = std::size_t;

PreservedAnalyses Mem2Reg::run(Function &F, FunctionAnalysisManager &) {

    IRBuilder<> Builder = IRBuilder<>(F.getContext());

    numBlocks = 0;
    for (BasicBlock &BB : F) {
        if (!isEntryBlock(&BB) && BB.hasNPredecessors(0))
            continue;

        blockList.insert(&BB);
        blockVecList.push_back(&BB);
        blockIndex.insert({&BB, numBlocks++});
    }

    getPromAllocas();
    performLiveAnalysis();
    initDomSets();

    while (!runIteration())
        ;

    for (BasicBlock *BB : blockList)
        iDoms[BB] = getIDom(BB);

    buildDomTree();
    getDomFrontiers();

    PlacePHINodes();
    renamePass();
    reset();

    return PreservedAnalyses::none();
}

bool Mem2Reg::isEntryBlock(BasicBlock *BB) {
    return BB == &(BB->getParent()->getEntryBlock());
}

void Mem2Reg::initDomSets() {
    for (BasicBlock *BB : blockList) {
        if (isEntryBlock(BB)) {
            BitVector self(numBlocks);
            self.set(blockIndex[BB]);
            domSets[BB] = self;
        } else {
            domSets[BB] = BitVector(numBlocks, true);
        }
    }
}

bool Mem2Reg::runIteration() {
    int changes = 0;

    for (BasicBlock *BB : blockList) {
        if (isEntryBlock(BB))
            continue;

        BitVector initialList = BitVector(numBlocks, true);

        for (BasicBlock *Pred : predecessors(BB)) {
            BitVector predDomSet = domSets[Pred];

            initialList &= predDomSet;
        }

        initialList.set(blockIndex[BB]);

        if (domSets[BB] != initialList) {
            changes++;
            domSets[BB] = initialList;
        }
    }

    return changes == 0;
}

BasicBlock *Mem2Reg::getIDom(BasicBlock *BB) {
    BitVector strictDomSet = domSets[BB];
    strictDomSet.reset(blockIndex[BB]);

    for (unsigned idx : strictDomSet.set_bits()) {
        BasicBlock *cand = blockVecList[idx];

        BitVector missing = strictDomSet;
        missing.reset(domSets[cand]);
        if (!missing.any())
            return cand;
    }

    return nullptr;
}

void Mem2Reg::buildDomTree() {
    for (BasicBlock *BB : blockList) {
        std::set<BasicBlock *> valSet;

        for (BasicBlock *BBInner : blockList) {
            if (iDoms[BBInner] == BB)
                valSet.insert(BBInner);
        }

        domTree[BB] = valSet;
    }
}

void Mem2Reg::getDomFrontiers() {
    for (BasicBlock *BB : blockList) {
        if (isEntryBlock(BB) || !BB->hasNPredecessorsOrMore(2))
            continue;

        for (BasicBlock *Pred : predecessors(BB)) {
            BasicBlock *currentNode = Pred;

            while (currentNode != iDoms[BB]) {
                domFrontier[currentNode].insert(BB);
                currentNode = iDoms[currentNode];
            }
        }
    }
}

std::set<BasicBlock *> Mem2Reg::computeIDF(std::vector<BasicBlock *> &defSites) {
    std::set<BasicBlock *> result;
    std::vector<BasicBlock *> workList = defSites;

    while (!workList.empty()) {
        BasicBlock *B = workList[workList.size() - 1];
        workList.pop_back();

        for (BasicBlock *frontier : domFrontier[B]) {
            if (result.count(frontier) == 0) {
                result.insert(frontier);
                workList.push_back(frontier);
            }
        }
    }

    return result;
}

std::vector<BasicBlock *> Mem2Reg::getDefSites(Value *allocainst) {
    std::vector<BasicBlock *> defsites;
    for (User *U : allocainst->users()) {
        StoreInst *SI = dyn_cast<StoreInst>(U);

        if (SI)
            defsites.push_back(SI->getParent());
    }

    return defsites;
}

std::map<BasicBlock *, StoreInst *> Mem2Reg::getBlockDefs(AllocaInst *allocainst) {
    std::map<BasicBlock *, StoreInst *> blockDefs;

    for (User *U : allocainst->users()) {
        if (StoreInst *SI = dyn_cast<StoreInst>(U))
            blockDefs[SI->getParent()] = SI;
    }

    return blockDefs;
}

void Mem2Reg::PlacePHINodes() {
    for (Value *value : promotableAllocas) {
        std::vector<BasicBlock *> defsites = getDefSites(value);

        std::set<BasicBlock *> idfSites = computeIDF(defsites);

        AllocaInst *allocainst = dyn_cast<AllocaInst>(value);

        for (BasicBlock *idfBlock : idfSites) {
            if (!LiveInMap[idfBlock][valIndex[value]])
                continue;

            int num = pred_size(idfBlock);

            PHINode *phi = PHINode::Create(allocainst->getAllocatedType(), num,
                                           allocainst->getName().str(), &idfBlock->front());

            valPhiPos[value].insert(phi);
        }
    }
}

bool Mem2Reg::isPredOf(BasicBlock *child, BasicBlock *Parent) {
    for (BasicBlock *Pred : predecessors(child)) {
        if (Pred == Parent)
            return true;
    }

    return false;
}

void Mem2Reg::getPromAllocas() {
    for (BasicBlock *BB : blockList) {
        if (!isEntryBlock(BB))
            continue;

        for (Instruction &I : *BB) {
            AllocaInst *allInst = dyn_cast<AllocaInst>(&I);

            if (allInst && isAllocaPromotable(allInst)) {
                promotableAllocas.insert(allInst);
            }
        }
    }

    unsigned i = 0;
    for (Value *val : promotableAllocas) {
        allocas.push_back(val);
        valIndex.insert({val, i++});
    }

    for (BasicBlock *BB : blockList) {
        UseMap[BB] = BitVector(i);
        DefMap[BB] = BitVector(i);
        LiveInMap[BB] = BitVector(i);
        LiveOutMap[BB] = BitVector(i);
    }
}

void Mem2Reg::performLiveAnalysis() {
    for (BasicBlock *BB : blockList) {
        for (auto it = BB->begin(); it != BB->end();) {
            Instruction &I = *it++;

            LoadInst *loadinst = dyn_cast<LoadInst>(&I);
            StoreInst *storeinst = dyn_cast<StoreInst>(&I);

            if (storeinst) {
                Value *val = storeinst->getOperand(1);

                if (!promotableAllocas.count(val))
                    continue;

                DefMap[BB].set(valIndex[val]);
            }

            if (loadinst) {
                Value *val = loadinst->getOperand(0);

                if (!promotableAllocas.count(val))
                    continue;

                if (!DefMap[BB][valIndex[val]])
                    UseMap[BB].set(valIndex[val]);
            }
        }
    }

    bool constant = false;
    while (!constant) {
        constant = true;

        for (BasicBlock *BB : blockVecList) {
            BitVector liveOut = LiveOutMap[BB];
            BitVector liveInSet = UseMap[BB];
            liveInSet |= liveOut.reset(DefMap[BB]);

            if (LiveInMap[BB] != liveInSet) {
                constant = false;
                LiveInMap[BB] = liveInSet;
            }

            BitVector liveOutSet;

            for (BasicBlock *succ : successors(BB)) {
                liveOutSet |= LiveInMap[succ];
            }

            if (liveOutSet != LiveOutMap[BB]) {
                constant = false;
                LiveOutMap[BB] = liveOutSet;
            }
        }
    }
}

std::string Mem2Reg::getNewName(Value *allocainst) {
    auto combine = [](std::string old, int subscript) {
        std::string newString;
        newString = old + "." + std::to_string(subscript);
        return newString;
    };

    int i = counter[allocainst];
    counter[allocainst] += 1;

    return combine(allocainst->getName().str(), i);
}

void Mem2Reg::renamePass() {
    for (BasicBlock *BB : blockList) {
        if (!isEntryBlock(BB))
            continue;

        for (Instruction &I : *BB) {
            AllocaInst *allInst = dyn_cast<AllocaInst>(&I);

            if (allInst && isAllocaPromotable(allInst)) {
                counter[allInst] = 0;
            }
        }
    }

    rename(blockVecList[0]);
}

void Mem2Reg::rename(BasicBlock *BB) {
    for (PHINode &phiNode : BB->phis()) {
        Value *allocainst = nullptr;
        for (auto element : valPhiPos) {
            if (element.second.count(&phiNode))
                allocainst = element.first;
        }

        phiNode.setName(getNewName(allocainst));
        allocaValStack[allocainst].push(&phiNode);
    }

    for (auto it = BB->begin(); it != BB->end();) {
        Instruction &I = *it++;
        LoadInst *loadinst = dyn_cast<LoadInst>(&I);
        StoreInst *storeinst = dyn_cast<StoreInst>(&I);

        if (storeinst) {
            Value *storedVal = storeinst->getOperand(0);
            Value *ptrVal = storeinst->getOperand(1);

            allocaValStack[ptrVal].push(storedVal);
        }

        if (loadinst) {
            Value *ptrVal = loadinst->getOperand(0);

            if (!promotableAllocas.count(ptrVal))
                continue;

            Value *currVal = allocaValStack[ptrVal].top();

            I.replaceAllUsesWith(currVal);
            I.eraseFromParent();
        }
    }

    for (BasicBlock *successor : successors(BB)) {
        for (PHINode &phiNode : successor->phis()) {
            Value *allocainst = nullptr;
            for (auto element : valPhiPos) {
                if (element.second.count(&phiNode))
                    allocainst = element.first;
            }

            if (!allocaValStack.count(allocainst))
                continue;

            Value *val = allocaValStack[allocainst].top();

            bool found = false;

            for (BasicBlock *phiBB : phiNode.blocks()) {
                if (phiBB == BB)
                    found = true;
            }

            if (!found)
                phiNode.addIncoming(val, BB);
        }
    }

    for (BasicBlock *child : domTree[BB])
        rename(child);

    for (auto it = BB->begin(); it != BB->end();) {
        Instruction &I = *it++;
        StoreInst *storeinst = dyn_cast<StoreInst>(&I);
        AllocaInst *allocainst = dyn_cast<AllocaInst>(&I);

        if (allocainst && isAllocaPromotable(allocainst))
            I.eraseFromParent();

        if (storeinst) {
            Value *ptrVal = storeinst->getOperand(1);

            allocaValStack[ptrVal].pop();

            if (promotableAllocas.count(ptrVal))
                I.eraseFromParent();
        }
    }

    for (PHINode &phiNode : BB->phis()) {
        Value *allocainst = nullptr;
        for (auto element : valPhiPos) {
            if (element.second.count(&phiNode))
                allocainst = element.first;
        }

        allocaValStack[allocainst].pop();
    }
}

void Mem2Reg::reset() {
    numBlocks = 0;

    valIndex.clear();
    allocas.clear();
    blockIndex.clear();

    blockList.clear();
    domSets.clear();
    iDoms.clear();
    domTree.clear();
    domFrontier.clear();

    blockVecList.clear();
    valPhiPos.clear();
    allocaValStack.clear();
    counter.clear();
    promotableAllocas.clear();

    UseMap.clear();
    DefMap.clear();
    LiveInMap.clear();
    LiveOutMap.clear();
}
