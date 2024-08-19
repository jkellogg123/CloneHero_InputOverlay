#include <windows.h>
#include <SDL.h>
#include <SDL_syswm.h>
#include <SDL_ttf.h>
#include <iostream>
#include <unordered_map>
#include <vector>

const int FRET_WIDTH = 100;
const int FRET_HEIGHT = 80;
const int FRET_GAP = 10;
const int STRUM_GAP = 20;
const int INACTIVE_a = 50;

std::vector<SDL_Color> active_colors = {
    {0, 255, 0, 255},  // Green
    {255, 0, 0, 255},  // Red
    {255, 255, 0, 255},// Yellow
    {0, 0, 255, 255},  // Blue
    {255, 165, 0, 255},// Orange
    {128, 0, 128, 255} // Purple (strum)
};
std::vector<SDL_Color> inactive_colors;

SDL_Window* window = nullptr;
HWND hwnd;
SDL_Renderer* renderer = nullptr;
SDL_Joystick* guitar = nullptr;
TTF_Font* font = nullptr;
bool running = true;
bool render_flag = true;
bool dragging = false;
int mouse_x = 0, mouse_y = 0;
const int window_w = (6 * FRET_WIDTH) + (5 * FRET_GAP);
const int window_h = FRET_HEIGHT;
int window_x = SDL_WINDOWPOS_CENTERED;
int window_y = 0;

enum button {green, red, yellow, blue, orange, sup=5, sdown=8, strum=5};
std::unordered_map<int, unsigned long> press_count;
std::unordered_map<int, bool> pressed;


void renderTextCentered(const std::string& text, SDL_Rect rect) {
    SDL_Color color = {0, 0, 0, 255};   // black

    SDL_Surface* textSurface = TTF_RenderText_Solid(font, text.c_str(), color);
    if (!textSurface) {
        std::cerr << "Unable to render text to surface! SDL_ttf Error: " << TTF_GetError() << std::endl;
        return;
    }

    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    if (!textTexture) {
        std::cerr << "Unable to create texture from rendered text! SDL Error: " << SDL_GetError() << std::endl;
        SDL_FreeSurface(textSurface);
        return;
    }

    int textWidth = textSurface->w;
    int textHeight = textSurface->h;

    SDL_FreeSurface(textSurface);

    SDL_Rect textRect;
    textRect.w = textWidth;
    textRect.h = textHeight;
    textRect.x = rect.x + (rect.w - textWidth) / 2;
    textRect.y = rect.y + (rect.h - textHeight) / 2;

    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
    SDL_DestroyTexture(textTexture);
}

void render() {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0);
    SDL_RenderClear(renderer);
    int x = 0;

    // frets
    for (int i = 0; i < 5; ++i) {
        SDL_Rect rect = { x, 0, FRET_WIDTH, FRET_HEIGHT };
        SDL_Color color = pressed[i] ? active_colors[i] : inactive_colors[i];
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        SDL_RenderFillRect(renderer, &rect);
        if (font) renderTextCentered(std::to_string(press_count[i]), rect);
        x += FRET_WIDTH + FRET_GAP;
    }

    // up strum
    SDL_Rect topRect = { x, 0, FRET_WIDTH, (FRET_HEIGHT - STRUM_GAP) / 2 };
    SDL_SetRenderDrawColor(renderer, active_colors[strum].r, active_colors[strum].g, active_colors[strum].b, pressed[sup] ? active_colors[strum].a : INACTIVE_a);
    SDL_RenderFillRect(renderer, &topRect);
    if (font) renderTextCentered(std::to_string(press_count[sup]), topRect);

    // down strum
    SDL_Rect bottomRect = { x, (FRET_HEIGHT - STRUM_GAP) / 2 + STRUM_GAP, FRET_WIDTH, (FRET_HEIGHT - STRUM_GAP) / 2 };
    SDL_SetRenderDrawColor(renderer, active_colors[strum].r, active_colors[strum].g, active_colors[strum].b, pressed[sdown] ? active_colors[strum].a : INACTIVE_a);
    SDL_RenderFillRect(renderer, &bottomRect);
    if (font) renderTextCentered(std::to_string(press_count[sdown]), bottomRect);

    SDL_RenderPresent(renderer);
    render_flag = false;
}

void fixWindowTop() {
    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    if (SDL_GetWindowWMInfo(window, &wmInfo)) {
        hwnd = wmInfo.info.win.window;
        SetWindowPos(hwnd, HWND_TOPMOST, window_x, window_y, window_w, window_h, SWP_NOSIZE | SWP_NOMOVE);
    } else {
        std::cerr << "Failed to get window manager info: " << SDL_GetError() << std::endl;
    }
}

