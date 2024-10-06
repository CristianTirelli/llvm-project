#include "llvm/Transforms/CGRAExtract/CGRAExtract.h"
#include "llvm/Analysis/LoopAnalysisManager.h"
#include "llvm/IR/Metadata.h"
#include "llvm/Transforms/CGRAExtract/graph.h"

// Linux/Unix only directory creation
#include <bits/stdc++.h>
#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>

using namespace llvm;

int nodeId = 0;
int edgeId = 0;
bool canDoIS = true;
int loop_number = 0;
std::string flG;

bool hasPragmaCGRAAcc(Loop *L, StringRef name) {

  MDNode *MD_loop = L->getLoopID();
  if (!MD_loop) {
    return false;
  }

  for (unsigned i = 1, e = MD_loop->getNumOperands(); i < e; ++i) {
    const MDNode *MD_operand = dyn_cast<MDNode>(MD_loop->getOperand(i));
    if (!MD_operand)
      continue;

    const MDString *S = dyn_cast<MDString>(MD_operand->getOperand(0));
    if (!S)
      continue;

    if (name.compare(S->getString()) == 0) {
      return true;
    }
  }

  return false;
}

std::fstream &GotoLine(std::fstream &file, int num) {
  file.seekg(std::ios::beg);
  for (int i = 0; i < num - 1; ++i) {
    file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
  }
  return file;
}

// Not the correct way to do it
// NOTE: DO NOT MODIFY ANYTHING RELATED TO THE NODE ID
int getDistance(node *from, node *to) {

  if (from->getId() < to->getId())
    return 0;
  return 1;
}

void addNode(Instruction *inst, graph *g) {

  instructionNode *n;
  std::string instruction_name;

  // std::fstream myfile;

  // raw_string_ostream rso(name);
  //
  // inst->print(rso);
  //  Assign to each instruction the corrisponding C code, by appending to name
  //  the associate line of code NOTE: When compiling with optimization eg -O3
  //  this relation is not garanted
  //        To enable this feature the code should be compiled with debug
  //        information eg -g
  //  if (DILocation *Loc = inst->getDebugLoc()) {
  //   int Line = Loc->getLine();
  //   StringRef File = Loc->getFilename();
  //   StringRef Dir = Loc->getDirectory();
  //   // bool ImplicitCode = Loc->isImplicitCode();
  //   std::string line;
  //   myfile.open(Dir.str() + "/" + File.str());
  //   GotoLine(myfile, Line);
  //   getline(myfile, line);
  //   // strip from string \t chars
  //   line.erase(std::remove(line.begin(), line.end(), '\t'), line.end());
  //   name = name + "\n" + line;
  //   myfile.close();
  // }

  if (inst->getOpcode() == Instruction::ICmp) {
    if (cast<CmpInst>(inst)->getPredicate() == llvm::CmpInst::FCMP_OEQ ||
        cast<CmpInst>(inst)->getPredicate() == llvm::CmpInst::FCMP_UEQ ||
        cast<CmpInst>(inst)->getPredicate() == llvm::CmpInst::ICMP_EQ) {
      instruction_name = "cmpEQ";
    } else if (cast<CmpInst>(inst)->getPredicate() == llvm::CmpInst::FCMP_ONE ||
               cast<CmpInst>(inst)->getPredicate() == llvm::CmpInst::FCMP_UNE ||
               cast<CmpInst>(inst)->getPredicate() == llvm::CmpInst::ICMP_NE) {
      instruction_name = "cmpNEQ";
    } else if (cast<CmpInst>(inst)->getPredicate() == llvm::CmpInst::FCMP_OGT ||
               cast<CmpInst>(inst)->getPredicate() == llvm::CmpInst::FCMP_UGT ||
               cast<CmpInst>(inst)->getPredicate() == llvm::CmpInst::ICMP_SGT) {
      instruction_name = "cmpSGT";
    } else if (cast<CmpInst>(inst)->getPredicate() == llvm::CmpInst::ICMP_UGT) {
      instruction_name = "cmpUGT";
    } else if (cast<CmpInst>(inst)->getPredicate() == llvm::CmpInst::FCMP_OGE ||
               cast<CmpInst>(inst)->getPredicate() == llvm::CmpInst::FCMP_UGE ||
               cast<CmpInst>(inst)->getPredicate() == llvm::CmpInst::ICMP_SGE) {
      instruction_name = "cmpSGEQ";
    } else if (cast<CmpInst>(inst)->getPredicate() == llvm::CmpInst::ICMP_UGE) {
      instruction_name = "cmpUGEQ";
    } else if (cast<CmpInst>(inst)->getPredicate() == llvm::CmpInst::FCMP_OLT ||
               cast<CmpInst>(inst)->getPredicate() == llvm::CmpInst::FCMP_ULT ||
               cast<CmpInst>(inst)->getPredicate() == llvm::CmpInst::ICMP_SLT) {
      instruction_name = "cmpSLT";
    } else if (cast<CmpInst>(inst)->getPredicate() == llvm::CmpInst::ICMP_ULT) {
      instruction_name = "cmpULT";
    } else if (cast<CmpInst>(inst)->getPredicate() == llvm::CmpInst::FCMP_OLE ||
               cast<CmpInst>(inst)->getPredicate() == llvm::CmpInst::FCMP_ULE ||
               cast<CmpInst>(inst)->getPredicate() == llvm::CmpInst::ICMP_SLE) {
      instruction_name = "cmpSLEQ";
    } else if (cast<CmpInst>(inst)->getPredicate() == llvm::CmpInst::ICMP_ULE) {
      instruction_name = "cmpULEQ";
    }
  } else {
    instruction_name = inst->getOpcodeName();
  }

  n = new instructionNode(nodeId++, dyn_cast<Value>(inst));
  n->setInstructionName(instruction_name);
  g->addNode(dyn_cast<node>(n));
}

