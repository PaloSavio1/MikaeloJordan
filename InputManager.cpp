#include "InputManager.h"

InputManager::InputManager()
    : m_mouseX(0)
    , m_mouseY(0)
{
    // Inicializar estados
    for (int i = 0; i < 256; ++i)
    {
        m_keyState[static_cast<KeyCode>(i)] = false;
        m_prevKeyState[static_cast<KeyCode>(i)] = false;
    }
    
    m_mouseState[MouseButton::Left] = false;
    m_mouseState[MouseButton::Right] = false;
    m_mouseState[MouseButton::Middle] = false;
    m_doubleClick[MouseButton::Left] = false;
    m_doubleClick[MouseButton::Right] = false;
    m_doubleClick[MouseButton::Middle] = false;
    
    m_prevMouseState = m_mouseState;
}

void InputManager::OnMouseDoubleClick(int x, int y, MouseButton button)
{
    m_mouseX = x;
    m_mouseY = y;
    m_doubleClick[button] = true;
}

void InputManager::OnMouseWheel(int x, int y, int delta)
{
    m_mouseX = x;
    m_mouseY = y;
    m_wheelDelta += delta;
}

InputManager::~InputManager()
{
}

void InputManager::Update()
{
    m_prevKeyState = m_keyState;
    m_prevMouseState = m_mouseState;
}

void InputManager::OnKeyDown(KeyCode key)
{
    m_keyState[key] = true;
}

void InputManager::OnKeyUp(KeyCode key)
{
    m_keyState[key] = false;
}

bool InputManager::IsKeyPressed(KeyCode key) const
{
    auto it = m_keyState.find(key);
    if (it != m_keyState.end())
    {
        return it->second && !m_prevKeyState.at(key);
    }
    return false;
}

bool InputManager::IsKeyDown(KeyCode key) const
{
    auto it = m_keyState.find(key);
    if (it != m_keyState.end())
    {
        return it->second;
    }
    return false;
}

void InputManager::OnMouseMove(int x, int y)
{
    m_mouseX = x;
    m_mouseY = y;
}

void InputManager::OnMouseButtonDown(int x, int y, MouseButton button)
{
    m_mouseX = x;
    m_mouseY = y;
    m_mouseState[button] = true;
}

void InputManager::OnMouseButtonUp(int x, int y, MouseButton button)
{
    m_mouseX = x;
    m_mouseY = y;
    m_mouseState[button] = false;
}

bool InputManager::IsMouseButtonPressed(MouseButton button) const
{
    auto it = m_mouseState.find(button);
    if (it != m_mouseState.end())
    {
        return it->second && !m_prevMouseState.at(button);
    }
    return false;
}

bool InputManager::ConsumeDoubleClick(MouseButton button)
{
    const bool result = m_doubleClick[button];
    m_doubleClick[button] = false;
    return result;
}

int InputManager::ConsumeWheelDelta()
{
    const int result = m_wheelDelta;
    m_wheelDelta = 0;
    return result;
}

bool InputManager::IsMouseButtonDown(MouseButton button) const
{
    auto it = m_mouseState.find(button);
    if (it != m_mouseState.end())
    {
        return it->second;
    }
    return false;
}