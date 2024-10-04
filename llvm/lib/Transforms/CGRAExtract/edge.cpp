#include "llvm/Transforms/CGRAExtract/edge.h"

// Constructor implementation
Edge::Edge(int id, node *source, node *destination, int distance, int latency)
    : id(id), source(source), destination(destination), distance(distance),
      latency(latency) {}

Edge::~Edge() {}

int Edge::getId() { return id; }

int Edge::getLatency() { return latency; }

node *Edge::getSource() { return source; }

node *Edge::getDestination() { return destination; }

int Edge::getDistance() { return distance; }

void Edge::setDistance(int d) { distance = d; }

void Edge::setSource(node *n) { source = n; }

void Edge::setDestination(node *n) { destination = n; }

void Edge::setLatency(int lat) { latency = lat; }
