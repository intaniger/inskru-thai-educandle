#include "coord.cpp"

class ScreenViewState
{
private:
  Coord *center;
  float scale;

public:
  ScreenViewState(float center[2], float scale, float regionRatio, float screenRatio) : center{new Coord(center, regionRatio, screenRatio)}, scale{scale} {};
  ~ScreenViewState();
  void ZoomTo(float X, float Y, float scale);
  void RelMoveTo(float deltaX, float deltaY);
  float getScale();
  float *getCenter();
  float *getContextPosition(float X, float Y);
  float *getBound();
  float _center[2];
  float _contextPosition[2];
  float _bound[4];
};

void ScreenViewState::ZoomTo(float X, float Y, float step)
{
  float viewBoxScale = 1 / this->scale;

  float mouseDiff[2] = {
      static_cast<float>((X - 0.5) * viewBoxScale * (1 - 1 / step)),
      static_cast<float>((Y - 0.5) * viewBoxScale * (1 - 1 / step)),
  };

  *this->center += mouseDiff;
  this->scale = this->scale * step;
}

void ScreenViewState::RelMoveTo(float deltaX, float deltaY)
{
  float viewBoxScale = 1 / this->scale;

  float mouseDiff[2] = {
      deltaX * viewBoxScale,
      deltaY * viewBoxScale,
  };

  *this->center += mouseDiff;
}

float *ScreenViewState::getContextPosition(float X, float Y)
{
  float viewBoxScale = 1 / this->scale;
  float diff[2] = {
      static_cast<float>((X - 0.5) * viewBoxScale),
      static_cast<float>((Y - 0.5) * viewBoxScale),
  };
  Coord c = *(this->center) + diff;
  this->_contextPosition[0] = c[0];
  this->_contextPosition[1] = c[1];
  return this->_contextPosition;
}

float *ScreenViewState::getBound()
{
  float viewBoxScale = 1 / this->scale;
  float topLeftDiff[2] = {
      static_cast<float>(-0.5 * viewBoxScale),
      static_cast<float>(-0.5 * viewBoxScale),
  };
  float rightBottomDiff[2] = {
      static_cast<float>(0.5 * viewBoxScale),
      static_cast<float>(0.5 * viewBoxScale),
  };
  Coord tlc = *(this->center) + topLeftDiff,
        rbc = *(this->center) + rightBottomDiff;

  this->_bound[0] = tlc[0];
  this->_bound[1] = tlc[1];
  this->_bound[2] = rbc[0];
  this->_bound[3] = rbc[1];

  return this->_bound;
}

float ScreenViewState::getScale()
{
  return this->scale;
}

float *ScreenViewState::getCenter()
{
  this->_center[0] = (*this->center)[0];
  this->_center[1] = (*this->center)[1];
  return this->_center;
}

ScreenViewState::~ScreenViewState()
{
}
