#include <algorithm>
#include <vector>
#include <queue>
// #include "candle.cpp"
#include "render_group.cpp"

class CandlesManager
{
private:
  std::vector<RenderGroup *> groups;
  std::vector<RenderGroup *> merged_render_groups;
  size_t currentRenderingIndex;
  float inverseContainerRatio;

public:
  float constanlyUniforms[5];
  float *const screenRatioUniform = constanlyUniforms;
  float *const containerRatioUniform = constanlyUniforms + 1;
  float *const centerUniform = constanlyUniforms + 2;
  float *const scaleUniform = constanlyUniforms + 4;

  float *individualUniforms;
  float *lightPositions;
  float *renderRectangle;
  int lightNumber;

  // float *const lightNumberUniform = individualUniforms;
  // float *const luminanceUniform = individualUniforms + 1;
  // float *const radiusUniform = individualUniforms + 2;

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
                                inverseContainerRatio{objectHeight / objectWidth}
  {
    // 0.5 + (center[1] - 0.5) * inverseContainerRatio
    __refCandle = new Candle(0, 0.5 - 0.5 * inverseContainerRatio);
  };

  int getRenderGroupCounts();
  void registerCandle(float X, float Y);
  int mergeRenderGroups();
  int renderNextGroup();

  int resetIndex();
  void setCenter(float center[2]);
  void setScale(float scale);
  ~CandlesManager();
};

int CandlesManager::getRenderGroupCounts()
{
  return this->merged_render_groups.size();
}

int CandlesManager::mergeRenderGroups()
{
  std::sort(
      this->groups.begin(),
      this->groups.end(),
      render_group_distance_less_than());

  for (auto mgrp : this->merged_render_groups)
    delete mgrp;

  this->merged_render_groups.clear();

  std::priority_queue<RenderGroup *, std::vector<RenderGroup *>, render_group_distance_greater_than> waveFront;
  std::vector<RenderGroup *> temp;
  for (auto singleGroup : this->groups)
  {
    auto copiedGroup = new RenderGroup(singleGroup->topleftCandle);

    // while (!waveFront.empty())
    // {
    //   auto existingGroup = waveFront.top();
    //   temp.push_back(existingGroup);
    //   waveFront.pop();
    //   if (existingGroup->shouldMerge(copiedGroup))
    //   {
    //     existingGroup->merge(copiedGroup);
    //     break;
    //   }
    // }

    // for (auto poppedGroup : temp)
    //   // if (grp->distanceFromOrigin - poppedGroup->distanceFromOrigin < 2 * sqrt(2) * poppedGroup->bottomRightCandle->radius)
    //   waveFront.push(poppedGroup);

    for (auto existingGroup : temp)
    {
      if (existingGroup->shouldMerge(copiedGroup))
      {
        existingGroup->merge(copiedGroup);
        break;
      }
    }

    if (!copiedGroup->mergedWith)
    {
      temp.push_back(copiedGroup);
      waveFront.push(copiedGroup);
    }
  }

  for (auto i = temp.begin(); i != temp.end(); i++)
    for (auto j = temp.begin(); j != temp.end(); j++)
      if ((*j)->mergedWith == NULL && (*j)->shouldMerge(*i))
        (*j)->merge(*i);

  for (auto group : temp)
    if ((group)->mergedWith == NULL)
      this->merged_render_groups.push_back(group);

  return this->merged_render_groups.size();
}
void CandlesManager::registerCandle(float X, float Y)
{
  float projectedX = X,
        projectedY = 0.5 + (Y - 0.5) * inverseContainerRatio;

  // printf("%f, %f\n", projectedX, projectedY);
  Candle *candle = new Candle(projectedX, projectedY);
  RenderGroup *newGrp = new RenderGroup(candle);
  this->groups.push_back(newGrp);
  this->mergeRenderGroups();
  // this->renderNextGroup();
}

int CandlesManager::renderNextGroup()
{
  RenderGroup *group = this->merged_render_groups[this->currentRenderingIndex++];

  this->individualUniforms = group->individualUniforms;
  this->lightPositions = group->lightPositions;
  this->renderRectangle = group->renderRectangle;
  this->lightNumber = *group->lightNumberUniform;

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
  this->centerUniform[1] = 0.5 + (center[1] - 0.5) * inverseContainerRatio;
}

void CandlesManager::setScale(float scale)
{
  *this->scaleUniform = scale;
}

CandlesManager::~CandlesManager()
{
}
