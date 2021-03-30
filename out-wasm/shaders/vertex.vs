attribute highp vec2 position;

uniform highp vec2 container;
uniform highp vec2 container_offset;
uniform highp vec2 center;
uniform highp vec2 screen;

uniform highp float scale;

void main() {
    highp vec2 compound_ratio = container.xy * screen.yx;
    highp float factor = compound_ratio.x / compound_ratio.y;
    highp vec2 basis = vec2(factor, -1);

    highp vec2 axis_containerized_position = (position - container_offset) / container;
    highp vec2 translated_position = axis_containerized_position - center;
    highp vec2 u1_position = translated_position * basis;
    
    gl_Position = vec4(u1_position.xy * 2.0 * scale, 0, 1);
}