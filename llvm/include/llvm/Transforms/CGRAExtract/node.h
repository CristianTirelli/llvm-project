#ifndef node_H_
#define node_H_

#include "llvm/IR/Instruction.h"
#include "llvm/IR/Value.h"

#include "utility.h"
#include <string>
#include <vector>

// Base class node

enum class nodeType { INSTRUCTION, LIVE_IN, LIVE_OUT, CONSTANT, UNDEF };

class node {

protected:
  nodeType type;
  int id;

public:
  node(int id, nodeType type) : id(id), type(type) {}

  virtual ~node() = default;

  // Get node ID
  int getId() const { return id; }

  // Get node type
  nodeType getType() const { return type; }

  virtual std::string dump() const { return "Generic node\n"; }
};

// Derived class for instructionNode
class instructionNode : public node {

private:
  int id;
  llvm::Value *llvm_instruction;
  std::string instruction_name;
  int opcode;

  node *left_operand;
  node *right_operand;
  node *predicate_operand;

public:
  instructionNode(int id, llvm::Value *llvm_instruction);

  void setLeftOperand(node *operand);
  void setRightOperand(node *operand);
  void setPredicateOperand(node *operand);
  void setInstructionName(std::string name);
  void setValue(llvm::Value *llvm_value);
  void setOpcode(int op);

  node *getLeftOperand();
  node *getRightOperand();
  node *getPredicateOperand();
  std::string getInstructionName();
  llvm::Value *getValue();
  int getOpcode();

  // Overridden dump method
  std::string dump() const override;

  static bool classof(const node *node) {
    // Check if the node is of type InstructionNode
    return node->getType() == nodeType::INSTRUCTION;
  }
};

// Derived class for constantNode
class constantNode : public node {

private:
  int id;

  int immediate;
  int immediate_position;

public:
  constantNode(int id, int immediate);
  void setImmediate(int value);
  void setImmediatePosition(int pos);

  int getImmediate();
  int getImmediatePosition();

  static bool classof(const node *node) {
    return node->getType() == nodeType::CONSTANT;
  }
  // Overridden dump method
  std::string dump() const override;
};

// Derived class for liveInNode
class liveInNode : public node {

private:
  int id;
  llvm::Value *llvm_instruction;
  std::string instruction_name;
  int opcode;

public:
  liveInNode(int id, llvm::Value *llvm_instruction);

  void setLiveInName(std::string name);
  void setValue(llvm::Value *llvm_value);
  void setOpcode(int opcode);

  std::string getLiveInName();
  llvm::Value *getValue();
  int getOpcode();

  // Overridden dump method
  std::string dump() const override;
  static bool classof(const node *node) {
    return node->getType() == nodeType::LIVE_IN;
  }
};

// Derived class for liveOutNode
class liveOutNode : public node {

private:
  int id;
  llvm::Value *llvm_instruction;
  std::string instruction_name;
  int opcode;

public:
  liveOutNode(int id, llvm::Value *llvm_instruction);

  void setLiveOutName(std::string name);
  void setValue(llvm::Value *llvm_value);
  void setOpcode(int opcode);

  std::string getLiveOutName();
  llvm::Value *getValue();
  int getOpcode();

  // Overridden dump method
  std::string dump() const override;

  static bool classof(const node *node) {
    return node->getType() == nodeType::LIVE_OUT;
  }
};
#endif