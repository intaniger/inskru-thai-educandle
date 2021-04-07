#include <algorithm>
#include <vector>
#include <queue>
#include "candles_repository.cpp"
#ifndef RENDER_GROUP
#include "render_group.cpp"
#endif
#ifndef HELPER
#include "helper.cpp"
#endif

#define FORWARD_MERGE 1
#define BACKWARD_MERGE -1
#define RG_OPTIMIZE_FACTOR 0.8
#define RG_ADVANCE_FACTOR 0.2

class CandlesRenderer
{
private:
  std::vector<RenderGroup *> merged_render_groups;
  size_t currentRenderingIndex;
  float inverseContainerRatio;

  std::vector<RenderGroup *> pre_merged;
  ScreenViewState *viewState;
  CandlesRange range;

  float mergedGroupsBound[4];

  void toAdvancedBound(float (&)[4], const float *);

public:
  CandlesRepository *const candlesRepo;
  SterotypedLightInfo sLightInfo;
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

  CandlesRenderer(
      ScreenViewState *view,
      float screenWidth,
      float screenHeight,
      float objectWidth,
      float objectHeight,
      float center[2],
      float scale)
      : inverseContainerRatio{objectHeight / objectWidth},
        constanlyUniforms{
            // screen[1]
            screenWidth / screenHeight,
            // container[1]
            objectWidth / objectHeight,
            // center[2]
            center[0], center[1] * inverseContainerRatio,
            // scale[1]
            scale},
        sLightInfo{.radius = DEFAULT_RADIUS},
        candlesRepo{new CandlesRepository(objectWidth, objectHeight, &sLightInfo)},
        viewState{view} {};

  int getRenderGroupCounts();
  int prepareRender();
  int renderNextGroup();

  int mergeRenderGroups();
  std::vector<RenderGroup *>::iterator merge(std::vector<RenderGroup *>::iterator start, int direction);
  void merge(std::vector<RenderGroup *>::iterator start);

  void updateView();
  bool shouldUpdateRenderGroups(const float *);
  void updateRenderGroups(const float *);

  void registerCandle(float X, float Y);
  ~CandlesRenderer();
};

void CandlesRenderer::toAdvancedBound(float (&out)[4], const float *bound)
{
  out[0] = bound[0] - (RG_ADVANCE_FACTOR * (bound[2] - bound[0]));
  out[1] = bound[1] - (RG_ADVANCE_FACTOR * (bound[3] - bound[1]));
  out[2] = bound[2] + (RG_ADVANCE_FACTOR * (bound[2] - bound[0]));
  out[3] = bound[3] + (RG_ADVANCE_FACTOR * (bound[3] - bound[1]));
}

void CandlesRenderer::updateRenderGroups(const float *viewBound)
{
  this->toAdvancedBound(this->mergedGroupsBound, viewBound);
  this->candlesRepo->queryCandlesInFrame(this->range, this->mergedGroupsBound);
  this->mergeRenderGroups();
}

bool CandlesRenderer::shouldUpdateRenderGroups(const float *viewBound)
{
  float advBound[4];

  this->toAdvancedBound(advBound, viewBound);

  CandlesRange r;
  bool doesViewBoundIsSubsetToMergedBound = isFrameSubset(viewBound, mergedGroupsBound);

  if (!doesViewBoundIsSubsetToMergedBound)
  {
    // printf("This view is not subset of rendered one!\n");
    return true;
  }

  // re-fetch candles in current merged bound
  this->candlesRepo->queryCandlesInFrame(r, mergedGroupsBound);

  // if (r.begin != this->range.begin || r.end != this->range.end)
  if ((r.end - r.begin) != (this->range.end - this->range.begin))
  {
    // printf("Nah, some new candle(s)'d been added.\n");
    return true;
  }

  this->candlesRepo->queryCandlesInFrame(r, advBound);
  if ((r.end - r.begin) <= RG_OPTIMIZE_FACTOR * (this->range.end - this->range.begin))
  {
    // printf("Time to optimize! %lu<-%lu\n", (r.end - r.begin), (this->range.end - this->range.begin));
    return true;
  }

  return false;
}

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

  for (auto it = this->range.begin; it != this->range.end; it++)
  {
    this->pre_merged.push_back(new RenderGroup(*(*it)));
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

int CandlesRenderer::renderNextGroup()
{
  RenderGroup *group = this->merged_render_groups[this->currentRenderingIndex++];

  this->individualUniforms = group->individualUniforms;
  this->lightPositions = group->lightPositions;
  this->renderRectangle = group->renderRectangle;
  this->lightNumber = *group->lightNumberUniform;

  return this->currentRenderingIndex;
}

int CandlesRenderer::prepareRender()
{
  this->currentRenderingIndex = 0;
  return this->currentRenderingIndex;
}

void CandlesRenderer::updateView()
{
  float *center = this->viewState->getCenter();
  float scale = this->viewState->getScale();
  this->centerUniform[0] = center[0];
  this->centerUniform[1] = center[1] * inverseContainerRatio;

  *this->scaleUniform = scale;

  float *viewBound = this->viewState->getBound();
  if (this->shouldUpdateRenderGroups(viewBound))
    this->updateRenderGroups(viewBound);
}

CandlesRenderer::~CandlesRenderer()
{
}
