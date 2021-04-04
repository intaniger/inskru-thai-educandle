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

Candle *__refCandle;

class RenderGroup
{
private:
  std::vector<Candle *> candles;
  VertexGroup *vg;

public:
  float distanceFromOrigin;
  RenderGroup *mergedWith;

  float individualUniforms[3];
  float *const lightNumberUniform = individualUniforms;
  float *const luminanceUniform = individualUniforms + 1;
  float *const radiusUniform = individualUniforms + 2;

  float lightPositions[256];
  float renderRectangle[8];
  Candle *topleftCandle, *bottomRightCandle;

  RenderGroup(Candle *firstCandle)
  {
    float bound[2][2] = {
        {firstCandle->position[0] - firstCandle->radius, firstCandle->position[1] - firstCandle->radius},
        {firstCandle->position[0] + firstCandle->radius, firstCandle->position[1] + firstCandle->radius},
    };
    this->mergedWith = NULL;
    this->candles.push_back(firstCandle);

    this->vg = new VertexGroup(firstCandle->position, bound[0]);
    this->vg->AddVertex(bound[1][0], bound[1][1]);

    this->distanceFromOrigin = distance(bound[1], __refCandle->position);
    // length(firstCandle->position[0] - __refCandle->position[0], firstCandle->position[1] - __refCandle->position[1]);
    this->topleftCandle = firstCandle;
    this->bottomRightCandle = firstCandle;

    this->updateUniform();
  };
  float *getBoundary();
  bool shouldMerge(RenderGroup *rhs);
  int merge(RenderGroup *rhs);
  void updateUniform();
  ~RenderGroup();
};

bool RenderGroup::shouldMerge(RenderGroup *rhs)
{

  float *bound = this->getBoundary();
  float *rhsBound = rhs->getBoundary();

  return isintersect(bound, rhsBound);
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
  this->distanceFromOrigin = distance(&bound[2], __refCandle->position);

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
  *this->radiusUniform = this->bottomRightCandle->radius;

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

struct render_group_distance_less_than
{
  inline bool operator()(RenderGroup *gr1, RenderGroup *gr2)
  {
    return (gr1->distanceFromOrigin < gr2->distanceFromOrigin);
  }
  inline bool operator()(RenderGroup *gr1, const float d)
  {
    return (gr1->distanceFromOrigin < d);
  }
  inline bool operator()(const float d, RenderGroup *gr1)
  {
    return (gr1->distanceFromOrigin < d);
  }
};
struct render_group_distance_greater_than
{
  inline bool operator()(const float d, RenderGroup *gr1)
  {
    return (gr1->distanceFromOrigin > d);
  }
  inline bool operator()(RenderGroup *gr1, RenderGroup *gr2)
  {
    return (gr1->distanceFromOrigin > gr2->distanceFromOrigin);
  }
};
struct render_group_distance_equal
{
  inline bool operator()(RenderGroup *gr1, RenderGroup *gr2)
  {
    return (gr1->distanceFromOrigin == gr2->distanceFromOrigin);
  }
};