void addDependency(Instruction *s_inst, graph *g) {

  for (User *U : s_inst->users()) {
    if (Instruction *d_inst = dyn_cast<Instruction>(U)) {
      node *destination, *source;
      Edge *e;
      destination = g->getNode(d_inst);
      source = g->getNode(s_inst);

      if (source == nullptr or destination == nullptr) {
        // errs() << "Probably some liveine or const value\n\t" << *s_inst <<
        // "\n\t" << *d_inst <<"\n";
        continue;
      }

      int distance = getDistance(source, destination);
      // errs() << "Distance of (" << n2->getId() << ", " << n1->getId() << ")=
      // " << distance << "\n";
      e = new Edge(edgeId++, dyn_cast<node>(source),
                   dyn_cast<node>(destination), distance);
      g->addEdge(e);
    }
  }
}

// Adds livein values to the DFG
// return vector only for debug, can remove later
// TODO: Fix case pointer of pointer
void addLiveIn(BasicBlock *BB, graph *g) {
  std::vector<Value *> livein;
  Function *F = BB->getParent();
  std::vector<instructionNode *> nodes = g->getInstructionNodes();
  liveInNode *source;

  for (instructionNode *n : nodes) {
    Instruction *current_node_inst = dyn_cast<Instruction>(n->getValue());

    for (Use &operand_current_node : current_node_inst->operands()) {
      Value *v = operand_current_node.get();
      if (Instruction *operand_inst =
              dyn_cast<Instruction>(v)) { // operand defined by instruction

        for (BasicBlock &B : *F) { // iterate over all the bbs of the function
          if (&B == BB) {
            continue;
          }

          for (Instruction &I : B) { // and over all the instructions in the bbs
            if (&I == operand_inst &&
                std::find(livein.begin(), livein.end(), &I) == livein.end()) {
              source = new liveInNode(nodeId++, dyn_cast<Value>(&I));
              source->setLiveInName("LiveInFromInst");
              // save livein dep

              for (User *U : dyn_cast<Value>(&I)->users()) {
                if (Instruction *succ = dyn_cast<Instruction>(U)) {

                  node *destination;
                  Edge *e;
                  destination = g->getNode(succ);

                  if (destination == nullptr)
                    continue;

                  e = new Edge(edgeId++, dyn_cast<node>(source), destination,
                               0);
                  g->addEdge(e);

                  if (dyn_cast<instructionNode>(destination)
                          ->getLeftOperand() == nullptr)

                    dyn_cast<instructionNode>(destination)
                        ->setLeftOperand(source);

                  if (dyn_cast<instructionNode>(destination)
                          ->getRightOperand() == nullptr)

                    dyn_cast<instructionNode>(destination)
                        ->setRightOperand(source);
                }
              }
              g->addNode(dyn_cast<node>(source));
              livein.push_back(dyn_cast<Value>(&I));
            }
          }
        }
      } else if (Argument *arg = dyn_cast<Argument>(
                     operand_current_node)) { // operand defined by function
                                              // argument
        Value *varg = dyn_cast<Value>(arg);

        if (std::find(livein.begin(), livein.end(), varg) == livein.end()) {
          source = new liveInNode(nodeId++, varg);
          source->setLiveInName("LiveInFromArg");
          for (User *U : varg->users()) {
            if (Instruction *succ = dyn_cast<Instruction>(U)) {
              node *destination;
              Edge *e;
              destination = g->getNode(succ);
              if (destination == nullptr)
                continue;

              e = new Edge(edgeId++, dyn_cast<node>(source), destination, 0);
              g->addEdge(e);
              if (dyn_cast<instructionNode>(destination)->getLeftOperand() ==
                  nullptr)

                dyn_cast<instructionNode>(destination)->setLeftOperand(source);

              if (dyn_cast<instructionNode>(destination)->getRightOperand() ==
                  nullptr)

                dyn_cast<instructionNode>(destination)->setRightOperand(source);
            }
          }

          g->addNode(dyn_cast<node>(source));

          livein.push_back(varg);
        }
        // stripPointerCasts takes care of bitcast function so I can forget
        // about the isPointerTy
      } else if (GlobalVariable *gbvar = dyn_cast<GlobalVariable>(
                     operand_current_node
                         ->stripPointerCasts())) { // operand defined by global
                                                   // variable
        // errs() << "gvar\n";
        // errs() << *operand_current_node << "\n";
        Value *varg = dyn_cast<Value>(gbvar);
        if (std::find(livein.begin(), livein.end(), varg) == livein.end()) {
          source = new liveInNode(nodeId++, varg);
          source->setLiveInName("LiveInFromGVar");
          bool has_reference = false;
          for (User *U : varg->users()) {
            if (Instruction *succ = dyn_cast<Instruction>(U)) {
              has_reference = true;
              node *destination;
              Edge *e;
              destination = g->getNode(succ);
              if (destination == nullptr)
                continue;
              if (dyn_cast<instructionNode>(destination)->getLeftOperand() ==
                  nullptr)

                dyn_cast<instructionNode>(destination)->setLeftOperand(source);

              if (dyn_cast<instructionNode>(destination)->getRightOperand() ==
                  nullptr)

                dyn_cast<instructionNode>(destination)->setRightOperand(source);
              e = new Edge(edgeId++, dyn_cast<node>(source), destination, 0);
              g->addEdge(e);
            }
          }
          if (has_reference) {
            g->addNode(dyn_cast<node>(source));
          }

          livein.push_back(varg);
        }
      }
    }
  }
}

