#include "llvm/Transforms/CGRAExtract/graph.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

/**************************** Graph ******************************************/
graph::graph() {}

graph::~graph() {
  nodes.clear();
  edges.clear();
}

/******************* ADD *******************/

void graph::addNode(node *n) { nodes.push_back(n); }

void graph::addEdge(Edge *a) { edges.push_back(a); }

/******************* GET *******************/

node *graph::getNode(Instruction *inst) {
  for (auto inst_node : getInstructionNodes())
    if (dyn_cast<Instruction>(inst_node->getValue()) == inst)
      return inst_node;
  return nullptr;
}

std::vector<node *> graph::getNodes() { return nodes; }

std::vector<Edge *> graph::getEdges() { return edges; }

std::vector<Edge *> graph::getAssociateEdges(node *n) {
  std::vector<Edge *> associate_edges;

  for (auto e : edges) {
    if (e->getSource()->getId() == n->getId() ||
        e->getDestination()->getId() == n->getId())
      associate_edges.push_back(e);
  }

  return associate_edges;
}

std::vector<instructionNode *> graph::getInstructionNodes() {
  std::vector<instructionNode *> instructionNodes;
  for (auto inst_node : nodes) {
    if (instructionNode *tmp_node = dyn_cast<instructionNode>(inst_node)) {
      instructionNodes.push_back(tmp_node);
    }
  }
  return instructionNodes;
}

std::vector<liveInNode *> graph::getLiveInNodes() {
  std::vector<liveInNode *> liveInNodes;
  for (auto in_node : nodes) {
    if (liveInNode *tmp_node = dyn_cast<liveInNode>(in_node)) {
      liveInNodes.push_back(tmp_node);
    }
  }
  return liveInNodes;
}

std::vector<liveOutNode *> graph::getLiveOutNodes() {
  std::vector<liveOutNode *> liveOutNodes;
  for (auto out_node : nodes) {
    if (liveOutNode *tmp_node = dyn_cast<liveOutNode>(out_node)) {
      liveOutNodes.push_back(tmp_node);
    }
  }
  return liveOutNodes;
}

std::vector<constantNode *> graph::getConstantNodes() {
  std::vector<constantNode *> constantNodes;
  for (auto const_node : nodes) {
    if (constantNode *tmp_node = dyn_cast<constantNode>(const_node)) {
      constantNodes.push_back(tmp_node);
    }
  }
  return constantNodes;
}

std::vector<node *> graph::getSuccessors(node *n) {
  std::vector<node *> succs;
  for (auto e : edges) {
    if (e->getSource()->getId() == n->getId()) {
      if (std::find(succs.begin(), succs.end(), n) == succs.end()) {
        succs.push_back(e->getDestination());
      }
    }
  }
  return succs;
}

std::vector<node *> graph::getPredecessors(node *n) {
  std::vector<node *> preds;
  for (auto e : edges) {
    if (e->getDestination()->getId() == n->getId()) {
      if (std::find(preds.begin(), preds.end(), n) == preds.end()) {
        preds.push_back(e->getSource());
      }
    }
  }
  return preds;
}

/******************* PRINT *******************/

void graph::printDot(std::string filename) {
  std::ofstream dotFile;
  std::string graphname = filename;
  filename.append("_loop_graph.dot");
  dotFile.open(filename.c_str());
  dotFile << "digraph " << graphname << " { \n{\n compound=true;";

  // print nodes
  for (auto inst_node : getInstructionNodes()) {
    dotFile << "\n"
            << inst_node->getId() << " [color=black, label=\""
            << inst_node->getId() << "  " << inst_node->getInstructionName()
            << "\"];\n";
  }

  // print edges
  for (size_t i = 0; i < edges.size(); i++) {
    if (edges[i]->getSource() != nullptr &&
        edges[i]->getDestination() != nullptr) {
      if (edges[i]->getSource()->getType() == nodeType::INSTRUCTION) {

        if (edges[i]->getDistance() > 0)
          dotFile << edges[i]->getSource()->getId() << " -> "
                  << edges[i]->getDestination()->getId() << " [color=red]"
                  << "\n";
        else
          dotFile << edges[i]->getSource()->getId() << " -> "
                  << edges[i]->getDestination()->getId() << "\n";
      }

      if (edges[i]->getSource()->getType() == nodeType::LIVE_IN) {
        dotFile << edges[i]->getSource()->getId() << " -> "
                << edges[i]->getDestination()->getId() << " [color=purple1]"
                << "\n";
      }

      if (edges[i]->getSource()->getType() == nodeType::LIVE_OUT) {
        dotFile << edges[i]->getSource()->getId() << " -> "
                << edges[i]->getDestination()->getId() << " [color=dodgerblue1]"
                << "\n";
      }

      if (edges[i]->getSource()->getType() == nodeType::CONSTANT) {
        dotFile << edges[i]->getSource()->getId() << " -> "
                << edges[i]->getDestination()->getId() << " [color=goldenrod1]"
                << "\n";
      }

    } else {
      errs() << "ERROR on edge: " << edges[i] << "\n";
    }
  }

  // print constants
  for (auto const_node : getConstantNodes()) {
    dotFile << "\n"
            << const_node->getId() << " [color=goldenrod1, label=\""
            << const_node->getId() << " C_" << const_node->getImmediate()
            << "\"];\n";
  }

  // nodeType::LIVE_IN nodes
  for (auto live_in_node : getLiveInNodes()) {
    dotFile << "\n"
            << live_in_node->getId() << " [color=purple1, label=\""
            << live_in_node->getId() << " " << live_in_node->getLiveInName()
            << "\"];\n";
  }

  // nodeType::LIVE_OUT nodes
  for (auto live_out_node : getLiveOutNodes()) {
    dotFile << "\n"
            << live_out_node->getId() << " [color=dodgerblue1, label=\""
            << live_out_node->getId() << " " << live_out_node->getLiveOutName()
            << "\"];\n";
  }

  dotFile << "\n}\n";

  dotFile << "\n}";
  dotFile.close();
}

