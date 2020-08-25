#include "w_create.h"
#include "m_math.h"
#include "w_world.h"

void w_GeneratePlayerShip(const int i)
{
    Vertex* verts = w_ObjectVertexBuffer + w_Geos[i].vertIndex;
    
    float r = 0.05;
    verts[0] = (Vec2){0.0, -r};
    verts[1] = (Vec2){-r/2, r/2};
    verts[2] = (Vec2){r/2, r/2};
    verts[3] = (Vec2){0.0, -r};
    w_Colliders[i].radius = r*2;
}

void w_GenerateAsteroidSquare(const int id)
{
    w_Geos[id].vertIndex = id * MAX_VERTS_PER_OBJ;
    w_Geos[id].vertCount = 5;
    Vertex* verts = w_ObjectVertexBuffer + w_Geos[id].vertIndex;

    float r = 0.05 + id * .01;
    verts[0] = (Vec2){r, r};
    verts[1] = (Vec2){-r, r};
    verts[2] = (Vec2){-r, -r};
    verts[3] = (Vec2){r, -r};
    verts[4] = (Vec2){r, r};
    w_Colliders[id].radius = r*2;
}

void w_GenerateAsteroidRand1(const int id, const float radius)
{
    const int vc = MAX_VERTS_PER_OBJ;
    Vertex* verts = w_ObjectVertexBuffer + w_Geos[id].vertIndex;

    for (int i = 0; i < vc - 1; i++) 
    {
        float r = radius + m_Rand() * radius;
        float angle = i * (M_PI * 2 / vc);
        verts[i] = m_PolarToCart(angle, r);
    }
    verts[vc - 1] = verts[0];
    w_Colliders[id].radius = 2 * radius;
}
