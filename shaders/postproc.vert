#version 460

vec2 verts[3] = {
    {-1, -1}, 
    {-1,  3},
    { 3, -1}
};

void main()
{
    gl_Position = vec4(verts[gl_VertexIndex], 0, 1);
}
