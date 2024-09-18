#ifndef LLVM_TRANSFORMS_CGRAEXTRACT_CGRAEXTRACT_H
#define LLVM_TRANSFORMS_CGRAEXTRACT_CGRAEXTRACT_H

#include "llvm/IR/PassManager.h"
#include "llvm/Transforms/Scalar/LoopPassManager.h"
#include "llvm/Analysis/LoopAnalysisManager.h"


namespace llvm {

class CGRAExtractPass : public PassInfoMixin<CGRAExtractPass> {
public:
  PreservedAnalyses run(Loop &L, LoopAnalysisManager &AM, LoopStandardAnalysisResults &AR, LPMUpdater &U);
};

} // namespace llvm

#endif // LLVM_TRANSFORMS_CGRAEXTRACT_CGRAEXTRACT_H