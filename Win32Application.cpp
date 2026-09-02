#include "Win32Application.h"
#include <chrono>
#include <windowsx.h>

Win32Application* Win32Application::s_instance = nullptr;

Win32Application::Win32Application()
    : m_window(nullptr), m_instance(nullptr), m_isRunning(false), m_width(0), m_height(0)
{
}

Win32Application::~Win32Application()
{
    if (m_game)
    {
        m_game->Shutdown();
    }
    s_instance = nullptr;
}

bool Win32Application::Initialize(HINSTANCE instance, const std::wstring& title, int width, int height)
{
    if (!instance || width <= 0 || height <= 0)
    {
        return false;
    }

    m_instance = instance;
    m_width = width;
    m_height = height;
    s_instance = this;

    constexpr wchar_t className[] = L"ArenaTcgWindowClass";
    WNDCLASSEXW windowClass{sizeof(WNDCLASSEXW)};
    windowClass.hInstance = m_instance;
    windowClass.lpfnWndProc = StaticWindowProc;
    windowClass.lpszClassName = className;
    windowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    if (!RegisterClassExW(&windowClass) && GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
    {
        return false;
    }

    m_window = CreateWindowExW(0, className, title.c_str(), WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, width, height, nullptr, nullptr, m_instance, this);
    if (!m_window)
    {
        return false;
    }

    m_inputManager = std::make_unique<InputManager>();
    m_game = std::make_unique<MyGame>();
    if (!m_game->Initialize(m_window, width, height))
    {
        return false;
    }
    static_cast<MyGame*>(m_game.get())->SetInputManager(m_inputManager.get());
    ShowWindow(m_window, SW_SHOWDEFAULT);
    UpdateWindow(m_window);
    return true;
}

int Win32Application::Run()
{
    m_isRunning = true;
    MSG message{};
    auto previous = std::chrono::steady_clock::now();
    while (m_isRunning)
    {
        if (m_inputManager) m_inputManager->Update();
        while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE))
        {
            if (message.message == WM_QUIT)
            {
                m_isRunning = false;
                break;
            }
            TranslateMessage(&message);
            DispatchMessageW(&message);
        }
        if (!m_isRunning) break;
        const auto now = std::chrono::steady_clock::now();
        const float deltaTime = std::chrono::duration<float>(now - previous).count();
        previous = now;
        m_game->Update(deltaTime);
        m_game->Render();
    }
    return static_cast<int>(message.wParam);
}

Win32Application* Win32Application::GetInstance() { return s_instance; }

InputManager* Win32Application::GetInputManager()
{
    return s_instance ? s_instance->m_inputManager.get() : nullptr;
}

void Win32Application::ProcessInput()
{
    if (m_inputManager) m_inputManager->Update();
}

LRESULT CALLBACK Win32Application::StaticWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    auto* application = reinterpret_cast<Win32Application*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    if (message == WM_NCCREATE)
    {
        auto* create = reinterpret_cast<CREATESTRUCTW*>(lParam);
        application = static_cast<Win32Application*>(create->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(application));
    }
    return application ? application->HandleMessage(hwnd, message, wParam, lParam)
                       : DefWindowProcW(hwnd, message, wParam, lParam);
}

LRESULT CALLBACK Win32Application::WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    return s_instance ? s_instance->HandleMessage(hwnd, message, wParam, lParam)
                      : DefWindowProcW(hwnd, message, wParam, lParam);
}

LRESULT Win32Application::HandleMessage(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CLOSE: DestroyWindow(hwnd); return 0;
    case WM_DESTROY: PostQuitMessage(0); return 0;
    case WM_SIZE:
        m_width = static_cast<int>(LOWORD(lParam));
        m_height = static_cast<int>(HIWORD(lParam));
        if (m_game) m_game->OnResize(m_width, m_height);
        return 0;
    case WM_MOUSEMOVE:
        if (m_inputManager) m_inputManager->OnMouseMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        return 0;
    case WM_LBUTTONDOWN:
        if (m_inputManager) m_inputManager->OnMouseButtonDown(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), MouseButton::Left);
        return 0;
    case WM_LBUTTONUP:
        if (m_inputManager) m_inputManager->OnMouseButtonUp(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), MouseButton::Left);
        return 0;
    case WM_LBUTTONDBLCLK:
        if (m_inputManager) m_inputManager->OnMouseDoubleClick(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), MouseButton::Left);
        return 0;
    case WM_MOUSEWHEEL:
        if (m_inputManager)
        {
            POINT point{ GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
            ScreenToClient(hwnd, &point);
            m_inputManager->OnMouseWheel(point.x, point.y, GET_WHEEL_DELTA_WPARAM(wParam));
        }
        return 0;
    case WM_KEYDOWN:
        if (!(lParam & (static_cast<LPARAM>(1) << 30)) && m_inputManager)
            m_inputManager->OnKeyDown(static_cast<KeyCode>(wParam));
        return 0;
    case WM_KEYUP:
        if (m_inputManager) m_inputManager->OnKeyUp(static_cast<KeyCode>(wParam));
        return 0;
    default: return DefWindowProcW(hwnd, message, wParam, lParam);
    }
}
