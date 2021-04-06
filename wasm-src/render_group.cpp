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
  float distanceFromOrigin;
  unsigned int zIndex;
  RenderGroup *mergedWith;

  float individualUniforms[3];
  float *const lightNumberUniform = individualUniforms;
  float *const luminanceUniform = individualUniforms + 1;
  float *const radiusUniform = individualUniforms + 2;

  float lightPositions[256];
  float renderRectangle[8];
  Candle *topleftCandle, *bottomRightCandle;

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

    this->distanceFromOrigin = distance(bound[1], refPosition);
    this->topleftCandle = firstCandle;
    this->bottomRightCandle = firstCandle;
    this->zIndex = 1;
    this->refPosition = refPosition;
    this->sLightinfo = sLightinfo;

    this->updateUniform();
  };
  float *getBoundary();
  char shouldMerge(RenderGroup *rhs);
  int merge(RenderGroup *rhs);

  char updateZIndex(RenderGroup *rhs);
  void updateUniform();
  ~RenderGroup();
};

bool RenderGroup::isCornerCollapseWithAnyCandleRay(RenderGroup *rhs)
{
  float *rhsBound = rhs->getBoundary();

  float corners[4][2] = {
      {rhsBound[0], rhsBound[1]},
      {rhsBound[0], rhsBound[3]},
      {rhsBound[2], rhsBound[1]},
      {rhsBound[2], rhsBound[3]},
  };
  for (auto &&c : this->candles)
    for (auto corner : corners)
      if (
          distance(c->position, corner) <= sLightinfo->radius ||
          distance(c->position, rhs->topleftCandle->position) <= 2 * sLightinfo->radius ||
          distance(c->position, rhs->bottomRightCandle->position) <= 2 * sLightinfo->radius)
        return true;
  return false;
}

char RenderGroup::updateZIndex(RenderGroup *rhs)
{

  float *rhsBound = rhs->getBoundary();

  for (auto &&candle : this->candles)
  {
    float bound[4] = {
        candle->position[0] - sLightinfo->radius,
        candle->position[1] - sLightinfo->radius,
        candle->position[0] + sLightinfo->radius,
        candle->position[1] + sLightinfo->radius,
    };
    if (isFrameCollapse(bound, rhsBound))
    {
      // this->zIndex = rhs->zIndex + 1;
      // rhs->zIndex = this->zIndex + 1;
      this->zIndex += rhs->zIndex;
      return Z_INDEX_UPDATED;
    }
  }

  return Z_INDEX_UNCHANGED;
}

char RenderGroup::shouldMerge(RenderGroup *rhs)
{

  float *bound = this->getBoundary();
  float *rhsBound = rhs->getBoundary();

  return isFrameCollapse(bound, rhsBound) ? isCornerCollapseWithAnyCandleRay(rhs) ? SHOULD_MERGE : COLLAPSE_ONLY : NO_MERGE;
}

int RenderGroup::merge(RenderGroup *rhs)
{
  RenderGroup *target = rhs;
  size_t offset = 0;
  while (target->mergedWith != NULL)
  {
    target = target->mergedWith;
    offset += 1;
  }

  if (target == this)
    return SELF_MERGED;

  this->candles.insert(this->candles.end(), target->candles.begin(), target->candles.end());
  float *targetBound = target->vg->getBoundary();

  auto change = this->vg->AddVertex(targetBound[0], targetBound[1]);
  if (change & TOPLEFT_CHANGED)
    this->topleftCandle = target->topleftCandle;
  if (change & BOTTOMRIGHT_CHANGED)
    this->bottomRightCandle = target->topleftCandle;

  change = this->vg->AddVertex(targetBound[2], targetBound[3]);
  if (change & TOPLEFT_CHANGED)
    this->topleftCandle = target->bottomRightCandle;
  if (change & BOTTOMRIGHT_CHANGED)
    this->bottomRightCandle = target->bottomRightCandle;

  float *bound = this->getBoundary();
  this->distanceFromOrigin = distance(&bound[2], this->refPosition);

  target->mergedWith = this;
  this->mergedWith = NULL;
  target->candles.clear();

  this->zIndex = std::min<unsigned int>(this->zIndex, rhs->zIndex);
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