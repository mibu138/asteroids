#include "w_collision.h"
#include "m_math.h"

static bool testRaySegmentIntersect(Ray ray, Segment segment)
{
    return true;
}

static bool testPointInCircle(Vec2 point, Vec2 center, const float radius)
{
    m_Scale(-1, &center);
    m_Translate(center, &point);
    return (m_Length2(point) < radius * radius);
}

static bool testPointInGeo(const Vec2 point, const Geo geo)
{
    return true;
}

static bool testEmitInObject(int emitId, int objId)
{
    if (testPointInCircle(w_Emitables[emitId].pos, 
                w_Objects[objId].pos, 
                w_Colliders[objId].radius))
        return (testPointInGeo(w_Emitables[emitId].pos, w_Geos[objId]));
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
            if (testEmitInObject(i, j))
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
