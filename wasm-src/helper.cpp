#include <math.h>
#ifndef HELPER
#define HELPER
#endif

float length(float x, float y)
{
  return sqrt(x * x + y * y);
};

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

bool between(float x, float x1, float x2)
{
  return (x1 <= x && x <= x2) || (x2 <= x && x <= x1);
}
bool collapse(float *frame1, float *frame2)
{
  bool didXbetween = between(frame2[0], frame1[0], frame1[2]) || between(frame2[2], frame1[0], frame1[2]),
       didYbetween = between(frame2[1], frame1[1], frame1[3]) || between(frame2[3], frame1[1], frame1[3]);

  return didXbetween && didYbetween;
}
bool isFrameCollapse(float *frame1, float *frame2)
{
  return collapse(frame1, frame2) || collapse(frame2, frame1);
}