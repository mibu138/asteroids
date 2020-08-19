#version 460

layout(location = 0) in vec2 pos;

void main()
{
    gl_PointSize = 4.0;
    gl_Position = vec4(pos, 0.0, 1.0);
}
