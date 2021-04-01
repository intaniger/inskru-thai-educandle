#include <vector>
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
  int mergeRenderGroup(RenderGroup *grp);
  int mergeRenderGroup();
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

int CandlesManager::mergeRenderGroup()
{
  std::sort(
      this->groups.begin(),
      this->groups.end(),
      render_group_distance_less_than());

  this->merged_render_groups.clear();
  for (auto &&grp : this->groups)
  {
    // printf("%f, %f\n", grp->topleftCandle->position[0], grp->topleftCandle->position[1]);
    this->mergeRenderGroup(new RenderGroup(grp->topleftCandle));
  }

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
  this->mergeRenderGroup();
  // this->renderNextGroup();
}

int CandlesManager::mergeRenderGroup(RenderGroup *grp)
{
  // Assumed that merged_render_groups is sorted by distance;
  auto lower = std::lower_bound(
           this->merged_render_groups.begin(),
           this->merged_render_groups.end(),
           grp->distanceFromOrigin - (2 * 1.414 * grp->topleftCandle->radius),
           render_group_distance_less_than()),
       upper = std::upper_bound(
           this->merged_render_groups.begin(),
           this->merged_render_groups.end(),
           grp->distanceFromOrigin + (2 * 1.414 * grp->topleftCandle->radius),
           render_group_distance_greater_than());

  // printf("This is da grouppp \n");
  // for (auto &&grp : this->merged_render_groups)
  // {
  //   printf("%.2f\n", grp->distanceFromOrigin);
  // }

  for (auto rgrp = lower; rgrp != upper && rgrp != this->merged_render_groups.end(); ++rgrp)
  {

    auto existingRenderGroup = *rgrp;
    // printf("%.2f %.2f, %.2f\n", existingRenderGroup->topleftCandle->position[0], existingRenderGroup->topleftCandle->position[1], existingRenderGroup->distanceFromOrigin);
    if (existingRenderGroup->shouldMerge(grp))
    {
      existingRenderGroup->merge(grp);
      return this->merged_render_groups.size();
    }
  }
  this->merged_render_groups.push_back(grp);
  std::sort(
      this->merged_render_groups.begin(),
      this->merged_render_groups.end(),
      render_group_distance_less_than());

  return this->merged_render_groups.size();
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
