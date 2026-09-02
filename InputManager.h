#pragma once
#include <unordered_map>
#include <functional>
#include <cstdint>

enum class KeyCode
{
    W = 0x57,
    A = 0x41,
    S = 0x53,
    D = 0x44,
    Space = 0x20,
    Escape = 0x1B,
    // ... más teclas
};

enum class MouseButton
{
    Left,
    Right,
    Middle
};

class InputManager
{
public:
    InputManager();
    ~InputManager();

    void Update();

    // Eventos de teclado
    void OnKeyDown(KeyCode key);
    void OnKeyUp(KeyCode key);
    bool IsKeyPressed(KeyCode key) const;
    bool IsKeyDown(KeyCode key) const;

    // Eventos de ratón
    void OnMouseMove(int x, int y);
    void OnMouseButtonDown(int x, int y, MouseButton button);
    void OnMouseButtonUp(int x, int y, MouseButton button);
    void OnMouseDoubleClick(int x, int y, MouseButton button);
    void OnMouseWheel(int x, int y, int delta);
    bool IsMouseButtonPressed(MouseButton button) const;
    bool IsMouseButtonDown(MouseButton button) const;
    bool ConsumeDoubleClick(MouseButton button);
    int ConsumeWheelDelta();

    // Obtener posición del ratón
    int GetMouseX() const { return m_mouseX; }
    int GetMouseY() const { return m_mouseY; }

private:
    std::unordered_map<KeyCode, bool> m_keyState;
    std::unordered_map<KeyCode, bool> m_prevKeyState;
    
    std::unordered_map<MouseButton, bool> m_mouseState;
    std::unordered_map<MouseButton, bool> m_prevMouseState;
    
    int m_mouseX;
    int m_mouseY;
    std::unordered_map<MouseButton, bool> m_doubleClick;
    int m_wheelDelta = 0;
};