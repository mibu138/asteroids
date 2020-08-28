#version 460

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform sampler2D inTex;

void main()
{
    vec2 uv =  gl_FragCoord.xy / 1000.0;
    outColor = texture(inTex, uv);
}
