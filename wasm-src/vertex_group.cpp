#ifndef VERTEX_GROUP
#define VERTEX_GROUP
#endif

#define NOTHING_CHANGED 0
#define TOPLEFT_CHANGED 1
#define BOTTOMRIGHT_CHANGED 2
#define BOTH_CHANGED TOPLEFT_CHANGED | BOTTOMRIGHT_CHANGED

class VertexGroup
{
private:
  float rectangle[2][2]; // upper-left to bottom-right
  float *start;
  float *end;

public:
  VertexGroup(float *start) : rectangle{
                                  {start[0], start[1]},
                                  {start[0], start[1]},
                              },
                              start{start} {};
  VertexGroup(float *start, float *initVal) : rectangle{
                                                  {initVal[0], initVal[1]},
                                                  {initVal[0], initVal[1]},
                                              },
                                              start{start} {};
  void EndAt(float *end);
  int AddVertex(float x, float y);
  float *getBoundary();
  ~VertexGroup();
};
void VertexGroup::EndAt(float *end)
{
  this->end = end;
}

int VertexGroup::AddVertex(float x, float y)
{

  float currentTopleftX = this->rectangle[0][0],
        currentTopleftY = this->rectangle[0][1],
        currentBottomRightX = this->rectangle[1][0],
        currentBottomRightY = this->rectangle[1][1];

  // X-Axis
  this->rectangle[0][0] = currentTopleftX > x ? x : currentTopleftX;
  this->rectangle[1][0] = currentBottomRightX < x ? x : currentBottomRightX;

  // Y-Axis
  this->rectangle[0][1] = currentTopleftY > y ? y : currentTopleftY;
  this->rectangle[1][1] = currentBottomRightY < y ? y : currentBottomRightY;

  int change = NOTHING_CHANGED;
  if (currentTopleftX > x || currentTopleftY > y)
    change |= TOPLEFT_CHANGED;
  if (currentBottomRightX < x || currentBottomRightY < y)
    change |= BOTTOMRIGHT_CHANGED;

  return change;
}

float *VertexGroup::getBoundary()
{
  return &this->rectangle[0][0];
}

VertexGroup::~VertexGroup()
{
}
