#include <stdio.h>
#include <string.h>

#define DEFAULT_QUAD_INDICES_CAPACITY 6
const unsigned int DEFAULT_QUAD_INDICES[] = {
   0, 1, 2, 2, 3, 0                         
};                                  

typedef struct {
    unsigned int *array;
} buffer;


int main(void)
{
    unsigned int indices[12]; 
    unsigned int count = 12;

    buffer list = {
        .array = indices
    };


    int shape_count = 2;
    memcpy(indices, DEFAULT_QUAD_INDICES, sizeof(DEFAULT_QUAD_INDICES));
    for (int i = 1; i < shape_count; i++)
    {
        indices[(i*6) + 0]   = DEFAULT_QUAD_INDICES[0] + 4; 
        indices[(i*6) + 1]   = DEFAULT_QUAD_INDICES[1] + 4;
        indices[(i*6) + 2]   = DEFAULT_QUAD_INDICES[2] + 4;
        indices[(i*6) + 3]   = DEFAULT_QUAD_INDICES[3] + 4;
        indices[(i*6) + 4]   = DEFAULT_QUAD_INDICES[4] + 4;
        indices[(i*6) + 5]   = DEFAULT_QUAD_INDICES[5] + 4;
    }
        

    for (int i = 0; i < count; i++)
        printf("%i ", list.array[i]);
    printf("\n");



    return 0;
}
