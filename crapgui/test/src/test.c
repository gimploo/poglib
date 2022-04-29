#include <poglib/crapgui.h>


void appinit(application_t *app) 
{
    crapgui_t *gui = app->content;
    *gui = crapgui_init();

    crapgui_layout(gui) {

        frame("FRAME01") {
            /*label("Test");*/
            button("Yes"); button("No");
            button("Yes"); button("No");
            /*label("Confirm"); button("Submit");*/
            label("THIS IS A LABEL");
        }

        frame("FRAME02") {
            button("Yes"); button("No");
            button("Yes"); button("No");
            label("THIS IS A LABEL");
        }
        frame("FRAME03") {
            button("Yes"); button("No");
            button("Yes"); button("No");
            label("THIS IS A LABEL");
        }

        frame("FRAME04") {
            button("Yes"); button("No");
            button("Yes"); button("No");
            label("THIS IS A LABEL");
        }

    };
}

void appupdate(application_t *app) 
{
    crapgui_t *gui = app->content;
    window_update_user_input(global_window);

    crapgui_update(gui);
    button_t *button = crapgui_get_button(gui, "FRAME01", "Yes");
    if (button->is_active)
        printf("button SUBMIT clicked\n");

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
