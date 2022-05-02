#pragma once
#include "../decl.h"
#include "./uielem.h"

void __label_update(uielem_t *uielem)
{
    (void)uielem;
}

label_t label_init(void)
{
    return (label_t ) {
        .textcolor = COLOR_WHITE
    };
}
