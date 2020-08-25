#include "m_math.h"
#include <stdlib.h>
#include <math.h>

static void m_Mat2Vec2Mul(const Mat2* m, Vec2* v)
{
    float x = m->x00 * v->x + m->x01 * v->y;
    v->y = m->x10 * v->x + m->x11 * v->y;
    v->x = x;
}

void m_Rotate(const float angle, Vec2* v)
{
    Mat2 rot = {
        .x00 = cos(angle),
        .x01 = -sin(angle),
        .x10 = sin(angle),
        .x11 = cos(angle)
    };
    m_Mat2Vec2Mul(&rot, v);
//    v->x = v->x * cos(angle) - v->y * sin(angle);
//    v->y = v->x * sin(angle) + v->y * cos(angle);
}

void m_Translate(const Vec2 t, Vec2* v)
{
    v->x += t.x;
    v->y += t.y;
}

void m_Scale(const float s, Vec2* v)
{
    v->x *= s;
    v->y *= s;
}

float m_Length(const Vec2 v)
{
    return sqrt(m_Length2(v));
}

float m_Length2(const Vec2 v)
{
    return v.x * v.x + v.y * v.y;
}

void m_Add(const Vec2 v1, Vec2* v2)
{
    v2->x += v1.x;
    v2->y += v1.y;
}

Vec2 m_Subtract(const Vec2 v1, const Vec2 v2)
{
    Vec2 v;
    v.x = v1.x - v2.x;
    v.y = v1.y - v2.y;
    return v;
}

float m_Rand(void)
{
    return rand() / (float)RAND_MAX;
}

float m_RandNeg(void)
{
    float r = rand() / (float)RAND_MAX;
    return r * 2 - 1;
}

float m_Determinant(const Mat2 m)
{
    return m.x00 * m.x11 - m.x10 * m.x01;
}

Vec2 m_PolarToCart(const float angle, const float radius)
{
    Vec2 v = {
        cos(angle) * radius,
        sin(angle) * radius
    };
    return v;
}
