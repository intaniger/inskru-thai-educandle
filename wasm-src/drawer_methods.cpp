#ifndef DRAWER_METHODS
#define DRAWER_METHODS
#endif
#include "drawer.h"

float *StaticShapeDrawer::readArgumentsStream(float *&ptr, size_t number)
{
  float *toReturn = ptr;
  ptr += number;
  return toReturn;
}

float *StaticShapeDrawer::getVerticesPtr()
{
  return this->verticesPtr;
}

size_t StaticShapeDrawer::getVerticesSize()
{
  return this->verticesSize;
}

StaticShapeDrawer::~StaticShapeDrawer()
{
}

float *StaticShapeDrawer::getFrameBound()
{
  return this->frame->getBoundary();
}