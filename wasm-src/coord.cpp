#include <algorithm>
#ifndef COORD
#define COORD
#endif

class Coord
{
private:
  float xFactor, coords[2];

public:
  Coord(float coords[2], float regionRatio, float screenRatio) : coords{coords[0], coords[1]}, xFactor{screenRatio / regionRatio} {};
  ~Coord();
  const float operator[](size_t i)
  {
    return this->coords[i];
  }

  friend Coord operator+(Coord lhs, float operand[2])
  {
    lhs += operand;
    return lhs;
  }
  // Coord &operator+(float squareOperand)
  // {
  //   this->coords[0] += squareOperand * xFactor;
  //   this->coords[1] += squareOperand;
  //   return *this;
  // }
  Coord &operator+=(float operand[2])
  {
    this->coords[0] += operand[0] * xFactor;
    this->coords[1] += operand[1];
    return *this;
  }

  Coord &operator=(const Coord &rhs)
  {
    this->coords[0] = rhs.coords[0];
    this->coords[1] = rhs.coords[1];
    this->xFactor = rhs.xFactor;
    return *this;
  }
};

Coord::~Coord()
{
}
