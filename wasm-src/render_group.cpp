#include <vector>
#include <math.h>
#include "candle.cpp"
#ifndef VERTEX_GROUP
#include "vertex_group.cpp"
#endif

Candle *__refCandle;

class RenderGroup
{
private:
  std::vector<Candle *> candles;
  VertexGroup *vg;
  float length(float x, float y)
  {
    return sqrt(x * x + y * y);
  };
  bool between(float x, float x1, float x2)
  {
    return (x1 <= x && x <= x2) || (x2 <= x && x <= x1);
  }
  bool isintersect(float *frame1, float *frame2)
  {
    return collapse(frame1, frame2) || collapse(frame2, frame1);
  }
  bool collapse(float *frame1, float *frame2)
  {
    bool didXbetween = between(frame2[0], frame1[0], frame1[2]) || between(frame2[2], frame1[0], frame1[2]),
         didYbetween = between(frame2[1], frame1[1], frame1[3]) || between(frame2[3], frame1[1], frame1[3]);

    return didXbetween && didYbetween;
  }
  float distance(float *br, float *tl)
  {
    float vector[2] = {
        br[0] - tl[0],
        br[1] - tl[1],
    };

    // if (vector[0] < 0 && vector[1] < 0)
    //   return 0;
    return length(vector[0], vector[1]);
  };

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
  void merge(RenderGroup *rhs);
  void updateUniform();
  ~RenderGroup();
};

bool RenderGroup::shouldMerge(RenderGroup *rhs)
{

  float *bound = this->getBoundary();
  float *rhsBound = rhs->getBoundary();

  return isintersect(bound, rhsBound);
}

void RenderGroup::merge(RenderGroup *rhs)
{
  RenderGroup *target = rhs;
  size_t offset = 0;
  while (target->mergedWith != NULL)
  {
    target = target->mergedWith;
    offset += 1;
  }

  if (target == this)
    return;

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