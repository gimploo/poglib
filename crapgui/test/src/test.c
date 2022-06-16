#include <poglib/crapgui.h>

void appinit(application_t *app) 
{
    crapgui_t *gui = app->content;
    *gui = crapgui_init();


    crapgui_layout(gui) {

        frame("FRAME01", DEFAULT_FRAME_STYLE ) {
            /*label("Test");*/
            button("Button01", DEFAULT_BUTTON_STYLE); 
            button("Button02", DEFAULT_BUTTON_STYLE);

            button("Button03", DEFAULT_BUTTON_STYLE); 
            button("Button04", DEFAULT_BUTTON_STYLE);
            label("Confirm", DEFAULT_LABEL_STYLE); 
            button("Submit", DEFAULT_BUTTON_STYLE);
            label("THIS IS A LABEL", DEFAULT_LABEL_STYLE);
        }

        frame("FRAME02", DEFAULT_FRAME_STYLE) {
            label("Confirm", DEFAULT_LABEL_STYLE); 
            button("Submit", DEFAULT_BUTTON_STYLE);
            label("THIS IS A LABEL", DEFAULT_LABEL_STYLE);
        }
        frame("FRAME03", DEFAULT_FRAME_STYLE) {
            label("THIS IS A LABEL", DEFAULT_LABEL_STYLE);
        }

        frame("FRAME04", DEFAULT_FRAME_STYLE) {
            button("Button03", DEFAULT_BUTTON_STYLE); 
            button("Button04", DEFAULT_BUTTON_STYLE);
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
