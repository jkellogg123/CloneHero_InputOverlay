#include <windows.h>
#include <winuser.h>    // virtual key codes "VK_UP" / "VK_DOWN"
#include <SDL.h>
// #include <SDL_syswm.h>

#include <QApplication>
#include <QWidget>
#include <QPainter>
#include <QMessageBox>
#include <QTimer>
#include <QScreen>
#include <QKeyEvent>
#include <QMouseEvent>

#include <iostream>
#include <unordered_map>
#include <vector>

const int FRET_WIDTH = 100;
const int FRET_HEIGHT = 80;
const int FRET_ROUND = 20;
const int FRET_GAP = 10;
const int STRUM_GAP = 15;
const int INACTIVE_a = 50;

std::vector<QColor> active_colors = {
    {0, 255, 0, 255},  // Green
    {255, 0, 0, 255},  // Red
    {255, 255, 0, 255},// Yellow
    {0, 0, 255, 255},  // Blue
    {255, 165, 0, 255},// Orange
    {204, 0, 204, 255} // Purple (strum)
};
std::vector<QColor> inactive_colors = {
    {0, 255, 0, INACTIVE_a},  // Green
    {255, 0, 0, INACTIVE_a},  // Red
    {255, 255, 0, INACTIVE_a},// Yellow
    {0, 0, 255, INACTIVE_a},  // Blue
    {255, 165, 0, INACTIVE_a},// Orange
    {204, 0, 204, INACTIVE_a} // Purple (strum)
};
SDL_Joystick* guitar = nullptr;
bool render_flag = true;
const int window_w = (6 * FRET_WIDTH) + (5 * FRET_GAP);
const int window_h = FRET_HEIGHT;
enum button {green, red, yellow, blue, orange, sup=5, sdown=8, strum=5};
std::unordered_map<int, button> key_to_button = {
    {'G', green},
    {'H', red},
    {'J', yellow},
    {'K', blue},
    {'L', orange},
    {VK_UP, sup},
    {VK_DOWN, sdown}
};
std::unordered_map<int, unsigned long> press_count;
std::unordered_map<int, bool> pressed;

void print(const char* str) {
    std::cout << str << std::endl;
    return;
}

class GraphicsApp : public QWidget {
public:
    GraphicsApp() {
        setWindowFlags(Qt::FramelessWindowHint);
        setAttribute(Qt::WA_TranslucentBackground);
    }

protected:
    void drawCenteredText(QPainter& painter, const QRect &rect, const QString &text) {
        QFontMetrics fm(painter.font());
        int textWidth = fm.horizontalAdvance(text);

        int x = rect.x() + (rect.width() - textWidth) / 2;
        int y = rect.y() + (rect.height() + fm.ascent() - fm.descent()) / 2;

        painter.setPen(QColor(Qt::white));
        painter.drawText(x, y, text);
    }

    // render function
    void paintEvent(QPaintEvent *event) override {
        Q_UNUSED(event);
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setFont(QFont("Arial", 20));

        int x = 0;

        for (int i = 0; i < 5; ++i) {
            QRect fret(x, 0, FRET_WIDTH, FRET_HEIGHT);
            painter.setBrush(pressed[i] ? active_colors[i] : inactive_colors[i]);
            painter.setPen(Qt::NoPen);
            painter.drawRoundedRect(fret, FRET_ROUND, FRET_ROUND);
            drawCenteredText(painter, fret, QString::number(press_count[i]));
            x += FRET_WIDTH + FRET_GAP;
        }

        int strum_height = (FRET_HEIGHT - STRUM_GAP) / 2;

        QRect top_strum(x, 0, FRET_WIDTH, strum_height);
        painter.setBrush(pressed[sup] ? active_colors[strum] : inactive_colors[strum]);
        painter.setPen(Qt::NoPen);
        painter.drawRoundedRect(top_strum, FRET_ROUND/2, FRET_ROUND/2);
        drawCenteredText(painter, top_strum, QString::number(press_count[sup]));

        QRect bottom_strum(x, strum_height + STRUM_GAP, FRET_WIDTH, strum_height);
        painter.setBrush(pressed[sdown] ? active_colors[strum] : inactive_colors[strum]);
        painter.setPen(Qt::NoPen);
        painter.drawRoundedRect(bottom_strum, FRET_ROUND/2, FRET_ROUND/2);
        drawCenteredText(painter, bottom_strum, QString::number(press_count[sdown]));
    }