// Adds liveout values to the DFG
// return vector only for debug, can remove later
void addLiveOut(BasicBlock *BB, graph *g) {

  std::vector<Instruction *> liveout;
  std::vector<instructionNode *> nodes = g->getInstructionNodes();
  liveOutNode *destination;
  Function *F = BB->getParent();
  Edge *e;

  for (instructionNode *n : nodes) {
    Instruction *curr = dyn_cast<Instruction>(n->getValue());
    for (User *U : curr->users()) {
      if (Instruction *succ = dyn_cast<Instruction>(U)) {
        if (succ->getParent() == BB) {
          continue;
        }
        if (std::find(liveout.begin(), liveout.end(), curr) == liveout.end()) {
          destination = new liveOutNode(nodeId++, dyn_cast<Value>(succ));
          destination->setLiveOutName("LiveOut");
          e = new Edge(edgeId++, n, dyn_cast<node>(destination), 0);
          // errs() << *succ << " liveout di " << *curr << '\n';
          g->addNode(dyn_cast<node>(destination));
          liveout.push_back(succ);
          g->addEdge(e);
        }
      }
    }
  }
}

// Save operands assignment of the IR
// They are used by SAT-MapIt to generate the correct instruction
void assignInstructionOperands(graph *g) {

  std::vector<instructionNode *> nodes = g->getInstructionNodes();
  for (instructionNode *n : nodes) {
    Instruction *curr = dyn_cast<Instruction>(n->getValue());
    int n_ops = curr->getNumOperands();
    if (n_ops > 3) {
      errs() << "More than 3 operands for inst " << *curr
             << "\nInstruction Selection will be disabled!";
      canDoIS = false;
      continue;
    } else if (n_ops == 3 && curr->getOpcode() == Instruction::Select) {

      SelectInst *curr_sel = dyn_cast<SelectInst>(curr);
      node *tmp;
      n->setPredicateOperand(
          g->getNode(dyn_cast<Instruction>(curr_sel->getCondition())));
      // I don't care if tmp is a nullptr since I'll handle this case in the
      // printNodes function
      tmp = g->getNode(dyn_cast<Instruction>(curr_sel->getTrueValue()));
      n->setLeftOperand(tmp);
      tmp = g->getNode(dyn_cast<Instruction>(curr_sel->getFalseValue()));
      n->setRightOperand(tmp);

    } else if (n_ops == 3 && curr->getOpcode() == Instruction::GetElementPtr) {
      // TODO: check this
      node *tmp;
      tmp = g->getNode(dyn_cast<Instruction>(curr->getOperand(n_ops - 1)));
      n->setRightOperand(tmp);

    } else if (n_ops == 2) {
      n->setLeftOperand(g->getNode(dyn_cast<Instruction>(curr->getOperand(0))));
      n->setRightOperand(
          g->getNode(dyn_cast<Instruction>(curr->getOperand(1))));

    } else if (n_ops == 1) {
      // errs() << "1 operand inst " << n->getName() << "\n";
      n->setLeftOperand(g->getNode(dyn_cast<Instruction>(curr->getOperand(0))));

    } else if (curr->getOpcode() == Instruction::Br) {
      BranchInst *cbr = dyn_cast<BranchInst>(curr);
      if (cbr->isConditional()) {
        node *tmp;
        tmp = g->getNode(dyn_cast<Instruction>(cbr->getCondition()));
        n->setRightOperand(tmp);
      }
    } else {
      errs() << "No assignment for instuction" << *curr << "\n";
      continue;
    }
  }
  // Constant and LiveIn are assigned on creation
}

