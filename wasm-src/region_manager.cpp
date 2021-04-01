#include "drawer.h"

class RegionManager
{
private:
  StaticShapeDrawer *drawer;

public:
  float uniforms[9];
  float *const regionContainerUniform = uniforms;
  float *const regionContainerOffsetUniform = uniforms + 2;
  float *const centerUniform = uniforms + 4;
  float *const screenUniform = uniforms + 6;
  float *const scaleUniform = uniforms + 8;

  RegionManager(
      StaticShapeDrawer *regionDrawer,
      float screenWidth,
      float screenHeight,
      float center[2],
      float scale) : uniforms{
                         // container (2)
                         0, 0,
                         // container_offset (2)
                         0, 0,
                         // center (2)
                         center[0], center[1],
                         // screen (2)
                         0, 0,
                         // scale (1)
                         scale},
                     drawer{regionDrawer}
  {
    float *objectBound = regionDrawer->getFrameBound();

    regionContainerUniform[0] = objectBound[2] - objectBound[0];
    regionContainerUniform[1] = objectBound[3] - objectBound[1]; // TODO: convert to negatively;

    regionContainerOffsetUniform[0] = objectBound[0];
    regionContainerOffsetUniform[1] = objectBound[1];

    screenUniform[0] = screenWidth;
    screenUniform[1] = screenHeight;
  };

  void setCenter(float center[2])
  {
    this->centerUniform[0] = center[0];
    this->centerUniform[1] = center[1];
  }

  void setScale(float scale)
  {
    *this->scaleUniform = scale;
  }

  float *getVerticesPtr()
  {
    return this->drawer->getVerticesPtr();
  }

  size_t getVerticesSize()
  {
    return this->drawer->getVerticesSize();
  }

  float getWidth()
  {
    return this->regionContainerUniform[0];
  }

  float getHeight()
  {
    return this->regionContainerUniform[1];
  }
  ~RegionManager();
};

RegionManager::~RegionManager()
{
}
