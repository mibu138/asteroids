#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#define KERN_SIZE 7
#define KERN_WIDTH KERN_SIZE / 2

#define STR_SIZE  512

char ks[STR_SIZE];

float length(const float i, const float j)
{
    return sqrt(i * i + j * j);
}

float radiusMinusLengthNorm(const int i, const int j)
{
    float r = (float)KERN_SIZE / 2;
    return (r - length(i, j)) / r;
}

int main(int argc, char *argv[])
{
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
            l += sprintf(ks + l, "%f, ", value); 
        }
        l += sprintf(ks + l, "\n");
    }
    l += sprintf(ks + l, "};\n");
    printf("%s", ks);
    return 0;
}
