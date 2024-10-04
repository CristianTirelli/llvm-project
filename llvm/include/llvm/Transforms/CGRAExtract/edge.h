#ifndef LLVM_TRANSFORMS_CGRAEXTRACT_EDGE_H
#define LLVM_TRANSFORMS_CGRAEXTRACT_EDGE_H

#include "node.h"
#include <memory>

class Edge {
  int id;
  node *source;
  node *destination;
  int distance;
  int latency;

public:
  Edge(int id, node *source, node *destination, int distance, int latency = 1);
  virtual ~Edge();

  /******************* GET *******************/
  int getId();
  int getLatency();
  node *getSource();
  node *getDestination();
  int getDistance();

  /******************* SET *******************/
  void setDistance(int d);
  void setSource(node *n);
  void setDestination(node *n);
  void setLatency(int lat);
};

#endif // LLVM_TRANSFORMS_CGRAEXTRACT_EDGE_H