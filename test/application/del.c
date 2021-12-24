#include <SDL2/SDL.h>
#include <stdio.h>

int main(void)
{
    /*SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO);*/
    printf("hello world\n");
    SDL_Log("Log works\n");
    return 0;
}
