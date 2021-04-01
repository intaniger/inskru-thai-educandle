#define DEFAULT_LUMINANCE 0.5
#define DEFAULT_RADIUS 0.05

class Candle
{
public:
  float position[2];
  float luminance, radius;
  Candle(float position[2], float luminance, float radius) : position{position[0], position[1]}, luminance{luminance}, radius{radius} {};
  Candle(float position[2]) : position{position[0], position[1]}, luminance{DEFAULT_LUMINANCE}, radius{DEFAULT_RADIUS} {};
  Candle(float X, float Y) : position{X, Y}, luminance{DEFAULT_LUMINANCE}, radius{DEFAULT_RADIUS} {};
  ~Candle();
};

Candle::~Candle()
{
}
