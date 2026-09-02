#pragma once

#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include "InputManager.h"
#include "CardInteraction.h"
#include <iostream>
#include <filesystem>

class Game
{
public:
    virtual ~Game() = default;

    virtual bool Initialize(HWND window, int width, int height) = 0;
    virtual void Update(float deltaTime) = 0;
    virtual void Render() = 0;
    virtual void OnResize(int width, int height) = 0;
    virtual void Shutdown() = 0;
};

class MyGame final : public Game
{
public:
    void SetInputManager(InputManager* input) noexcept
    {
        m_input = input;
    }

    bool Initialize(HWND window, int width, int height) override
    {
        if (!window || width <= 0 || height <= 0)
        {
            return false;
        }

        m_window = window;
        OnResize(width, height);
        if (std::filesystem::exists(L"Cartas"))
        {
            for (const auto& file : std::filesystem::directory_iterator(L"Cartas"))
            {
                if (file.path().extension() == L".png" && file.path().filename() != L"KcriaGames.png")
                {
                    Card card;
                    card.frontTexture = file.path().wstring();
                    const float x = -220.0f + static_cast<float>(m_cards.size() % 4) * 145.0f;
                    const float y = 120.0f - static_cast<float>(m_cards.size() / 4) * 190.0f;
                    card.position = { x, y, 0.0f };
                    m_cards.push_back(card);
                }
            }
        }
        m_interaction.SetCards(&m_cards);
        m_isInitialized = true;
        return true;
    }

    void Update(float deltaTime) override
    {
        if (!m_isInitialized)
        {
            return;
        }

        // Avoid large simulation jumps after pausing or losing focus.
        if (deltaTime == deltaTime)
        {
            const float frameTime = deltaTime < 0.0f
                ? 0.0f
                : (deltaTime > 0.1f ? 0.1f : deltaTime);
            m_interaction.Update(frameTime, *m_input);
        }

        if (!m_input)
        {
            return;
        }

        if (m_input->IsKeyPressed(KeyCode::Space))
        {
            std::cout << "Space pressed!\n";
        }

        if (m_input->IsMouseButtonPressed(MouseButton::Left))
        {
            std::cout << "Mouse clicked at: " << m_input->GetMouseX()
                      << ", " << m_input->GetMouseY() << '\n';
        }

        if (m_input->IsKeyPressed(KeyCode::Escape))
        {
            PostQuitMessage(0);
        }
    }

    void Render() override
    {
        if (!m_isInitialized || !m_window)
        {
            return;
        }

        HDC hdc = GetDC(m_window);
        if (!hdc)
        {
            return;
        }

        RECT rect{};
        GetClientRect(m_window, &rect);
        HBRUSH brush = CreateSolidBrush(RGB(0, 0, 0));
        HPEN pen = CreatePen(PS_SOLID, 2, RGB(255, 255, 255));

        if (brush && pen)
        {
            FillRect(hdc, &rect, brush);
            HGDIOBJ oldPen = SelectObject(hdc, pen);

            const int centerX = m_width / 2;
            const int centerY = m_height / 2;
            const int radius = 50;
            const int offset = static_cast<int>(m_rotation * 10.0f);
            Ellipse(hdc, centerX - radius + offset, centerY - radius,
                    centerX + radius + offset, centerY + radius);

            SelectObject(hdc, oldPen);
        }

        if (pen)
        {
            DeleteObject(pen);
        }
        if (brush)
        {
            DeleteObject(brush);
        }
        ReleaseDC(m_window, hdc);
    }

    void OnResize(int width, int height) override
    {
        m_width = width > 0 ? width : 0;
        m_height = height > 0 ? height : 0;
        m_interaction.SetViewport(m_width, m_height);
    }

    void Shutdown() override
    {
        m_isInitialized = false;
        m_window = nullptr;
        m_input = nullptr;
    }

private:
    HWND m_window = nullptr;
    InputManager* m_input = nullptr;
    int m_width = 0;
    int m_height = 0;
    float m_rotation = 0.0f;
    std::vector<Card> m_cards;
    CardInteraction m_interaction;
    bool m_isInitialized = false;
};
