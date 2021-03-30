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
  void AddVertex(float x, float y);
  float *getBoundary();
  ~VertexGroup();
};
void VertexGroup::EndAt(float *end)
{
  this->end = end;
}

void VertexGroup::AddVertex(float x, float y)
{

  // X-Axis
  this->rectangle[0][0] = this->rectangle[0][0] > x ? x : this->rectangle[0][0];
  this->rectangle[1][0] = this->rectangle[1][0] < x ? x : this->rectangle[1][0];

  // Y-Axis
  this->rectangle[0][1] = this->rectangle[0][1] < y ? y : this->rectangle[0][1];
  this->rectangle[1][1] = this->rectangle[1][1] > y ? y : this->rectangle[1][1];
}

float *VertexGroup::getBoundary()
{
  return &this->rectangle[0][0];
}

VertexGroup::~VertexGroup()
{
}