void eventHandler(const SDL_Event& event) {
    switch (event.type) {
        case SDL_JOYBUTTONDOWN:
            render_flag = true;
            press_count[event.jbutton.button] += 1;
            pressed[event.jbutton.button] = true;
            break;
        case SDL_JOYBUTTONUP:
            render_flag = true;
            pressed[event.jbutton.button] = false;
            break;

        case SDL_MOUSEBUTTONDOWN:
            if (event.button.button == SDL_BUTTON_LEFT) {
                dragging = true;
                SDL_GetGlobalMouseState(&mouse_x, &mouse_y);
                SDL_GetWindowPosition(window, &window_x, &window_y);
            }
            break;
        case SDL_MOUSEBUTTONUP:
            if (event.button.button == SDL_BUTTON_LEFT) {
                dragging = false;
                SDL_GetWindowPosition(window, &window_x, &window_y);
            }
            break;
        case SDL_MOUSEMOTION:
            if (dragging) {
                int newmouse_x, newmouse_y;
                SDL_GetGlobalMouseState(&newmouse_x, &newmouse_y);
                int dx = newmouse_x - mouse_x;
                int dy = newmouse_y - mouse_y;
                SDL_SetWindowPosition(window, window_x + dx, window_y + dy);
            }
            break;
        case SDL_WINDOWEVENT:
            if (event.window.event == SDL_WINDOWEVENT_RESTORED) render_flag = true;
            else if (event.window.event == SDL_WINDOWEVENT_MINIMIZED) ShowWindow(hwnd, SW_RESTORE);
            break;
        case SDL_QUIT:
            running = false;
            break;
        case SDL_KEYDOWN:
            if (event.key.keysym.sym == SDLK_ESCAPE) {
                running = false;
            }
            break;
    }

}

void clean() {
    TTF_CloseFont(font);
    SDL_JoystickClose(guitar);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
}

SDL_Joystick* getGuitar() {
    if (SDL_NumJoysticks() != 1) {
        MessageBoxA(NULL, "No joystick connected or multiple??", "lol bozo", MB_OK);
        return nullptr;
    }
    return SDL_JoystickOpen(0);
}

void fillWindowXY() {
    SDL_DisplayMode displayMode;
    if (SDL_GetCurrentDisplayMode(0, &displayMode) != 0) {
        std::cerr << "Could not get display mode! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return;
    }

    window_x = displayMode.w - window_w;
    window_y = displayMode.h - window_h;
}

SDL_Color blendColor(const SDL_Color& src, const SDL_Color& dst) {
    SDL_Color result;
    float alpha = INACTIVE_a / 255.0;
    
    result.r = static_cast<uint8_t>((alpha * src.r) + ((1 - alpha) * dst.r));
    result.g = static_cast<uint8_t>((alpha * src.g) + ((1 - alpha) * dst.g));
    result.b = static_cast<uint8_t>((alpha * src.b) + ((1 - alpha) * dst.b));
    result.a = 255;

    return result;
}

void createInactiveColors() {
    inactive_colors.reserve(active_colors.size());
    for (const SDL_Color& color : active_colors) {
        SDL_Color blended = blendColor(color, {255, 255, 255, 0});
        inactive_colors.push_back(blended);
    }
    // for (const SDL_Color& color : inactive_colors) {
    //     std::cout << "(" << std::to_string(color.r) << ", " << std::to_string(color.g) << ", " << std::to_string(color.b) << ", " << std::to_string(color.a) << ")" << std::endl;
    // }
}

void setWindowTransparent() {
    SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
    SetLayeredWindowAttributes(hwnd, RGB(255, 255, 255), 0, LWA_COLORKEY);
    
    // i wanted to set multiple colors as transparent but this function only allows 1 at any time
    // for (const SDL_Color& color : inactive_colors) {
    //     SetLayeredWindowAttributes(hwnd, RGB(color.r, color.g, color.b), 100, LWA_COLORKEY | LWA_ALPHA);
    // }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) != 0) {
        MessageBoxA(NULL, SDL_GetError(), "SDL Error", MB_OK);
        return 1;
    }
    if (TTF_Init() == -1) {
        MessageBoxA(NULL, TTF_GetError(), "SDL_ttf Error", MB_OK);
        clean();
        return -1;
    }

    guitar = getGuitar();
    if (!guitar) {
        clean();
        return 1;
    }
    SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");
    fillWindowXY();
    window = SDL_CreateWindow("", window_x, window_y, window_w, window_h, SDL_WINDOW_ALWAYS_ON_TOP | SDL_WINDOW_BORDERLESS | SDL_WINDOW_SHOWN | SDL_WINDOW_SKIP_TASKBAR);
    if (window == nullptr) {
        MessageBoxA(NULL, SDL_GetError(), "SDL Error", MB_OK);
        clean();
        return 1;
    }
    fixWindowTop();
    createInactiveColors();
    setWindowTransparent();
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == nullptr) {
        MessageBoxA(NULL, SDL_GetError(), "SDL Error", MB_OK);
        clean();
        return 1;
    }
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    font = TTF_OpenFont("./swansea-font/Swansea-q3pd.ttf", 24);
    if (!font) {
        std::cerr << "Couldn't load font" << std::endl;
    }

    // Main loop
    SDL_Event event;
    while (running) {
        while (SDL_PollEvent(&event)) {
            eventHandler(event);
        }
        if (render_flag) render();
        // SDL_Delay(10);
    }

    clean();
    return 0;
}
