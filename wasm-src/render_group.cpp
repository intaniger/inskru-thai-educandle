#ifndef RENDER_GROUP
#define RENDER_GROUP
#endif

#include <vector>
#include <math.h>
#include "candle.cpp"
#ifndef VERTEX_GROUP
#include "vertex_group.cpp"
#endif
#ifndef HELPER
#include "helper.cpp"
#endif

#define SELF_MERGED 0
#define OTHER_MERGED 1

#define NO_MERGE 0
#define COLLAPSE_ONLY 1
#define SHOULD_MERGE 3

#define Z_INDEX_UNCHANGED 0
#define Z_INDEX_UPDATED 1

typedef struct
{
  float radius;
} SterotypedLightInfo;

class RenderGroup
{
private:
  std::vector<Candle *> candles;
  VertexGroup *vg;
  bool isCornerCollapseWithAnyCandleRay(RenderGroup *rhs);
  float *refPosition;
  SterotypedLightInfo *sLightinfo;

public:
  float topLeftDistanceFromOrigin, bottomRightDistanceFromOrigin;
  unsigned int zIndex;
  RenderGroup *mergedWith;

  float individualUniforms[3];
  float *const lightNumberUniform = individualUniforms;
  float *const luminanceUniform = individualUniforms + 1;
  float *const radiusUniform = individualUniforms + 2;

  float lightPositions[256];
  float renderRectangle[8];
  Candle *topleftCandle, *bottomRightCandle;

  RenderGroup(RenderGroup &rg)
  {
    this->mergedWith = NULL;
    this->candles = rg.candles;
    this->topleftCandle = rg.topleftCandle;
    this->bottomRightCandle = rg.bottomRightCandle;
    this->zIndex = rg.zIndex;
    this->refPosition = rg.refPosition;
    this->sLightinfo = rg.sLightinfo;
    this->bottomRightDistanceFromOrigin = rg.bottomRightDistanceFromOrigin;
    this->topLeftDistanceFromOrigin = rg.topLeftDistanceFromOrigin;

    float *oldBound = rg.getBoundary();
    float bound[2][2] = {
        {oldBound[0], oldBound[1]},
        {oldBound[2], oldBound[3]},
    };
    this->vg = new VertexGroup(
        this->topleftCandle->position,
        bound[0]);
    this->vg->AddVertex(bound[1][0], bound[1][1]);

    this->updateUniform();
  }

  RenderGroup(Candle *firstCandle, float *refPosition, SterotypedLightInfo *sLightinfo)
  {
    float bound[2][2] = {
        {firstCandle->position[0] - sLightinfo->radius, firstCandle->position[1] - sLightinfo->radius},
        {firstCandle->position[0] + sLightinfo->radius, firstCandle->position[1] + sLightinfo->radius},
    };
    this->mergedWith = NULL;
    this->candles.push_back(firstCandle);

    this->vg = new VertexGroup(firstCandle->position, bound[0]);
    this->vg->AddVertex(bound[1][0], bound[1][1]);

    this->topLeftDistanceFromOrigin = distance(bound[0], refPosition);
    this->bottomRightDistanceFromOrigin = distance(bound[1], refPosition);
    this->topleftCandle = firstCandle;
    this->bottomRightCandle = firstCandle;
    this->zIndex = 1;
    this->refPosition = refPosition;
    this->sLightinfo = sLightinfo;

    this->updateUniform();
  };
  float *getBoundary();
  bool shouldMerge(RenderGroup *rhs);
  bool isCollapse(RenderGroup *rhs);
  int merge(RenderGroup *rhs);

  char updateZIndex(RenderGroup *rhs);
  void updateUniform();
  ~RenderGroup();
};

bool RenderGroup::isCornerCollapseWithAnyCandleRay(RenderGroup *rhs)
{
  float *rhsBound = rhs->getBoundary();

  for (auto &&thisCandle : this->candles)
  {
    if (findShortestDistance(thisCandle->position, rhsBound) <= sLightinfo->radius)
      return true;
    for (auto &&rCandle : rhs->candles)
      if (distance(thisCandle->position, rCandle->position) <= 2 * sLightinfo->radius)
        return true;
  }

  return false;
}

char RenderGroup::updateZIndex(RenderGroup *rhs)
{

  float *rhsBound = rhs->getBoundary();

  for (auto &&candle : this->candles)
  {
    if (findShortestDistance(candle->position, rhsBound) < sLightinfo->radius)
    {
      this->zIndex += rhs->zIndex;
      return Z_INDEX_UPDATED;
    }
  }

  return Z_INDEX_UNCHANGED;
}

bool RenderGroup::shouldMerge(RenderGroup *rhs)
{
  return isFrameCollapse(this->getBoundary(), rhs->getBoundary()) && this->isCornerCollapseWithAnyCandleRay(rhs);
}

bool RenderGroup::isCollapse(RenderGroup *rhs)
{
  return isFrameCollapse(this->getBoundary(), rhs->getBoundary());
}

int RenderGroup::merge(RenderGroup *rhs)
{
  RenderGroup *target = rhs;
  while (target->mergedWith != NULL)
    target = target->mergedWith;

  if (target == this)
    return SELF_MERGED;

  this->candles.insert(this->candles.end(), target->candles.begin(), target->candles.end());
  float *targetBound = target->vg->getBoundary();

  this->vg->AddVertex(targetBound[0], targetBound[1]);
  this->vg->AddVertex(targetBound[2], targetBound[3]);

  float oldTopLeftCandleDistance = distance(this->topleftCandle->position, refPosition);
  if (distance(target->topleftCandle->position, refPosition) < oldTopLeftCandleDistance)
    this->topleftCandle = target->topleftCandle;

  float oldBottomRightCandleDistance = distance(this->bottomRightCandle->position, refPosition);
  if (distance(target->bottomRightCandle->position, refPosition) > oldBottomRightCandleDistance)
    this->bottomRightCandle = target->bottomRightCandle;

  float *bound = this->getBoundary();
  this->topLeftDistanceFromOrigin = distance(&bound[0], this->refPosition);
  this->bottomRightDistanceFromOrigin = distance(&bound[2], this->refPosition);

  target->mergedWith = this;
  this->mergedWith = NULL;
  target->candles.clear();
  this->updateUniform();
  return OTHER_MERGED;
}

float *RenderGroup::getBoundary()
{
  return this->vg->getBoundary();
}

void RenderGroup::updateUniform()
{
  size_t i = 0;

  for (auto c = this->candles.begin(); c != this->candles.end() && i < 256; i += 2, ++c)
  {

    this->lightPositions[i] = (*c)->position[0];
    this->lightPositions[i + 1] = (*c)->position[1];
  }

  *this->lightNumberUniform = i / 2;
  *this->luminanceUniform = this->topleftCandle->luminance;
  *this->radiusUniform = sLightinfo->radius;

  float *bound = this->getBoundary();
  float leftX = bound[0],
        rightX = bound[2],
        topY = bound[1],
        bottomY = bound[3];

  this->renderRectangle[0] = leftX;
  this->renderRectangle[1] = topY;

  this->renderRectangle[2] = rightX;
  this->renderRectangle[3] = topY;

  this->renderRectangle[4] = leftX;
  this->renderRectangle[5] = bottomY;

  this->renderRectangle[6] = rightX;
  this->renderRectangle[7] = bottomY;
}

RenderGroup::~RenderGroup()
{
}

#ifndef RENDER_GROUP_COMP
#include "render_group_comp.cpp"
#endif