void addConstants(std::vector<BasicBlock *> bbs, graph *g) {

  Instruction *inst;
  for (int i = 0; i < (int)bbs.size(); i++) {
    for (BasicBlock::iterator tmp_inst = bbs[i]->begin();
         tmp_inst != bbs[i]->end(); ++tmp_inst) {

      inst = &(*tmp_inst);
      if (inst->getOpcode() == Instruction::ICmp) {
        // check it only has one successor
        node *n = g->getNode(inst);
        if (g->getSuccessors(n).size() == 1) {
          // check successor is an instruction node of g
          if (instructionNode *next_node =
                  dyn_cast<instructionNode>(g->getSuccessors(n)[0])) {
            // check casting to llvm_instruction
            if (Instruction *next_inst =
                    dyn_cast<Instruction>(next_node->getValue())) {
              // check it is a branch instruction
              if (isa<BranchInst>(next_inst)) {
                // add constant as livein value
                for (int i = 0; i < (int)inst->getNumOperands(); i++) {
                  if (ConstantInt *CI =
                          dyn_cast<ConstantInt>(inst->getOperand(i))) {
                    liveInNode *source;
                    instructionNode *destination;
                    Edge *e;
                    source = new liveInNode(nodeId++, nullptr);
                    source->setLiveInName("LiveInForBr");
                    destination = dyn_cast<instructionNode>(g->getNode(inst));
                    e = new Edge(edgeId++, dyn_cast<node>(source),
                                 dyn_cast<node>(destination), 0);

                    destination->setRightOperand(dyn_cast<node>(source));
                    g->addNode(dyn_cast<node>(source));
                    g->addEdge(e);
                  }
                }
              } else {
                for (int i = 0; i < (int)inst->getNumOperands(); i++) {
                  if (ConstantInt *CI =
                          dyn_cast<ConstantInt>(inst->getOperand(i))) {
                    if ((int)CI->getSExtValue() > 4096 ||
                        (int)CI->getSExtValue() < -4097) {
                      // if the constant is too big it cannot be put as in
                      // the immediate fields of the CGRA_WORD, so we put it
                      // in a register (like a livein var)
                      liveInNode *source;
                      instructionNode *destination;
                      Edge *e;
                      source = new liveInNode(nodeId++, nullptr);
                      source->setLiveInName("LiveInForInst");
                      destination = dyn_cast<instructionNode>(g->getNode(inst));
                      e = new Edge(edgeId++, dyn_cast<node>(source),
                                   dyn_cast<node>(destination), 0);
                      if (i > 0) {
                        destination->setRightOperand(source);
                      } else {
                        destination->setLeftOperand(source);
                      }
                      g->addNode(dyn_cast<node>(source));
                      g->addEdge(e);
                    } else {
                      // can be immediate value
                      constantNode *source;
                      instructionNode *destination;
                      Edge *e;

                      source =
                          new constantNode(nodeId++, (int)CI->getSExtValue());
                      g->addNode(dyn_cast<node>(source));
                      destination = dyn_cast<instructionNode>(g->getNode(inst));
                      e = new Edge(edgeId++, dyn_cast<node>(source),
                                   dyn_cast<node>(destination), 0);
                      if (i > 0) {
                        destination->setRightOperand(source);
                      } else {
                        destination->setLeftOperand(source);
                      }
                      g->addEdge(e);
                    }
                  }
                }
              }
            }
          }
        } else {
          if (canDoIS) {
            llvm::errs() << "Icmp with multiple successors is not supported, "
                            "by the EPFL backend!\n";
            llvm::errs() << "Instruction Selection will be disabled.";
            canDoIS = false;
          }
        }
      } else {

        if (inst->getOpcode() != Instruction::ICmp && !isa<BranchInst>(inst)) {
          for (int i = 0; i < (int)inst->getNumOperands(); i++) {
            if (ConstantInt *CI = dyn_cast<ConstantInt>(inst->getOperand(i))) {
              if (inst->getOpcode() == Instruction::GetElementPtr) {
                if (i < (int)inst->getNumOperands() - 1) {
                  continue;
                }
              }
              if ((int)CI->getSExtValue() > 4096 ||
                  (int)CI->getSExtValue() < -4097) {
                // if the constant is too big it cannot be put as in
                // the immediate fields of the CGRA_WORD, so we put it
                // in a register (like a livein var)
                liveInNode *source;
                instructionNode *destination;
                Edge *e;
                source = new liveInNode(nodeId++, nullptr);
                source->setLiveInName("LiveInForInst");
                destination = dyn_cast<instructionNode>(g->getNode(inst));
                e = new Edge(edgeId++, dyn_cast<node>(source),
                             dyn_cast<node>(destination), 0);
                if (i > 0) {
                  destination->setRightOperand(source);
                } else {
                  destination->setLeftOperand(source);
                }
                g->addNode(dyn_cast<node>(source));
                g->addEdge(e);
              } else {
                // can be immediate value
                constantNode *source;
                instructionNode *destination;
                Edge *e;

                source = new constantNode(nodeId++, (int)CI->getSExtValue());
                g->addNode(dyn_cast<node>(source));
                destination = dyn_cast<instructionNode>(g->getNode(inst));
                e = new Edge(edgeId++, dyn_cast<node>(source),
                             dyn_cast<node>(destination), 0);
                if (i > 0) {
                  destination->setRightOperand(source);
                } else {
                  destination->setLeftOperand(source);
                }
                g->addEdge(e);
              }
            }
          }
        }
      }
    }
  }
}

// Check what kind of br is assigned to node n
bool isLess(node *n) {
  if (instructionNode *inst_node = dyn_cast<instructionNode>(n)) {
    Instruction *inst = dyn_cast<Instruction>(inst_node->getValue());

    if (cast<CmpInst>(inst)->getPredicate() == llvm::CmpInst::ICMP_ULT ||
        cast<CmpInst>(inst)->getPredicate() == llvm::CmpInst::FCMP_OLT ||
        cast<CmpInst>(inst)->getPredicate() == llvm::CmpInst::FCMP_ULT ||
        cast<CmpInst>(inst)->getPredicate() == llvm::CmpInst::ICMP_SLT) {
      return true;
    }
  }

  return false;
}

// Check what kind of br is assigned to node n
bool isLessEqual(node *n) {
  if (instructionNode *inst_node = dyn_cast<instructionNode>(n)) {
    Instruction *inst = dyn_cast<Instruction>(inst_node->getValue());

    if (cast<CmpInst>(inst)->getPredicate() == llvm::CmpInst::FCMP_OLE ||
        cast<CmpInst>(inst)->getPredicate() == llvm::CmpInst::FCMP_ULE ||
        cast<CmpInst>(inst)->getPredicate() == llvm::CmpInst::ICMP_SLE ||
        cast<CmpInst>(inst)->getPredicate() == llvm::CmpInst::ICMP_ULE) {
      return true;
    }
  }

  return false;
}

