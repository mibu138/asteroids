#ifndef W_COLLISION_H
#define W_COLLISION_H

#include "w_world.h"

typedef struct {
    bool collision;
    int  object1;
    int  object2;
} HitInfo;

HitInfo w_DetectBulletObjectCols(void);

#endif /* end of include guard: W_COLLISION_H */