void graph::printNodes(std::string filename) {
  std::ofstream dotFile;
  std::string graphname = filename;
  filename.append("_nodes");
  dotFile.open(filename.c_str());

  // print nodes
  for (auto n : nodes) {
    int node_id = n->getId();
    std::string node_type;
    std::string instruction_name = "nil";
    int opcode = -1;
    int left_operand_id = -1;
    int right_operand_id = -1;
    int predicate_operand_id = -1;
    int immediate = 0;
    int immediate_position = 0;

    if (n->getType() == nodeType::INSTRUCTION)
      node_type = "instruction";
    else if (n->getType() == nodeType::LIVE_IN)
      node_type = "live_in";
    else if (n->getType() == nodeType::LIVE_OUT)
      node_type = "live_out";
    else if (n->getType() == nodeType::CONSTANT)
      node_type = "constant";
    else
      node_type = "none";

    if (instructionNode *tmp_node = dyn_cast<instructionNode>(n)) {
      instruction_name = tmp_node->getInstructionName();
      opcode = tmp_node->getOpcode();
      if (tmp_node->getLeftOperand() != nullptr)
        left_operand_id = tmp_node->getLeftOperand()->getId();
      if (tmp_node->getRightOperand() != nullptr)
        right_operand_id = tmp_node->getRightOperand()->getId();
      if (tmp_node->getPredicateOperand() != nullptr)
        predicate_operand_id = tmp_node->getPredicateOperand()->getId();
    }

    if (liveInNode *tmp_node = dyn_cast<liveInNode>(n)) {
      instruction_name = tmp_node->getLiveInName();
      opcode = tmp_node->getOpcode();
    }

    if (liveOutNode *tmp_node = dyn_cast<liveOutNode>(n)) {
      instruction_name = tmp_node->getLiveOutName();
      opcode = tmp_node->getOpcode();
    }

    if (constantNode *tmp_node = dyn_cast<constantNode>(n)) {
      immediate = tmp_node->getImmediate();
      immediate_position = tmp_node->getImmediatePosition();
    }

    dotFile << node_id << " " << node_type << " " << instruction_name << " "
            << opcode << " " << left_operand_id << " " << right_operand_id
            << " " << predicate_operand_id << " " << immediate << " "
            << immediate_position << "\n";
  }

  dotFile.close();
}

void graph::printEdges(std::string filename) {
  std::ofstream dotFile;
  std::string graphname = filename;
  filename.append("_edges");
  dotFile.open(filename.c_str());

  // print edges
  for (size_t i = 0; i < edges.size(); i++)
    dotFile << edges[i]->getSource()->getId() << " "
            << edges[i]->getDestination()->getId() << " "
            << edges[i]->getDistance() << " " << edges[i]->getLatency() << "\n";

  dotFile.close();
}


void graph::removeNode(node *n) {
  // n should have only one pre
  if (getPredecessors(n).size() > 1) {
    errs() << "Should have only one pred\n";
    return;
  }

  node *pre = getPredecessors(n)[0];

  std::vector<Edge *> to_remove;
  // update successors assignment
  for (auto suc : getSuccessors(n)) {
    if (instructionNode *tmp_succ = dyn_cast<instructionNode>(suc)) {
      if (tmp_succ->getLeftOperand() != nullptr) {
        if (tmp_succ->getLeftOperand()->getId() == n->getId()) {
          tmp_succ->setLeftOperand(pre);
        }
      }
      if (tmp_succ->getRightOperand() != nullptr) {
        if (tmp_succ->getRightOperand()->getId() == n->getId()) {
          tmp_succ->setRightOperand(pre);
        }
      }
      if (tmp_succ->getPredicateOperand() != nullptr) {
        if (tmp_succ->getPredicateOperand()->getId() == n->getId()) {
          tmp_succ->setPredicateOperand(pre);
        }
      }
    }
  }

  // update edges
  for (auto e : getAssociateEdges(n)) {
    if (e->getSource()->getId() == n->getId()) {
      e->setSource(pre);
    }
    if (e->getDestination()->getId() == n->getId()) {
      // remove edge
      if (std::find(to_remove.begin(), to_remove.end(), e) == to_remove.end())
        to_remove.push_back(e);
    }
  }

  nodes.erase(std::remove(nodes.begin(), nodes.end(), n), nodes.end());
  for (auto e : to_remove) {
    edges.erase(std::remove(edges.begin(), edges.end(), e), edges.end());
  }
}