    // dragging logic
    void mousePressEvent(QMouseEvent *event) override {
        switch (event->button()) {
            case Qt::LeftButton:
                dragStartPosition = event->globalPosition().toPoint() - frameGeometry().topLeft();
                mousePressed = true;
                break;
            // right click to snap back
            case Qt::RightButton:
                QScreen* screen = QApplication::primaryScreen();
                if (!screen) break;

                QSize screenSize = screen->size();
                int screenWidth = screenSize.width();
                int screenHeight = screenSize.height();
                move(screenWidth - width(), screenHeight - height());
                break;
        }
        QWidget::mousePressEvent(event);
    }
    void mouseMoveEvent(QMouseEvent *event) override {
        if (mousePressed && (event->buttons() & Qt::LeftButton)) {
            move(event->globalPosition().toPoint() - dragStartPosition);
        }
        QWidget::mouseMoveEvent(event);
    }
    void mouseReleaseEvent(QMouseEvent *event) override {
        if (event->button() == Qt::LeftButton) {
            mousePressed = false;
        }
        QWidget::mouseReleaseEvent(event);
    }

    // press esc to close
    void keyPressEvent(QKeyEvent* event) override {
        if (event->key() == Qt::Key_Escape) {
            QApplication::quit();  // or this->close() to just close the window
        }
    }

private:
    bool mousePressed = false;
    QPoint dragStartPosition;
};
GraphicsApp* window = nullptr;

void clean() {
    delete window;
    SDL_JoystickClose(guitar);
    SDL_Quit();
}

SDL_Joystick* getGuitar() {
    if (SDL_NumJoysticks() != 1) {
        return nullptr;
    }
    return SDL_JoystickOpen(0);
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
        case SDL_JOYDEVICEREMOVED:
            SDL_JoystickClose(guitar);
            guitar = nullptr;
            break;
        case SDL_JOYDEVICEADDED:
            if (!guitar) {
                guitar = getGuitar();
            }
            break;
    }
}

int initSDL() {
    if (SDL_Init(SDL_INIT_JOYSTICK) != 0) {
        MessageBoxA(NULL, SDL_GetError(), "SDL Error", MB_OK);
        return -1;
    }

    SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");
    guitar = getGuitar();

    // darken colors a bit
    float darken_factor = 0.8;
    for (QColor& color : active_colors) {
        color.setRed(static_cast<int>(color.red() * darken_factor));
        color.setGreen(static_cast<int>(color.green() * darken_factor));
        color.setBlue(static_cast<int>(color.blue() * darken_factor));
    }
    // for (QColor& color : inactive_colors) {
    //     color.setRed(static_cast<int>(color.red() * darken_factor));
    //     color.setGreen(static_cast<int>(color.green() * darken_factor));
    //     color.setBlue(static_cast<int>(color.blue() * darken_factor));
    // }

    return 0;
}

void checkSDL() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        eventHandler(event);
    }
    if (render_flag) window->update();
}

int initQt() {
    window = new GraphicsApp();
    if (!window) {
        QMessageBox::critical(nullptr, "Error", "Couldn't create Qt window");
        return -1;
    }
    window->resize(window_w, window_h);
    window->setWindowFlags(window->windowFlags() | Qt::WindowStaysOnTopHint);

    QScreen* screen = QApplication::primaryScreen();
    if (screen) {
        QSize screenSize = screen->size();
        int screenWidth = screenSize.width();
        int screenHeight = screenSize.height();
        window->move(screenWidth - window->width(), screenHeight - window->height());
    }
    else {
        window->move(0, 0);
    }

    window->show();
    // window->setAttribute(Qt::WA_NoSystemBackground, true);
    // window->setAutoFillBackground(false);
    QTimer* timer = new QTimer(window);
    QObject::connect(timer, &QTimer::timeout, &checkSDL);
    timer->start(10);

    return 0;
}

// dumb, needed to capture keyboard inputs out of focus
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    // Check if the event is a key press
    if (nCode == HC_ACTION) {
        KBDLLHOOKSTRUCT *kbd = (KBDLLHOOKSTRUCT*)lParam;
        switch (wParam) {
            case WM_KEYDOWN:
            case WM_SYSKEYDOWN:
            case WM_KEYUP:
            case WM_SYSKEYUP:
                DWORD key = kbd->vkCode;
                if (key_to_button.find(key) != key_to_button.end()) {
                    button butt = key_to_button[key];
                    if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
                        if (!pressed[butt]) {
                            press_count[butt] += 1;
                            pressed[butt] = true;
                            render_flag = true;
                        }
                    }
                    // up/release
                    else {
                        pressed[butt] = false;
                        render_flag = true;
                    }
                }
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    int argc = 0;
    char* argv[] = {nullptr};
    QApplication app(argc, argv);
    if (initSDL() == -1 || initQt() == -1) {
        clean();
        return 1;
    }

    // see comment above this function
    HHOOK keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, NULL, 0);

    int result = app.exec();

    clean();
    UnhookWindowsHookEx(keyboardHook);
    return result;
}
