#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#define KERN_SIZE 25
#define KERN_WIDTH KERN_SIZE / 2

#define STR_SIZE 100000

char ks[STR_SIZE];

float length(const float i, const float j)
{
    return sqrt(i * i + j * j);
}

float radiusMinusLengthNorm(const int i, const int j)
{
    const float r = (float)KERN_SIZE / 2.0;
    //const float r = length(w, w);
    const float val = (r - length(i, j)) / r;
    return val < 0 ? 0 : val;
}

float radiusMinusLengthNorm2(const int i, const int j)
{
    const float r = (float)KERN_SIZE / 2.0;
    //const float r = length(w, w);
    float val = (r - length(i, j)) / r;
    val = val < 0 ? 0 : val;
    return val * val;
}

float radiusMinusLengthNorm4(const int i, const int j)
{
    const float r = (float)KERN_SIZE / 2.0;
    //const float r = length(w, w);
    float val = (r - length(i, j)) / r;
    val = val < 0 ? 0 : val;
    return val * val * val * val;
}
 
int main(int argc, char *argv[])
{
    assert(KERN_SIZE % 2 == 1);
    int l = 0;
    l += sprintf(ks, "#define KERN_SIZE %d\n", KERN_SIZE);
    l += sprintf(ks + l, "#define KERN_WIDTH %d\n", KERN_WIDTH);
    l += sprintf(ks + l, "#define KERN_LEN %d\n\n", KERN_SIZE * KERN_SIZE);
    l += sprintf(ks + l, "float glow[KERN_LEN] = {\n");
    for (int i = -1 * KERN_WIDTH; i <= KERN_WIDTH; i++) 
    {
        l += sprintf(ks + l, "    ");
        for (int j = -1 * KERN_WIDTH; j <= KERN_WIDTH; j++) 
        {
            float value = radiusMinusLengthNorm(i, j);
            l += sprintf(ks + l, "%.9f, ", value); 
        }
        l += sprintf(ks + l, "\n");
    }
    l += sprintf(ks + l, "};\n");
    printf("%s", ks);
    return 0;
}
