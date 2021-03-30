attribute highp vec2 coords;

uniform highp float screen_ratio;
uniform highp float container_ratio;

uniform highp vec2 center;
uniform highp float scale;

varying vec2 frag_position;

void main(){
  // highp vec2 compound_ratio = container.xy * screen.yx;
  highp float factor = container_ratio / screen_ratio;
  highp vec2 basis = factor * vec2(1, -screen_ratio);


  highp vec2 translated_position = coords.xy - center;
  highp vec2 u1_position = translated_position * basis;
  
  frag_position = coords.xy;
  
  gl_Position = vec4(u1_position * 2.0 * scale, 0, 1);
}