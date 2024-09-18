add_llvm_component_library(LLVMCGRAExtract
  CGRAExtract.cpp
  edge.cpp
  node.cpp
  graph.cpp

  ADDITIONAL_HEADER_DIRS
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/Transforms/CGRAExtract

  DEPENDS
  intrinsics_gen

  )