// Check what kind of br is assigned to node n
bool isGreater(node *n) {
  if (instructionNode *inst_node = dyn_cast<instructionNode>(n)) {
    Instruction *inst = dyn_cast<Instruction>(inst_node->getValue());

    if (cast<CmpInst>(inst)->getPredicate() == llvm::CmpInst::FCMP_OGT ||
        cast<CmpInst>(inst)->getPredicate() == llvm::CmpInst::FCMP_UGT ||
        cast<CmpInst>(inst)->getPredicate() == llvm::CmpInst::ICMP_SGT ||
        cast<CmpInst>(inst)->getPredicate() == llvm::CmpInst::ICMP_UGT) {
      return true;
    }
  }

  return false;
}

// Check what kind of br is assigned to node n
bool isGreaterEqual(node *n) {
  if (instructionNode *inst_node = dyn_cast<instructionNode>(n)) {
    Instruction *inst = dyn_cast<Instruction>(inst_node->getValue());

    if (cast<CmpInst>(inst)->getPredicate() == llvm::CmpInst::FCMP_OGE ||
        cast<CmpInst>(inst)->getPredicate() == llvm::CmpInst::ICMP_UGE ||
        cast<CmpInst>(inst)->getPredicate() == llvm::CmpInst::FCMP_UGE ||
        cast<CmpInst>(inst)->getPredicate() == llvm::CmpInst::ICMP_SGE) {
      return true;
    }
  }

  return false;
}

// Check what kind of br is assigned to node n
bool isEqual(node *n) {
  if (instructionNode *inst_node = dyn_cast<instructionNode>(n)) {
    Instruction *inst = dyn_cast<Instruction>(inst_node->getValue());

    if (cast<CmpInst>(inst)->getPredicate() == llvm::CmpInst::FCMP_OEQ ||
        cast<CmpInst>(inst)->getPredicate() == llvm::CmpInst::FCMP_UEQ ||
        cast<CmpInst>(inst)->getPredicate() == llvm::CmpInst::ICMP_EQ) {
      return true;
    }
  }

  return false;
}

// Check what kind of br is assigned to node n
bool isNotEqual(node *n) {
  if (instructionNode *inst_node = dyn_cast<instructionNode>(n)) {
    Instruction *inst = dyn_cast<Instruction>(inst_node->getValue());

    if (cast<CmpInst>(inst)->getPredicate() == llvm::CmpInst::FCMP_ONE ||
        cast<CmpInst>(inst)->getPredicate() == llvm::CmpInst::FCMP_UNE ||
        cast<CmpInst>(inst)->getPredicate() == llvm::CmpInst::ICMP_NE) {
      return true;
    }
  }

  return false;
}

