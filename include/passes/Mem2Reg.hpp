#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"

#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Support/raw_ostream.h"

#include "llvm/ADT/BitVector.h"
#include "llvm/ADT/DenseMap.h"

#include <iostream>
#include <map>
#include <set>
#include <stack>
#include <string_view>
#include <vector>

namespace llvm {

class Mem2Reg : public PassInfoMixin<Mem2Reg> {
  private:
    DenseMap<Value *, unsigned> valIndex;
    std::vector<Value *> allocas;

    DenseMap<BasicBlock *, unsigned> blockIndex;
    unsigned numBlocks = 0;

    std::set<BasicBlock *> blockList;
    std::map<BasicBlock *, BitVector> domSets;
    std::map<BasicBlock *, BasicBlock *> iDoms;
    std::map<BasicBlock *, std::set<BasicBlock *>> domTree;
    std::map<BasicBlock *, std::set<BasicBlock *>> domFrontier;

    std::vector<BasicBlock *> blockVecList;
    std::map<Value *, std::set<PHINode *>> valPhiPos;
    std::map<Value *, std::stack<Value *>> allocaValStack;
    std::map<Value *, int> counter;

    // Live analysis
    std::map<BasicBlock *, BitVector> UseMap;
    std::map<BasicBlock *, BitVector> DefMap;
    std::map<BasicBlock *, BitVector> LiveInMap;
    std::map<BasicBlock *, BitVector> LiveOutMap;

    std::set<Value *> promotableAllocas;

    // Helper functions
    bool isEntryBlock(BasicBlock *BB);
    bool isPredOf(BasicBlock *child, BasicBlock *Parent);
    void reset();
    std::string getNewName(Value *allocainst);

    void performLiveAnalysis();
    void initDomSets();
    bool runIteration();

    BasicBlock *getIDom(BasicBlock *BB);
    void buildDomTree();

    void getDomFrontiers();

    std::set<BasicBlock *> computeIDF(std::vector<BasicBlock *> &defSites);
    std::vector<BasicBlock *> getDefSites(Value *allocainst);
    std::map<BasicBlock *, StoreInst *> getBlockDefs(AllocaInst *allocainst);

    void PlacePHINodes();
    void renamePass();
    void getPromAllocas();
    void rename(BasicBlock *);

  public:
    PreservedAnalyses run(Function &F, FunctionAnalysisManager &);
};

}; // namespace llvm
