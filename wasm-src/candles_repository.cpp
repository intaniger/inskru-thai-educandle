#include <algorithm>
#ifndef RENDER_GROUP
#include "render_group.cpp"
#endif
#ifndef HELPER
#include "helper.cpp"
#endif
typedef struct
{
  std::vector<RenderGroup *>::iterator begin;
  std::vector<RenderGroup *>::iterator end;
} CandlesRange;

class CandlesRepository
{
private:
  float inverseContainerRatio;
  SterotypedLightInfo *info;
  float referencePoint[2];

public:
  std::vector<RenderGroup *> groups;

  CandlesRepository(float objectWidth,
                    float objectHeight,
                    SterotypedLightInfo *info)
      : referencePoint{0, 0},
        inverseContainerRatio{objectHeight / objectWidth},
        info{info} {};

  CandlesRepository(float referenceX,
                    float referenceY,
                    float objectWidth,
                    float objectHeight,
                    SterotypedLightInfo *info)
      : referencePoint{referenceX, referenceY},
        inverseContainerRatio{objectHeight / objectWidth},
        info{info} {};

  int registerCandle(float X, float Y);
  void queryCandlesInFrame(CandlesRange &r, const float *bound);
  ~CandlesRepository();
};

int CandlesRepository::registerCandle(float X, float Y)
{
  float projectedX = X,
        projectedY = Y * inverseContainerRatio;

  Candle *candle = new Candle(projectedX, projectedY);
  RenderGroup *newGrp = new RenderGroup(candle, this->referencePoint, info);
  this->groups.push_back(newGrp);
  std::sort(
      this->groups.begin(),
      this->groups.end(),
      render_group_br_distance_less_than());
  return this->groups.size();
}

void CandlesRepository::queryCandlesInFrame(CandlesRange &r, const float *bound)
{
  float transformedBound[4] = {
      bound[0],
      bound[1] * inverseContainerRatio,
      bound[2],
      bound[3] * inverseContainerRatio,
  };
  float minR = findShortestDistance(this->referencePoint, transformedBound),
        maxR = findLongestDistance(this->referencePoint, transformedBound);

  r.begin = std::lower_bound(this->groups.begin(), this->groups.end(), minR, render_group_br_distance_less_than());
  r.end = std::upper_bound(this->groups.begin(), this->groups.end(), maxR + 2 * info->radius, render_group_br_distance_greater_than());
}

CandlesRepository::~CandlesRepository()
{
}
