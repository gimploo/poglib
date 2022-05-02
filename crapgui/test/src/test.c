#include <poglib/crapgui.h>


void appinit(application_t *app) 
{
    crapgui_t *gui = app->content;
    *gui = crapgui_init();

    crapgui_layout(gui) {

        frame("FRAME01") {
            /*label("Test");*/
            button("Button01"); button("Button02");
            button("Button03"); button("Button04");
            label("Confirm"); button("Submit");
            label("THIS IS A LABEL");
        }

        frame("FRAME02") {
            label("Confirm"); button("Submit");
            label("THIS IS A LABEL");
        }
        frame("FRAME03") {
            label("THIS IS A LABEL");
        }

        frame("FRAME04") {
            button("Button03"); button("Button04");
        }

    };
}

void appupdate(application_t *app) 
{
    crapgui_t *gui = app->content;
    window_update_user_input(global_window);

    crapgui_update(gui);
    ui_t *button = crapgui_get_button(gui, "FRAME01", "Button01");
    if (button->is_active)
        printf("button SUBMIT clicked\n");

    ui_t *button02 = crapgui_get_button(gui, "FRAME02", "Submit");
    if (button02->is_active)
        printf("Submitted\n");

}

void apprender(application_t *app) 
{
    crapgui_t *gui = app->content;

    crapgui_render(gui);
}

void appdestroy(application_t *app) 
{
    crapgui_t *gui = app->content;

    crapgui_destroy(gui);
}


int main(void)
{
    crapgui_t gui;

    application_t app = {
        .window_width = 800,
        .window_height = 600,
        .window_title = "GUI TEST",
        
        .content = &gui,

        .init = appinit,
        .update = appupdate,
        .render = apprender,
        .destroy = appdestroy,
    };

    application_run(&app);

    return 0;
}
