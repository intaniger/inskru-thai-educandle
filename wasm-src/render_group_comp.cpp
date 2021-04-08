#ifndef RENDER_GROUP_COMP
#define RENDER_GROUP_COMP
#endif
#ifndef RENDER_GROUP
#include "render_group.cpp"
#endif

struct render_group_br_distance_less_than
{
  inline bool operator()(RenderGroup *gr1, RenderGroup *gr2)
  {
    return (gr1->bottomRightDistanceFromOrigin < gr2->bottomRightDistanceFromOrigin);
  }
  inline bool operator()(RenderGroup *gr1, const float d)
  {
    return (gr1->bottomRightDistanceFromOrigin < d);
  }
  inline bool operator()(const float d, RenderGroup *gr1)
  {
    return (gr1->bottomRightDistanceFromOrigin < d);
  }
};

struct render_group_tl_distance_less_than
{
  inline bool operator()(RenderGroup *gr1, RenderGroup *gr2)
  {
    return (gr1->topLeftDistanceFromOrigin < gr2->topLeftDistanceFromOrigin);
  }
  inline bool operator()(RenderGroup *gr1, const float d)
  {
    return (gr1->topLeftDistanceFromOrigin < d);
  }
  inline bool operator()(const float d, RenderGroup *gr1)
  {
    return (gr1->topLeftDistanceFromOrigin < d);
  }
};

struct render_group_br_distance_greater_than
{
  inline bool operator()(const float d, RenderGroup *gr1)
  {
    return (gr1->bottomRightDistanceFromOrigin > d);
  }
  inline bool operator()(RenderGroup *gr1, const float d)
  {
    return (gr1->bottomRightDistanceFromOrigin > d);
  }
  inline bool operator()(RenderGroup *gr1, RenderGroup *gr2)
  {
    return (gr1->bottomRightDistanceFromOrigin > gr2->bottomRightDistanceFromOrigin);
  }
};

struct render_group_zIndex_less_than
{
  inline bool operator()(const unsigned int d, RenderGroup *gr1)
  {
    return (gr1->zIndex < d);
  }
  inline bool operator()(RenderGroup *gr1, RenderGroup *gr2)
  {
    return (gr1->zIndex < gr2->zIndex);
  }
};

struct render_group_br_distance_equal
{
  inline bool operator()(RenderGroup *gr1, RenderGroup *gr2)
  {
    return (gr1->bottomRightDistanceFromOrigin == gr2->bottomRightDistanceFromOrigin);
  }
};