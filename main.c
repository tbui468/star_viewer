#include "SDL3/SDL.h"
#include "client.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_SDL_RENDERER_IMPLEMENTATION
#include "nuklear.h"
#include "nuklear_sdl_renderer.h"

#define WIN_WIDTH 1024
#define WIN_HEIGHT 768


int main(int argc, char** argv) {
    srand(time(NULL));
    argc = argc;
    argv = argv;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
        exit(1);
    }

    SDL_Window* window = SDL_CreateWindow("Star Viewer", WIN_WIDTH, WIN_HEIGHT, 0);
    //SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL, SDL_RENDERER_ACCELERATED);

    float font_scale = 1.0f;
    {
        int render_w, render_h;
        int window_w, window_h;
        float scale_x, scale_y;
        SDL_GetCurrentRenderOutputSize(renderer, &render_w, &render_h);
        SDL_GetWindowSize(window, &window_w, &window_h);
        scale_x = (float)(render_w) / (float)(window_w);
        scale_y = (float)(render_h) / (float)(window_h);
        SDL_SetRenderScale(renderer, scale_x, scale_y);
        font_scale = scale_y;
    }

    /* GUI */
    struct nk_context* ctx = nk_sdl_init(window, renderer);
    /* Load Fonts: if none of these are loaded a default font will be used  */
    /* Load Cursor: if you uncomment cursor loading please hide the cursor */
    {
        struct nk_font_atlas *atlas;
        struct nk_font_config config = nk_font_config(0);
        struct nk_font *font;

        /* set up the font atlas and add desired font; note that font sizes are
         * multiplied by font_scale to produce better results at higher DPIs */
        nk_sdl_font_stash_begin(&atlas);
        //font = nk_font_atlas_add_default(atlas, 13 * font_scale, &config);
        /*font = nk_font_atlas_add_from_file(atlas, "../../../extra_font/DroidSans.ttf", 14 * font_scale, &config);*/
        /*font = nk_font_atlas_add_from_file(atlas, "../../../extra_font/Roboto-Regular.ttf", 16 * font_scale, &config);*/
        font = nk_font_atlas_add_from_file(atlas, "../../kenvector_future_thin.ttf", 13 * font_scale, &config);
        /*font = nk_font_atlas_add_from_file(atlas, "../../../extra_font/ProggyClean.ttf", 12 * font_scale, &config);*/
        /*font = nk_font_atlas_add_from_file(atlas, "../../../extra_font/ProggyTiny.ttf", 10 * font_scale, &config);*/
        /*font = nk_font_atlas_add_from_file(atlas, "../../../extra_font/Cousine-Regular.ttf", 13 * font_scale, &config);*/
        nk_sdl_font_stash_end();

        /* this hack makes the font appear to be scaled down to the desired
         * size and is only necessary when font_scale > 1 */
        font->handle.height /= font_scale;
        /*nk_style_load_all_cursors(ctx, atlas->cursors);*/
        nk_style_set_font(ctx, &font->handle);
    }

    VDBHANDLE h = vdbclient_connect("127.0.0.1", "3333");
    char* buf;
    if (!(buf = vdbclient_execute_query(h, "open universe;"))) {
        printf("failed\n");
    } else {
        printf("%s\n", buf);
        free(buf);
    }


    SDL_FPoint* points;
    int star_count;
    //if (!(buf = vdbclient_execute_query(h, "select x, y from stars where x > -10.0 and x < 10.0 and y > -10.0 and y < 10.0;"))) {
    if (!(buf = vdbclient_execute_query(h, "select x, y from stars where x < 100.0;"))) {
        printf("failed\n");
    } else {
        printf("%s\n", buf);
        star_count = strlen(buf) / 20; //20 characters per line (8 each each coord and comma + space/newline)
        points = malloc(sizeof(SDL_FPoint) * star_count);
        for (int i = 0; i < star_count; i++) {
            //TODO: this won't work since negatives will mess everything up
            int off = i * 20;
            buf[off + 8] = '\0';
            buf[off + 18] = '\0';
            char* c;
            points[i].x = strtof(&buf[off], &c) * 10.0f;
            points[i].y = strtof(&buf[off + 10], &c) * 10.0f;
        }
        free(buf);
    }
    /*
    int star_count = 10000;
    SDL_FPoint points[10000];
    for (int i = 0; i < star_count; i++) {
        while (1) {
            points[i].x = (((float)rand()) / RAND_MAX - 0.5f) * WIN_WIDTH;
            points[i].y = (((float)rand()) / RAND_MAX - 0.5f) * WIN_WIDTH;
            if (sqrtf(points[i].x * points[i].x + points[i].y * points[i].y) < WIN_WIDTH / 2) {
                points[i].x += WIN_WIDTH / 2;
                points[i].y += WIN_HEIGHT / 2;
                break;
            }
        }
    }*/

    struct nk_colorf bg;
    bg.r = 0.10f, bg.g = 0.18f, bg.b = 0.24f, bg.a = 1.0f;
    bool run = true;
    SDL_Event e;
    const uint8_t* keys = SDL_GetKeyboardState(NULL);

    float anchor_x;
    float anchor_y;
    SDL_GetGlobalMouseState(&anchor_x, &anchor_y);
    bool drag = false;
    bool draw = false;

    SDL_FRect r;
    r.x = 0.0f;
    r.y = 0.0f;
    r.w = 0.0f;
    r.h = 0.0f;

    while (run) {
        nk_input_begin(ctx);
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
                case SDL_EVENT_QUIT:
                    run = false;
                    break;
                case SDL_EVENT_MOUSE_BUTTON_DOWN:
                    SDL_GetMouseState(&anchor_x, &anchor_y);
                    if (e.button.button == SDL_BUTTON_RIGHT) {
                        drag = true;
                    }
                    if (e.button.button == SDL_BUTTON_LEFT) {
                        r.x = anchor_x;
                        r.y = anchor_y;
                        draw = true;
                    }
                    break;
                case SDL_EVENT_MOUSE_BUTTON_UP:
                    if (e.button.button == SDL_BUTTON_RIGHT) {
                        drag = false;
                    }
                    if (e.button.button == SDL_BUTTON_LEFT) {
                        draw = false;
                        if (!(buf = vdbclient_execute_query(h, "select prop, x, y from stars where x > 100.0 and x < 300.0;"))) {
                            printf("failed\n");
                        } else {
                            printf("%s\n", buf);
                            free(buf);
                        }
                    }
                    break;
            }
            nk_sdl_handle_event(&e);
        }
        nk_input_end(ctx);

        if (drag) {
            float x;
            float y;
            SDL_GetMouseState(&x, &y);
            for (int i = 0; i < star_count; i++) {
                points[i].x += x - anchor_x;
                points[i].y += y - anchor_y;
            }
            anchor_x = x;
            anchor_y = y;

        }

        if (draw) {
            float x;
            float y;
            SDL_GetMouseState(&x, &y);
            r.w = x - r.x;
            r.h = y - r.y;
        }

        if (keys[SDL_SCANCODE_A])
            for (int i = 0; i < star_count; i++) {
                points[i].x += 3.0f;
            }
        if (keys[SDL_SCANCODE_D])
            for (int i = 0; i < star_count; i++) {
                points[i].x -= 3.0f;
            }
        if (keys[SDL_SCANCODE_W])
            for (int i = 0; i < star_count; i++) {
                points[i].y += 3.0f;
            }
        if (keys[SDL_SCANCODE_S])
            for (int i = 0; i < star_count; i++) {
                points[i].y -= 3.0f;
            }

        /* GUI */
        if (nk_begin(ctx, "Demo", nk_rect(0, WIN_HEIGHT - 100, WIN_WIDTH, 100),
            NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
            NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE))
        {
            enum {EASY, HARD};
            static int op = EASY;
            static int property = 20;

            nk_layout_row_static(ctx, 30, 80, 1);
            if (nk_button_label(ctx, "button"))
                fprintf(stdout, "button pressed\n");
            nk_layout_row_dynamic(ctx, 30, 2);
            if (nk_option_label(ctx, "easy", op == EASY)) op = EASY;
            if (nk_option_label(ctx, "hard", op == HARD)) op = HARD;
            nk_layout_row_dynamic(ctx, 25, 1);
            nk_property_int(ctx, "Compression:", 0, &property, 100, 10, 1);

            nk_layout_row_dynamic(ctx, 20, 1);
            nk_label(ctx, "background:", NK_TEXT_LEFT);
            nk_layout_row_dynamic(ctx, 25, 1);
            if (nk_combo_begin_color(ctx, nk_rgb_cf(bg), nk_vec2(nk_widget_width(ctx),400))) {
                nk_layout_row_dynamic(ctx, 120, 1);
                bg = nk_color_picker(ctx, bg, NK_RGBA);
                nk_layout_row_dynamic(ctx, 25, 1);
                bg.r = nk_propertyf(ctx, "#R:", 0, bg.r, 1.0f, 0.01f,0.005f);
                bg.g = nk_propertyf(ctx, "#G:", 0, bg.g, 1.0f, 0.01f,0.005f);
                bg.b = nk_propertyf(ctx, "#B:", 0, bg.b, 1.0f, 0.01f,0.005f);
                bg.a = nk_propertyf(ctx, "#A:", 0, bg.a, 1.0f, 0.01f,0.005f);
                nk_combo_end(ctx);
            }
        }
        nk_end(ctx);


        SDL_SetRenderDrawColor(renderer, 10, 10, 20, 255);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderPoints(renderer, points, star_count);

        SDL_RenderRect(renderer, &r);

        nk_sdl_render(NK_ANTI_ALIASING_ON);
        SDL_RenderPresent(renderer);
    }

    //TODO: should free buf after executing each query here
    buf = vdbclient_execute_query(h, "close universe;");
    buf = vdbclient_execute_query(h, "exit;");
    vdbclient_disconnect(h);

    nk_sdl_shutdown();
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