// Adapt IR to the ISA of the CGRA (EPFL ISA)
// TODO: Distinguish between unsigned and signd operations
//       for now they are all considered signed, but the ISA needs to
//       be extended to support also unsigned operations
// TODO: Fix case pointer of pointer
void instructionSelection(graph *g) {

  std::vector<instructionNode *> nodes = g->getInstructionNodes();
  std::vector<Edge *> edges = g->getEdges();

  std::vector<node *> new_nodes;
  std::vector<Edge *> new_edges;

  std::vector<node *> node_to_delete;
  std::vector<Edge *> edge_to_delete;

  for (instructionNode *n : nodes) {
    Instruction *inst = dyn_cast<Instruction>(n->getValue());
    // errs() << "Doing " << inst->getOpcodeName() << "\n";
    switch (inst->getOpcode()) {
    case Instruction::Add:
      n->setOpcode(SADD);
      break;
    case Instruction::Sub:
      n->setOpcode(SSUB);
      break;
    case Instruction::Mul:
      n->setOpcode(SMUL);
      break;
    case Instruction::Shl:
      n->setOpcode(SLT);
      break;
    case Instruction::LShr:
      n->setOpcode(SRT);
      break;
    case Instruction::AShr:
      n->setOpcode(SRA);
      break;
    case Instruction::And:
      n->setOpcode(LAND);
      break;
    case Instruction::Or:
      n->setOpcode(LOR);
      break;
    case Instruction::Xor:
      n->setOpcode(LXOR);
      break;
    case Instruction::FAdd:
      n->setOpcode(FXP_ADD);
      break;
    case Instruction::FSub:
      n->setOpcode(FXP_SUB);
      break;
    case Instruction::FDiv:
      n->setOpcode(FXP_DIV);
      break;
    case Instruction::FMul:
      n->setOpcode(FXP_MUL);
      break;
    case Instruction::Load:
      n->setOpcode(LWI);
      break;
    case Instruction::Store:
      n->setOpcode(SWI);
      break;
    case Instruction::Trunc: {
      if (g->getPredecessors(n).size() > 1) {
        // should not happen, just a safety check
        errs() << "Trunc with more then 1 pred\n";
        break;
      }

      node_to_delete.push_back(n);
      break;
    }
    case Instruction::SExt: {
      if (g->getPredecessors(n).size() > 1) {
        // should not happen, just a safety check
        errs() << "Trunc with more then 1 pred\n";
        break;
      }

      node_to_delete.push_back(n);
      break;
    }
    case Instruction::Select:
      // n->setOpcode(BSFA);
      // don't do anything since it's going to be set, when handling the icmp
      // instr
      break;
    case Instruction::ZExt: {

      if (g->getPredecessors(n).size() > 1) {
        // should not happen, just a safety check
        errs() << "Trunc with more then 1 pred\n";
        break;
      }

      node_to_delete.push_back(n);
      break;
    }
    case Instruction::PHI:
      n->setOpcode(MV);
      break;
    case Instruction::GetElementPtr: {
      // TODO:: Handle matrix and array of pointers etc

      // multiple way to handle this. The base address could retrived by
      // making a load this would reduce the register pressure but on average
      // it will probably be not the best solution, that's why for now we
      // don't care about register pressure and we store the address in a
      // register maybe
      // the can be also handled in the backend
      // getelemntptr node is going to be replaced by one
      // node that compute the
      // offset and one that compute offset + base address
      //
      //  node1 = 4 * index
      //  node2 = base_addr + node1
      //  node2 will have a livein that is addr and an incoming edge thatis
      //   node1 n is going to be replaced by node 2 (so that only one
      //   assignment needs to be changed)
      instructionNode *offset;
      Edge *offset_edge;
      std::vector<Edge *> associate_edges = g->getAssociateEdges(n);
      std::vector<liveInNode *> livein_nodes = g->getLiveInNodes();

      n->setOpcode(SADD);
      n->setInstructionName("add");

      if (n->getLeftOperand() != nullptr &&
          dyn_cast<instructionNode>(n->getLeftOperand()) != nullptr) {

        Instruction *left_op_inst = dyn_cast<Instruction>(
            dyn_cast<instructionNode>(n->getLeftOperand())->getValue());

        if (left_op_inst->getOpcode() == Instruction::GetElementPtr) {
          if (dyn_cast<constantNode>(n->getRightOperand()) != nullptr) {
            // if previous instruction is still geteleptr and right operand is
            // constant just multiply the constant by 4 and do the addition no
            // need to add an offset node
            constantNode *tmpc = dyn_cast<constantNode>(n->getRightOperand());
            tmpc->setImmediate(tmpc->getImmediate() * 4);
          }
        } else {

          offset = new instructionNode(nodeId++, nullptr);
          offset->setOpcode(SMUL);
          offset->setInstructionName("mul");
          offset->setLeftOperand(n->getRightOperand());

          n->setRightOperand(offset);
          constantNode *c;

          c = new constantNode(nodeId++, 4);
          // i*4 should be 1,2 or 4 but for now in theISA
          // there is only load word
          offset->setRightOperand(dyn_cast<node>(c));
          g->addNode(offset);

          // livein edges do not change. need to fix only edge going in n
          for (auto e : associate_edges) {
            if (e->getDestination() != nullptr) {
              if (e->getDestination()->getId() == n->getId()) {
                bool found = false;
                for (auto ln : livein_nodes) {
                  if (e->getSource() != nullptr) {
                    if (ln->getId() == e->getSource()->getId()) {
                      found = true;
                    }
                  }
                }
                if (!found)
                  e->setDestination(offset);
              }
            }
          }
          Edge *constant_edge;
          offset_edge = new Edge(edgeId++, offset, n, 0);
          constant_edge = new Edge(edgeId++, dyn_cast<node>(c), offset, 0);
          g->addEdge(offset_edge);
          g->addEdge(constant_edge);
          g->addNode(dyn_cast<node>(c));
        }
      } else {

        offset = new instructionNode(nodeId++, nullptr);
        offset->setOpcode(SMUL);
        offset->setInstructionName("mul");
        offset->setLeftOperand(n->getRightOperand());

        n->setRightOperand(offset);
        constantNode *c;
        c = new constantNode(nodeId++, 4);
        // i*4 should be 1,2 or 4 but for now in theISA
        // there is only load word
        offset->setRightOperand(c);
        g->addNode(offset);

        // livein edges do not change. need to fix only edge going in n
        for (auto e : associate_edges) {
          if (e->getDestination() != nullptr) {
            if (e->getDestination()->getId() == n->getId()) {
              bool found = false;
              for (auto ln : livein_nodes) {
                if (e->getSource() != nullptr) {
                  if (ln->getId() == e->getSource()->getId()) {
                    found = true;
                  }
                }
              }
              if (!found)
                e->setDestination(offset);
            }
          }
        }
        Edge *constant_edge;
        offset_edge = new Edge(edgeId++, offset, n, 0);
        constant_edge = new Edge(edgeId++, dyn_cast<node>(c), offset, 0);
        g->addEdge(offset_edge);
        g->addEdge(constant_edge);
        g->addNode(dyn_cast<node>(c));
      }

      // TODO: the address can also be computed from an Inst (ie pointer of
      // pointer) must handle also this case in the future

      break;
    }
    case Instruction::Br: {
      // Should be solved from the icmp
      // Only need to handle unconditional branches
      // NOT HANDLED RIGHT NOW, need to find possible problematic cases
      BranchInst *cbr = dyn_cast<BranchInst>(inst);
      if (cbr->isUnconditional()) {
        errs() << "Unconditional jumps not supported\n";
        exit(0);
        break;
      } else {
        // Should not happen
        if (dyn_cast<Instruction>(cbr->getCondition())->getOpcode() !=
            Instruction::ICmp) {
          errs() << "Not using iCMP condition\n"
                 << *cbr->getCondition() << "\n";
          break;
        }
      }
      break;
    }
    case Instruction::ICmp: {

      // TODO: need to divide in case where icmp is used by only one br, icmp
      // used by br and inst, br used by inst CHECK IF YOU CAN LEAVE THE ICMP
      // AND ALWAYS USE A SUB INST INSTEAD (wih this sometimes you will have
      // the same number
      //  of nodes, so no reduction by compressing the instruction)
      // if (g->getSuccessors(n).size() > 1){
      //   errs() << "TODO Handle\n";
      //   for(auto t: g->getSuccessors(n)){
      //     errs() << "Succ of icmp " << t->getName() << "\n";
      //   }
      //   break;
      // }

      std::string op;
      int cmpopcode = -1;
      if (isEqual(n)) {
        op = "beq";
        cmpopcode = BEQ;
      } else if (isNotEqual(n)) {
        op = "bne";
        cmpopcode = BNE;
      } else if (isGreater(n)) {
        op = "bgt";
        cmpopcode = BGT;
      } else if (isGreaterEqual(n)) {
        op = "bge";
        cmpopcode = BGE;
      } else if (isLess(n)) {
        op = "blt";
        cmpopcode = BLT;
      } else if (isLessEqual(n)) {
        op = "ble";
        cmpopcode = BLE;
      }

      if (cmpopcode != -1) {

        if (g->getSuccessors(n).size() == 1 &&
            dyn_cast<Instruction>(
                dyn_cast<instructionNode>(g->getSuccessors(n)[0])->getValue())
                    ->getOpcode() == Instruction::Br) {
          // Don't have to add another node, I can modify icmp and then remove
          // the br node
          n->setOpcode(cmpopcode);
          n->setInstructionName(op);
          if (std::find(node_to_delete.begin(), node_to_delete.end(),
                        g->getSuccessors(n)[0]) == node_to_delete.end())
            node_to_delete.push_back(g->getSuccessors(n)[0]);
          // Don't have to modify the assignments since the br node is removed
          // and compacted with the icmp that already has a proper assignment
          // (or at least should have)
        } else if (g->getSuccessors(n).size() == 1 &&
                   dyn_cast<Instruction>(
                       dyn_cast<instructionNode>(g->getSuccessors(n)[0])
                           ->getValue())
                           ->getOpcode() == Instruction::Select) {
          n->setInstructionName("sub");
          n->setOpcode(SSUB);
          Instruction *suc_inst = dyn_cast<Instruction>(
              dyn_cast<instructionNode>(g->getSuccessors(n)[0])->getValue());
          instructionNode *succ_node =
              dyn_cast<instructionNode>(g->getNode(suc_inst));
          if (succ_node == nullptr) {
            errs() << "Node should be in DFG\n";
            exit(0);
          }

          if (isEqual(n) || isNotEqual(n)) {
            // eq and neq
            succ_node->setInstructionName("bzfa");
            succ_node->setOpcode(BZFA);
          } else if (isLess(n) || isGreater(n)) {
            // l and g
            succ_node->setInstructionName("bsfa");
            succ_node->setOpcode(BSFA);
          } else if (isGreaterEqual(n) || isLessEqual(n)) {
            // ge and le
            errs() << "Fix icmp ge and le\n";
          }

        } else {
          n->setInstructionName("sub");
          n->setOpcode(SSUB);

          for (auto succ_node : g->getSuccessors(n)) {
            if (dyn_cast<instructionNode>(succ_node) == nullptr)
              continue;
            Instruction *succ_inst = dyn_cast<Instruction>(
                (dyn_cast<instructionNode>(succ_node))->getValue());

            if (succ_inst->getOpcode() == Instruction::Select) {
              if (isLess(n) || isGreater(n)) {
                errs() << "less or greater\n";
                dyn_cast<instructionNode>(succ_node)->setOpcode(BSFA);
                dyn_cast<instructionNode>(succ_node)->setInstructionName(
                    "bsfa");
              } else if (isEqual(n) || isNotEqual(n)) {
                dyn_cast<instructionNode>(succ_node)->setOpcode(BZFA);
                dyn_cast<instructionNode>(succ_node)->setInstructionName(
                    "bzfa");
              } else {
                errs() << "Handle <= and >= cases, probably due to unoptimized "
                       << "code ";
              }
            } else {
              // In all the other cases you should add nodes such that
              // you can use the flag as data in a register
              if (isLess(n) || isGreater(n)) {
                // create nodes  to check the flags
                instructionNode *n1;
                constantNode *c1, *c2;

                n1 = new instructionNode(nodeId++, nullptr);
                n1->setOpcode(BSFA);
                n1->setInstructionName("bsfa");
                n1->setPredicateOperand(n);

                c1 = new constantNode(nodeId++, 0);
                c2 = new constantNode(nodeId++, 1);
                Edge *e;
                // less
                if (isLess(n)) {
                  e = new Edge(edgeId++, dyn_cast<node>(c2), dyn_cast<node>(n1),
                               0);
                  g->addNode(dyn_cast<node>(c2));
                  g->addEdge(e);
                  // greater
                } else if (isGreater(n)) {
                  e = new Edge(edgeId++, dyn_cast<node>(c1), dyn_cast<node>(n1),
                               0);
                  g->addNode(dyn_cast<node>(c1));
                  g->addEdge(e);
                } else {
                  errs() << "Should not be in this if case (<, >)\n";
                }

                // update dependencies

                for (auto tmp_e : g->getAssociateEdges(n)) {
                  if (dyn_cast<instructionNode>(tmp_e->getDestination()) ==
                      nullptr)
                    continue;
                  if (tmp_e->getSource()->getId() == n->getId() &&
                      dyn_cast<Instruction>(
                          dyn_cast<instructionNode>(tmp_e->getDestination())
                              ->getValue())
                              ->getOpcode() != Instruction::Select) {
                    e->setSource(n1);

                    if (dyn_cast<instructionNode>(tmp_e->getDestination())
                                ->getLeftOperand() != nullptr &&
                        dyn_cast<instructionNode>(tmp_e->getDestination())
                                ->getLeftOperand()
                                ->getId() == n->getId()) {
                      dyn_cast<instructionNode>(tmp_e->getDestination())
                          ->setLeftOperand(n1);
                    }

                    if (dyn_cast<instructionNode>(tmp_e->getDestination()) !=
                        nullptr) {
                      if (dyn_cast<instructionNode>(tmp_e->getDestination())
                                  ->getRightOperand() != nullptr &&
                          dyn_cast<instructionNode>(tmp_e->getDestination())
                                  ->getRightOperand()
                                  ->getId() == n->getId()) {
                        dyn_cast<instructionNode>(tmp_e->getDestination())
                            ->setRightOperand(n1);
                      }
                    }

                    if (dyn_cast<instructionNode>(tmp_e->getDestination()) !=
                        nullptr) {
                      if (dyn_cast<instructionNode>(tmp_e->getDestination())
                                  ->getPredicateOperand() != nullptr &&
                          dyn_cast<instructionNode>(tmp_e->getDestination())
                                  ->getPredicateOperand()
                                  ->getId() == n->getId()) {
                        dyn_cast<instructionNode>(tmp_e->getDestination())
                            ->setPredicateOperand(n1);
                      }
                    }
                  }
                }

                Edge *e1;
                e1 = new Edge(edgeId++, dyn_cast<node>(n), dyn_cast<node>(n1),
                              0);

                g->addEdge(e1);
              } else if (isEqual(n) || isNotEqual(n)) {
                // create nodes  to check the flags
                instructionNode *n1;
                constantNode *c1, *c2;
                Edge *e;
                n1 = new instructionNode(nodeId++, nullptr);
                n1->setOpcode(BZFA);
                n1->setInstructionName("bzfa");
                n1->setPredicateOperand(n);

                // actually I don't need the following if cases, since
                // icmp fix the operands according to == or !=
                //(basically position of c1 and c2 can be fixed from thestart)
                // not equal to zero
                if (isNotEqual(n)) {
                  c1 = new constantNode(nodeId++, 0);
                  e = new Edge(edgeId++, dyn_cast<node>(c1), dyn_cast<node>(n1),
                               0);
                  g->addNode(dyn_cast<node>(c1));
                  // equal to zero
                } else if (isEqual(n)) {
                  c2 = new constantNode(nodeId++, 1);
                  e = new Edge(edgeId++, dyn_cast<node>(c2), dyn_cast<node>(n1),
                               0);
                  g->addNode(dyn_cast<node>(c2));
                } else {
                  errs() << "Should not be in this if case (==, !=)\n";
                }

                g->addNode(dyn_cast<node>(n1));
                g->addEdge(e);

                // update dependencies
                for (auto tmp_e : g->getAssociateEdges(n)) {
                  if (dyn_cast<instructionNode>(tmp_e->getDestination()) ==
                      nullptr)
                    continue;
                  if (tmp_e->getSource()->getId() == n->getId() &&
                      dyn_cast<Instruction>(
                          dyn_cast<instructionNode>(tmp_e->getDestination())
                              ->getValue())
                              ->getOpcode() != Instruction::Select) {
                    tmp_e->setSource(n1);
                  }
                }

                Edge *e1;
                e1 = new Edge(edgeId++, dyn_cast<node>(n), dyn_cast<node>(n1),
                              0);

                g->addEdge(e1);
              } else {
                errs() << "Handle <= and >= cases, probably due to unoptimized "
                          "code ";
              }
            }
          }
        }
      }
      break;
    }
    default: {
      errs() << "Instruction not supported or not defined in the ISA..."
             << inst->getOpcodeName() << "\n";
      break;
    }
    }
    // Remove
  }
  for (auto n : node_to_delete) {
    // Do not use if n must be sub with another node
    g->removeNode(n);
  }
}

