#include <vector>
#include <stdlib.h>
#include "instruction.h"
#include "vertex_group.cpp"

#ifndef DRAWER
#define DRAWER
#endif

class StaticShapeDrawer
{
private:
  size_t verticesSize;
  float *verticesPtr;
  float *readArgumentsStream(float *&ptr, size_t number);
  std::vector<VertexGroup *> vgs;
  VertexGroup *frame;

public:
  StaticShapeDrawer(std::vector<char> instructions, float *argsPtr);
  float *getVerticesPtr();
  size_t getVerticesSize();
  float *getFrameBound();
  ~StaticShapeDrawer();
};
