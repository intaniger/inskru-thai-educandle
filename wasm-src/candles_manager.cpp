#include <vector>
#include "candle.cpp"

class CandlesManager
{
private:
  std::vector<Candle *> candles;
  std::vector<Candle *> projected_candles;
  size_t currentRenderingIndex;
  float inverseContainerRatio;

public:
  float constanlyUniforms[5];
  float *const screenRatioUniform = constanlyUniforms;
  float *const containerRatioUniform = constanlyUniforms + 1;
  float *const centerUniform = constanlyUniforms + 2;
  float *const scaleUniform = constanlyUniforms + 4;

  float individualUniforms[3];
  float *const lightNumberUniform = individualUniforms;
  float *const luminanceUniform = individualUniforms + 1;
  float *const radiusUniform = individualUniforms + 2;

  float lightPositions[2];
  float renderRectangle[8];
  // lightNumber [1]
  // luminance [1]
  // radius [1]

  // float const *lightNumberRatioUniform = uniforms + 5;

  CandlesManager(float screenWidth,
                 float screenHeight,
                 float objectWidth,
                 float objectHeight,
                 float center[2],
                 float scale) : constanlyUniforms{
                                    // screen[1]
                                    screenWidth / screenHeight,
                                    // container[1]
                                    objectWidth / objectHeight,
                                    // center[2]
                                    center[0], center[1],
                                    // scale[1]
                                    scale},
                                individualUniforms{
                                    // lightNumber [1]
                                    1,
                                    // luminance [1]
                                    DEFAULT_LUMINANCE,
                                    // radius [1]
                                    DEFAULT_RADIUS,
                                },
                                lightPositions{-1.0, -1.0}, inverseContainerRatio{objectHeight / objectWidth} {};

  int getCandleCounts();
  void registerCandle(Candle *cdl);
  int renderNextCandle();

  int resetIndex();
  void setCenter(float center[2]);
  void setScale(float scale);
  ~CandlesManager();
};

int CandlesManager::getCandleCounts()
{
  return this->projected_candles.size();
}

void CandlesManager::registerCandle(Candle *cdl)
{
  this->candles.push_back(cdl);
  this->projected_candles.push_back(cdl);
  this->renderNextCandle();
}

int CandlesManager::renderNextCandle()
{
  auto cdl = this->projected_candles[this->currentRenderingIndex++];
  float projectedX = cdl->position[0],
        projectedY = 0.5 + (cdl->position[1] - 0.5) * inverseContainerRatio;

  *this->lightNumberUniform = 1;
  *this->luminanceUniform = cdl->luminance;
  *this->radiusUniform = cdl->radius;
  this->lightPositions[0] = projectedX;
  this->lightPositions[1] = projectedY;

  this->renderRectangle[0] = projectedX - cdl->radius;
  this->renderRectangle[1] = projectedY - cdl->radius;

  this->renderRectangle[2] = projectedX + cdl->radius;
  this->renderRectangle[3] = projectedY - cdl->radius;

  this->renderRectangle[4] = projectedX - cdl->radius;
  this->renderRectangle[5] = projectedY + cdl->radius;

  this->renderRectangle[6] = projectedX + cdl->radius;
  this->renderRectangle[7] = projectedY + cdl->radius;

  return this->currentRenderingIndex;
}

int CandlesManager::resetIndex()
{
  this->currentRenderingIndex = 0;
  return this->currentRenderingIndex;
}

void CandlesManager::setCenter(float center[2])
{
  this->centerUniform[0] = center[0];
  // 0.5 + (cdl->position[1] - 0.5) * inverseContainerRatio
  this->centerUniform[1] = 0.5 + (center[1] - 0.5) * inverseContainerRatio;
}

void CandlesManager::setScale(float scale)
{
  *this->scaleUniform = scale;
}

CandlesManager::~CandlesManager()
{
}
