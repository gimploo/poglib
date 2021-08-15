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

    memcpy(indices, DEFAULT_QUAD_INDICES, sizeof(DEFAULT_QUAD_INDICES));

    for (int i = DEFAULT_QUAD_INDICES_CAPACITY, index = 0; 
            i < count; 
            i++, index++)
    {
        indices[i] = 3 + DEFAULT_QUAD_INDICES[index % 4];
    }

    for (int i = 0; i < count; i++)
        printf("%i ", list.array[i]);
    printf("\n");



    return 0;
}
