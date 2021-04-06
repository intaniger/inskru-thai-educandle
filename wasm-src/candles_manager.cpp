#include <algorithm>
#include <vector>
#include <queue>
// #include "candle.cpp"
#include "render_group.cpp"
#ifndef HELPER
#include "helper.cpp"
#endif

#define FORWARD_MERGE 1
#define BACKWARD_MERGE -1

class CandlesRenderer
{
private:
  std::vector<RenderGroup *> groups;
  std::vector<RenderGroup *> merged_render_groups;
  size_t currentRenderingIndex;
  float inverseContainerRatio;

  std::vector<RenderGroup *> pre_merged;

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

  CandlesRenderer(float screenWidth,
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
  std::vector<RenderGroup *>::iterator merge(std::vector<RenderGroup *>::iterator start, int direction);
  void merge(std::vector<RenderGroup *>::iterator start);

  ~CandlesRenderer();
};

int CandlesRenderer::getRenderGroupCounts()
{
  return this->merged_render_groups.size();
}

void CandlesRenderer::merge(std::vector<RenderGroup *>::iterator start)
{
  this->merge(start, FORWARD_MERGE);
}

std::vector<RenderGroup *>::iterator CandlesRenderer::merge(std::vector<RenderGroup *>::iterator start, int direction)
{
  // assumed that pre_merged is sorted asc-ly before.
  auto tainted = this->pre_merged.end(),
       term = direction == FORWARD_MERGE ? this->pre_merged.end() : this->pre_merged.begin() - 1;

  for (auto it = start; it != term; it += direction)
  {
    RenderGroup *otherGroup, *consideringGroup;
    float *consideringBound, *othersBound;
    float consideringBoundDistance;

    consideringGroup = (*it);
    consideringBound = consideringGroup->getBoundary();
    consideringBoundDistance = distance(&consideringBound[0], &consideringBound[2]);

    if (consideringGroup->mergedWith != NULL)
      continue;

    otherGroup = consideringGroup;
    othersBound = otherGroup->getBoundary();

    for (
        auto dit = it;
        !(dit == term ||
          abs(otherGroup->distanceFromOrigin - consideringGroup->distanceFromOrigin) >
              std::max<float>(consideringBoundDistance, distance(&othersBound[0], &othersBound[2])));
        dit += direction)
    {
      otherGroup = (*dit);
      othersBound = otherGroup->getBoundary();

      if (otherGroup->mergedWith != NULL)
        continue;

      switch (otherGroup->shouldMerge(consideringGroup))
      {
      case SHOULD_MERGE:
        if (otherGroup->merge(consideringGroup) == OTHER_MERGED)
          tainted = dit;
        break;

      case COLLAPSE_ONLY:
        if (otherGroup->updateZIndex(consideringGroup) == Z_INDEX_UNCHANGED)
          consideringGroup->updateZIndex(otherGroup);
        break;

      default:
        break;
      }
    }
  }
  return tainted == this->pre_merged.end() ? tainted : this->merge(tainted, direction * -1);
}

int CandlesRenderer::mergeRenderGroups()
{
  for (auto mgrp : this->pre_merged)
    delete mgrp;

  this->merged_render_groups.clear();
  this->pre_merged.clear();

  for (auto singleGroup : this->groups)
  {
    auto g = new RenderGroup(singleGroup->topleftCandle);
    this->pre_merged.push_back(g);
  }

  std::sort(
      this->pre_merged.begin(),
      this->pre_merged.end(),
      render_group_distance_less_than());

  this->merge(this->pre_merged.begin());

  for (auto group : pre_merged)
    if ((group)->mergedWith == NULL)
      this->merged_render_groups.push_back(group);

  std::sort(
      this->merged_render_groups.begin(),
      this->merged_render_groups.end(),
      render_group_zIndex_less_than());

  return this->merged_render_groups.size();
}
void CandlesRenderer::registerCandle(float X, float Y)
{
  float projectedX = X,
        projectedY = 0.5 + (Y - 0.5) * inverseContainerRatio;

  // printf("%f, %f\n", projectedX, projectedY);
  Candle *candle = new Candle(projectedX, projectedY);
  RenderGroup *newGrp = new RenderGroup(candle);
  this->groups.push_back(newGrp);
  this->mergeRenderGroups();
}

int CandlesRenderer::renderNextGroup()
{
  RenderGroup *group = this->merged_render_groups[this->currentRenderingIndex++];

  this->individualUniforms = group->individualUniforms;
  this->lightPositions = group->lightPositions;
  this->renderRectangle = group->renderRectangle;
  this->lightNumber = *group->lightNumberUniform;

  return this->currentRenderingIndex;
}

int CandlesRenderer::resetIndex()
{
  this->currentRenderingIndex = 0;
  return this->currentRenderingIndex;
}

void CandlesRenderer::setCenter(float center[2])
{
  this->centerUniform[0] = center[0];
  this->centerUniform[1] = 0.5 + (center[1] - 0.5) * inverseContainerRatio;
}

void CandlesRenderer::setScale(float scale)
{
  *this->scaleUniform = scale;
}

CandlesRenderer::~CandlesRenderer()
{
}
