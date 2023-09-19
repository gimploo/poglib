#include <poglib/crapgui.h>

void appinit(application_t *app) 
{

    crapgui_t gui = crapgui_init();

    crapgui_layout(&gui) {

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

    application_pass_content(app, &gui);
}

void appupdate(application_t *app) 
{
    crapgui_t *gui = application_get_content(app);
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
    crapgui_t *gui = application_get_content(app);

    crapgui_render(gui);
}

void appdestroy(application_t *app) 
{
    crapgui_t *gui = application_get_content(app);

    crapgui_destroy(gui);
}


int main(void)
{
    application_t app = {
        .window = {
            .title = "GUI TEST",
            .width = 800,
            .height = 900,
            .aspect_ratio = (f32)800 / (f32)900,
            .fps_limit = 60,
            .background_color = COLOR_RED
        },   
        .content = {
            .size = sizeof(crapgui_t )
        },
        
        .init = appinit,
        .update = appupdate,
        .render = apprender,
        .destroy = appdestroy,
    };

    application_run(&app);

    return 0;
}
