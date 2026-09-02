#pragma once
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <memory>
#include <string>
#include "Game.h"
#include "InputManager.h"

class Win32Application
{
public:
    Win32Application();
    ~Win32Application();

    bool Initialize(HINSTANCE instance, const std::wstring& title, int width, int height);
    int Run();

    HWND GetWindow() const { return m_window; }
    
    // Métodos estáticos para acceso global
    static Win32Application* GetInstance();
    static InputManager* GetInputManager();

private:
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK StaticWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    
    LRESULT HandleMessage(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    void ProcessInput();

    HWND m_window;
    HINSTANCE m_instance;
    std::unique_ptr<Game> m_game;
    std::unique_ptr<InputManager> m_inputManager;
    
    bool m_isRunning;
    int m_width;
    int m_height;
    
    static Win32Application* s_instance;
};