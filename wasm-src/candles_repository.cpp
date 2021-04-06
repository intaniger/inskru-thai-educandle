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

float findShortestDistance(float pt[2], float bound[4])
{
  float ptx = pt[0],
        pty = pt[1],
        bx1 = bound[0],
        bx2 = bound[2],
        by1 = bound[1],
        by2 = bound[3];

  if (between(ptx, bx1, bx2))
    if (between(pty, by1, by2))
      return 0;
    else
      return std::min(abs(pty - by1), abs(pty - by2));
  else if (between(pty, by1, by2))
    return std::min(abs(ptx - bx1), abs(ptx - bx2));
  else
    return length(
        std::min(abs(ptx - bx1), abs(ptx - bx2)),
        std::min(abs(pty - by1), abs(pty - by2)));
}

float findLongestDistance(float pt[2], float bound[4])
{
  float ptx = pt[0],
        pty = pt[1],
        bx1 = bound[0],
        bx2 = bound[2],
        by1 = bound[1],
        by2 = bound[3];

  return length(
      std::max(abs(ptx - bx1), abs(ptx - bx2)),
      std::max(abs(pty - by1), abs(pty - by2)));
}

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
  void queryCandlesInFrame(CandlesRange &r, float *bound);
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
      render_group_distance_less_than());
  return this->groups.size();
}

void CandlesRepository::queryCandlesInFrame(CandlesRange &r, float *bound)
{
  float transformedBound[4] = {
      bound[0],
      bound[1] * inverseContainerRatio,
      bound[2],
      bound[3] * inverseContainerRatio,
  };
  float minR = findShortestDistance(this->referencePoint, transformedBound),
        maxR = findLongestDistance(this->referencePoint, transformedBound);

  // printf("%f - %f:%f\n", minR, maxR, maxR + info->radius);
  // printf("%f,%f => %f,%f\n", transformedBound[0], transformedBound[1], transformedBound[2], transformedBound[3]);

  r.begin = std::lower_bound(this->groups.begin(), this->groups.end(), minR, render_group_distance_less_than());
  r.end = std::upper_bound(this->groups.begin(), this->groups.end(), maxR, render_group_distance_greater_than());

  // printf("(%lu,%lu) / %lu\n", r.begin - this->groups.begin(), r.end - this->groups.begin(), this->groups.size());
  // for (auto it = r.begin; it != r.end; it++)
  // {
  //   printf("Read: %f \n", (*it)->distanceFromOrigin);
  // }
}

CandlesRepository::~CandlesRepository()
{
}
