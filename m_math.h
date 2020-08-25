#ifndef M_MATH_H
#define M_MATH_H

#include <math.h>

#define PI 3.141592653589793

typedef struct {
    float x;
    float y;
} Vec2;

typedef struct {
    float x00, x01, x10, x11;
} Mat2;

typedef struct {
    Vec2 orig;
    Vec2 dir;
} Ray;

typedef struct {
    Vec2 A;
    Vec2 B;
} Segment;

void m_Rotate(const float angle /* radians */, Vec2*);
void m_Translate(const Vec2 t, Vec2*);
void m_Scale(const float scale, Vec2*);
void m_Add(const Vec2, Vec2*);
Vec2 m_Subtract(const Vec2, const Vec2);
//returns a random float between 0 and 1
float m_Rand(void); 
//returns a random float between -1 and 1
float m_RandNeg(void); 
float m_Length(const Vec2);
float m_Length2(const Vec2);
float m_Determinant(const Mat2);
Vec2  m_PolarToCart(const float angle, const float radius);

#endif /* end of include guard: M_MATH_H */
