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
  bool collapse(float *frame1, float *frame2)
  {
    bool didXbetween = (frame1[0] < frame2[0] && frame2[0] < frame1[2]) || (frame1[0] < frame2[2] && frame2[2] < frame1[2]),
         didYbetween = (frame1[1] < frame2[1] && frame2[1] < frame1[3]) || (frame1[1] < frame2[3] && frame2[3] < frame1[3]);

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
  bool isMerged;

  float individualUniforms[3];
  float *const lightNumberUniform = individualUniforms;
  float *const luminanceUniform = individualUniforms + 1;
  float *const radiusUniform = individualUniforms + 2;

  float lightPositions[16];
  float renderRectangle[8];
  Candle *topleftCandle, *bottomRightCandle;

  RenderGroup(Candle *firstCandle)
  {
    this->isMerged = false;
    this->candles.push_back(firstCandle);
    this->vg = new VertexGroup(firstCandle->position);
    this->distanceFromOrigin = distance(firstCandle->position, __refCandle->position);
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
  float lightRadiusBottomRight = std::max(this->bottomRightCandle->radius, rhs->topleftCandle->radius),
        lightRadiusTopLeft = std::max(this->topleftCandle->radius, rhs->bottomRightCandle->radius);

  float *bound = this->getBoundary();
  float rect[4] = {
      bound[0] - lightRadiusTopLeft,
      bound[1] - lightRadiusTopLeft,
      bound[2] + lightRadiusBottomRight,
      bound[3] + lightRadiusBottomRight,
  };
  // float topleft[2] = {bound[0] - lightRadiusTopLeft, bound[1] - lightRadiusTopLeft};
  // float bottomright[2] = {bound[2] + lightRadiusBottomRight, bound[3] + lightRadiusBottomRight};

  float *rhsBound = rhs->getBoundary();
  float rhsRect[4] = {
      rhsBound[0] - lightRadiusTopLeft,
      rhsBound[1] - lightRadiusTopLeft,
      rhsBound[2] + lightRadiusBottomRight,
      rhsBound[3] + lightRadiusBottomRight,
  };
  // float rhsTopleft[2] = {rhsBound[0] - lightRadiusTopLeft, rhsBound[1] - lightRadiusTopLeft};
  // float rhsBottomright[2] = {rhsBound[2] + lightRadiusBottomRight, rhsBound[3] + lightRadiusBottomRight};

  return collapse(rect, rhsRect);
}

void RenderGroup::merge(RenderGroup *rhs)
{
  this->candles.insert(this->candles.end(), rhs->candles.begin(), rhs->candles.end());
  float *rhsBound = rhs->vg->getBoundary();

  auto change = this->vg->AddVertex(rhsBound[0], rhsBound[1]);
  if (change & TOPLEFT_CHANGED)
    this->topleftCandle = rhs->topleftCandle;
  if (change & BOTTOMRIGHT_CHANGED)
    this->bottomRightCandle = rhs->topleftCandle;

  change = this->vg->AddVertex(rhsBound[2], rhsBound[3]);
  if (change & TOPLEFT_CHANGED)
    this->topleftCandle = rhs->bottomRightCandle;
  if (change & BOTTOMRIGHT_CHANGED)
    this->bottomRightCandle = rhs->bottomRightCandle;

  this->distanceFromOrigin = distance(this->bottomRightCandle->position, __refCandle->position);
  rhs->isMerged = true;
  this->updateUniform();
}

float *RenderGroup::getBoundary()
{
  return this->vg->getBoundary();
}

void RenderGroup::updateUniform()
{
  size_t i = 0;

  *this->lightNumberUniform = this->candles.size();
  *this->luminanceUniform = this->topleftCandle->luminance;
  *this->radiusUniform = this->bottomRightCandle->radius;

  for (auto &&c : this->candles)
  {
    this->lightPositions[i] = c->position[0];
    this->lightPositions[i + 1] = c->position[1];

    i += 2;
  }

  float *bound = this->getBoundary();
  float leftX = bound[0] - this->topleftCandle->radius,
        rightX = bound[2] + this->topleftCandle->radius,
        topY = bound[1] - this->topleftCandle->radius,
        bottomY = bound[3] + this->topleftCandle->radius;

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
};
struct render_group_distance_equal
{
  inline bool operator()(RenderGroup *gr1, RenderGroup *gr2)
  {
    return (gr1->distanceFromOrigin == gr2->distanceFromOrigin);
  }
};