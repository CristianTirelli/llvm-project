#ifndef LLVM_TRANSFORMS_CGRAEXTRACT_GRAPH_H
#define LLVM_TRANSFORMS_CGRAEXTRACT_GRAPH_H

#include <fstream>

#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Value.h"

#include "edge.h"
#include "node.h"
#include "utility.h"
#include <variant>

namespace llvm {

class graph {

  std::vector<node *> nodes;
  std::vector<Edge *> edges;

public:
  graph();
  virtual ~graph();

  /******************* ADD *******************/

  void addNode(node *n);
  void addEdge(Edge *e);

  /******************* GET *******************/
  node *getNode(Instruction *n);
  std::vector<node *> getNodes();
  std::vector<Edge *> getEdges();
  std::vector<Edge *> getAssociateEdges(node *n);

  std::vector<instructionNode *> getInstructionNodes();
  std::vector<liveInNode *> getLiveInNodes();
  std::vector<liveOutNode *> getLiveOutNodes();
  std::vector<constantNode *> getConstantNodes();

  std::vector<node *> getSuccessors(node *n);
  std::vector<node *> getPredecessors(node *n);

  /******************* RMV *******************/
  void removeNode(node *n);
  void removeEdge(node *n);

  /******************* PRINT *******************/
  void printDot(std::string filename);
  void printInstructionEdges(std::string filename);
  void printNodes(std::string filename);
  void printEdges(std::string filename);
};

} // namespace llvm

#endif // LLVM_TRANSFORMS_CGRAEXTRACT_GRAPH_H