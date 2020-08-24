#include "w_collision.h"
#include "m_math.h"

static bool raySegmentIntersect(Ray ray, Segment segment)
{
    const Vec2 v1 = m_Subtract(ray.orig, segment.A);
    const Vec2 d1 = ray.dir;
    const Vec2 d2 = m_Subtract(segment.B, segment.A);
    const Mat2 m1 = {
        v1.x, d2.x,
        v1.y, d2.y
    };
    const Mat2 m2 = {
        v1.x, d1.x,
        v1.y, d1.y
    };
    const Mat2 m3 = {
        d2.x, d1.x,
        d2.y, d1.y
    };
    const float denom = m_Determinant(m3);
    if (denom == 0) return false;
    const float t1 = m_Determinant(m1) / denom;
    const float t2 = m_Determinant(m2) / denom;
    if (t1 < 0) return false;
    if (t2 < 0 || t2 > 1) return false;
    return true;
}

static bool pointInCircle(Vec2 point, Vec2 center, const float radius)
{
    m_Scale(-1, &center);
    m_Translate(center, &point);
    return (m_Length2(point) < radius * radius);
}

static bool pointInGeo(const Vec2 point, const Geo geo)
{
    int intersections = 0;
    const Vertex* verts = &w_ObjectVertexBuffer[geo.vertIndex];
    Ray ray = {
        .orig = point,
        .dir  = {1, 0}
    };
    for (int i = 0; i < geo.vertCount - 1; i++) 
    {
        Segment segment = {
            .A = verts[i],
            .B = verts[i+1]
        };
        if (raySegmentIntersect(ray, segment)) 
            intersections++;
    }
    if (intersections % 2 == 0)
        return false;
    return true;
}

static bool emitInObject(int emitId, int objId)
{
    if (pointInCircle(w_Emitables[emitId].pos, 
                w_Objects[objId].pos, 
                w_Colliders[objId].radius))
        return (pointInGeo(w_Emitables[emitId].pos, w_Geos[objId]));
    return false;
}

HitInfo w_DetectBulletObjectCols(void)
{
    HitInfo hitInfo;
    const int emitCount = w_EmitableCount;
    const int objCount = w_ObjectCount;
    for (int i = 0; i < emitCount; i++) 
    {
        // start at 1 becuase 0 is player
        for (int j = 1; j < objCount; j++) 
        {
            if (emitInObject(i, j))
            {
                hitInfo.collision = true;
                hitInfo.object1 = i;
                hitInfo.object2 = j;
                return hitInfo;
            }
        }
    }
    hitInfo.collision = false;
    return hitInfo;
}
