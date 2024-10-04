#include "llvm/Transforms/CGRAExtract/node.h"

// Instruction node

instructionNode::instructionNode(int id, llvm::Value *llvm_instruction)
    : node(id, nodeType::INSTRUCTION), llvm_instruction(llvm_instruction) {

  // this->id = id;
  // this->type = nodeType::INSTRUCTION;

  // this->llvm_instruction = llvm_instruction;

  this->opcode = -1;
  this->instruction_name = "";

  this->left_operand = nullptr;
  this->right_operand = nullptr;
  this->predicate_operand = nullptr;
}

std::string instructionNode::dump() const {

  return "Instruction Id: " + std::to_string(id) +
         " name: " + instruction_name + " opcode: " + std::to_string(opcode) +
         "\nLeft Operand: \n\t" + left_operand->dump() + "Rigth Operand: \n\t" +
         right_operand->dump() + "Predicate Operand: \n\t" +
         predicate_operand->dump();
}

void instructionNode::setLeftOperand(node *operand) {
  this->left_operand = operand;
}

void instructionNode::setRightOperand(node *operand) {
  this->right_operand = operand;
}

void instructionNode::setPredicateOperand(node *operand) {
  this->predicate_operand = operand;
}

void instructionNode::setInstructionName(std::string name) {
  this->instruction_name = name;
}

void instructionNode::setValue(llvm::Value *llvm_value) {
  this->llvm_instruction = llvm_value;
}

void instructionNode::setOpcode(int op) { this->opcode = op; }

node *instructionNode::getLeftOperand() { return this->left_operand; }

node *instructionNode::getRightOperand() { return this->right_operand; }

node *instructionNode::getPredicateOperand() { return this->predicate_operand; }

std::string instructionNode::getInstructionName() {
  return this->instruction_name;
}

llvm::Value *instructionNode::getValue() { return this->llvm_instruction; }

int instructionNode::getOpcode() { return this->opcode; }

// Constant node

constantNode::constantNode(int id, int immediate)
    : node(id, nodeType::CONSTANT), immediate(immediate) {

  this->immediate_position = 0;
}

std::string constantNode::dump() const {
  return "Constant Id: " + std::to_string(id) +
         " immediate: " + std::to_string(immediate) +
         " pos: " + std::to_string(immediate_position) + "\n";
}

void constantNode::setImmediate(int value) { this->immediate = value; }

void constantNode::setImmediatePosition(int pos) {
  this->immediate_position = pos;
}

int constantNode::getImmediate() { return this->immediate; }

int constantNode::getImmediatePosition() { return this->immediate_position; }

// Live In node

liveInNode::liveInNode(int id, llvm::Value *llvm_instruction)
    : node(id, nodeType::LIVE_IN), llvm_instruction(llvm_instruction) {

  this->id = id;
  this->type = nodeType::LIVE_IN;

  this->llvm_instruction = llvm_instruction;

  this->opcode = LWD;
}

std::string liveInNode::dump() const {
  return "Live In Id: " + std::to_string(id) + " name: " + instruction_name +
         "\n";
}

void liveInNode::setLiveInName(std::string name) {
  this->instruction_name = name;
}

void liveInNode::setValue(llvm::Value *llvm_instruction) {
  this->llvm_instruction = llvm_instruction;
}

void liveInNode::setOpcode(int opcode) { this->opcode = opcode; }

std::string liveInNode::getLiveInName() { return this->instruction_name; }

llvm::Value *liveInNode::getValue() { return this->llvm_instruction; }

int liveInNode::getOpcode() { return this->opcode; }

// Live Out node

liveOutNode::liveOutNode(int id, llvm::Value *llvm_instruction)
    : node(id, nodeType::LIVE_OUT), llvm_instruction(llvm_instruction) {

  this->id = id;
  this->type = nodeType::LIVE_OUT;

  this->llvm_instruction = llvm_instruction;

  this->opcode = SWD;
}

std::string liveOutNode::dump() const {
  return "Live Out Id: " + std::to_string(id) + " name: " + instruction_name +
         "\n";
}

void liveOutNode::setLiveOutName(std::string name) {
  this->instruction_name = name;
}

void liveOutNode::setValue(llvm::Value *llvm_instruction) {
  this->llvm_instruction = llvm_instruction;
}

void liveOutNode::setOpcode(int opcode) { this->opcode = opcode; }

std::string liveOutNode::getLiveOutName() { return this->instruction_name; }

llvm::Value *liveOutNode::getValue() { return this->llvm_instruction; }

int liveOutNode::getOpcode() { return this->opcode; }