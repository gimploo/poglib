#pragma once
#include "../decl.h"

void __uielem_destroy(uielem_t *elem)
{
    free(elem->value);
    elem->value = NULL;
    elem->__font = NULL;
    glbatch_destroy(&elem->__textbatch);
}
void __uielem_update(uielem_t *ui)
{
    if (!ui->__is_changed) return;

    printf("UPDATING UI (%i)\n", ui->type);
    ui->update(ui);
    ui->__is_changed = false;
}