PreservedAnalyses CGRAExtractPass::run(Loop &L, LoopAnalysisManager &AM,
                                       LoopStandardAnalysisResults &AR,
                                       LPMUpdater &U) {

  auto nestedLoop = L.getSubLoops();
  if (nestedLoop.size() == 0) {
    if (hasPragmaCGRAAcc(&L, "llvm.loop.cgra.acc")) {

      graph *g;

      Instruction *lastInst;

      canDoIS = true;

      std::string filename;
      std::string directory = "acc";
      std::vector<BasicBlock *> bbs = L.getBlocks();
      nodeId = 0;
      edgeId = 0;
      loop_number++;
      directory += std::to_string(loop_number);
      filename = directory + "/acc";
      g = new graph();

      // Create all the nodes; each instruction is a node
      for (int i = 0; i < (int)bbs.size(); i++) {
        for (BasicBlock::iterator inst = bbs[i]->begin(); inst != bbs[i]->end();
             ++inst) {
          addNode(&(*inst), g);
          lastInst = &(*inst);
        }
      }

      // Generate DFG
      for (int i = 0; i < (int)bbs.size(); i++)
        for (BasicBlock::iterator inst = bbs[i]->begin(); inst != bbs[i]->end();
             ++inst)
          addDependency(&(*inst), g);

      // operant assignment must be done before constant and livein
      // are added
      assignInstructionOperands(g);
      // constant are assigned and created at the same time
      addConstants(bbs, g);
      // livein are assigned and created at the same time
      addLiveIn(L.getBlocks()[0], g);
      addLiveOut(L.getBlocks()[0], g);

      //if (canDoIS)
      //  instructionSelection(g);

      mkdir(directory.c_str(), 0777);

      g->printDot(filename);
      //g->printNodes(filename);
      g->printInstructionEdges(filename);
      delete g;
    }
  }

  return PreservedAnalyses::all();
}
