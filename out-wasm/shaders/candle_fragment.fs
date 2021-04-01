const int MAX_SIZE = 32;

uniform highp vec2 lightPositions[MAX_SIZE];
uniform int lightNumbers;

uniform lowp float lum;
uniform highp float radius;

varying highp vec2 frag_position;
const lowp vec4 lightColor = vec4(1.0, 0.7607843137254902, 0.058823529411764705, 1);
// const lowp vec4 bgColor = vec4(0, 0, 0, 1);
const lowp vec4 bgColor = vec4(1, 0, 0, 1);

highp vec4 drawLight(highp vec2 p, highp vec2 pos, lowp vec4 color, mediump float range)
{ 
	// distance to light
	highp float ld = length(p - pos);

  // scaled radius
  highp float r = range;

  if(ld > r) return vec4(0.0);
  
	highp float fall = (r - ld)/r;
	fall *= fall;

  return vec4(1.0);
	// return fall * color;
}

highp float luminance(vec4 col)
{
	return 0.2126 * col.r + 0.7152 * col.g + 0.0722 * col.b;
}

lowp vec4 setLuminance(vec4 col, float lum)
{
	lum /= luminance(col);
	col *= lum;
  return col;
}

void main(){
  lowp vec4 light = setLuminance(lightColor, lum);
  gl_FragColor = bgColor;

  for(int i = 0; i < MAX_SIZE; i++) {
    gl_FragColor += clamp(float(lightNumbers) - float(i), 0., 1.) * drawLight(frag_position, lightPositions[i], light, radius);
  }
}