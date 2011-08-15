#include "Arc.h"
#include "Node.h"

Arc::Arc(Node* t, Node* h) : tail(t), head(h), cap(0), flow(0) {}

Arc::Arc(Node* t, Node* h, int ca, int co) : tail(t), head(h), cap(ca), flow(co) {}
