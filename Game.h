#pragma once

#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <objidl.h>
#include <gdiplus.h>

#include "InputManager.h"
#include "CardInteraction.h"
#include "TurnSystem.h"
#include <iostream>
#include <filesystem>
#include <algorithm>

#pragma comment(lib, "gdiplus.lib")

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
    MyGame() : m_turnSystem(m_eventBus, m_rules, "Jugador") {}

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
        Gdiplus::GdiplusStartupInput gdiplusInput;
        if (Gdiplus::GdiplusStartup(&m_gdiplusToken, &gdiplusInput, nullptr) != Gdiplus::Ok)
        {
            std::cerr << "ERROR: No se pudo inicializar GDI+" << std::endl;
            return false;
        }
        std::cout << "GDI+ inicializado correctamente" << std::endl;
        OnResize(width, height);
        auto& resourceManager = ResourceManager::getInstance();
        const std::filesystem::path cardsDirectory = L"Assets/Cartas";
        const std::filesystem::path mapsDirectory = L"Assets/Maps";
        if (std::filesystem::exists(cardsDirectory))
        {
            for (const auto& file : std::filesystem::directory_iterator(cardsDirectory))
            {
                if (file.path().extension() == L".png")
                {
                    Card card;
                    card.frontTexture = file.path().generic_wstring();
                    resourceManager.loadTexture(file.path().generic_string());
                    const float x = -220.0f + static_cast<float>(m_cards.size() % 4) * 145.0f;
                    const float y = 120.0f - static_cast<float>(m_cards.size() / 4) * 190.0f;
                    card.position = { x, y, 0.0f };
                    m_cards.push_back(card);
                }
            }
        }
        if (std::filesystem::exists(mapsDirectory))
        {
            for (const auto& file : std::filesystem::directory_iterator(mapsDirectory))
            {
                if (file.path().extension() == L".png")
                {
                    m_mapTexture = file.path().generic_string();
                    break;
                }
            }
        }
        if (!m_mapTexture.empty())
        {
            resourceManager.loadTexture(m_mapTexture);
        }
        m_turnSystem.SetResourceManager(resourceManager);
        m_turnSystem.SetMapTexture(m_mapTexture);
        m_turnSystem.SetDrawAction([this]()
        {
            if (m_cards.empty()) return std::vector<std::string>{};
            m_drawnCard = m_cards[m_nextCard++ % m_cards.size()].frontTexture;
            return std::vector<std::string>{ std::filesystem::path(m_drawnCard).generic_string() };
        });
        m_interaction.SetCards(&m_cards);
        m_turnSystem.StartTurn();
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
            m_turnSystem.NextPhase();
            std::cout << "Fase actual: " << static_cast<int>(m_turnSystem.CurrentPhase()) << '\n';
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

            const std::wstring& imagePath = m_turnSystem.CurrentPhase() == TurnPhase::Action
                ? std::wstring(m_mapTexture.begin(), m_mapTexture.end())
                : m_drawnCard;
            if (!imagePath.empty() && (m_turnSystem.CurrentPhase() == TurnPhase::Deploy ||
                m_turnSystem.CurrentPhase() == TurnPhase::Action))
            {
                Gdiplus::Image image(imagePath.c_str());
                if (image.GetLastStatus() == Gdiplus::Ok)
                {
                    const int imageWidth = m_turnSystem.CurrentPhase() == TurnPhase::Action ? 700 : 180;
                    const int imageHeight = m_turnSystem.CurrentPhase() == TurnPhase::Action ? 450 : 260;
                    Gdiplus::Graphics graphics(hdc);
                    graphics.DrawImage(&image, centerX - imageWidth / 2, centerY - imageHeight / 2,
                        imageWidth, imageHeight);
                }
            }

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
        if (m_gdiplusToken != 0)
        {
            Gdiplus::GdiplusShutdown(m_gdiplusToken);
            m_gdiplusToken = 0;
        }
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
    std::wstring m_drawnCard;
    std::string m_mapTexture;
    std::size_t m_nextCard = 0;
    CardInteraction m_interaction;
    EventBus m_eventBus;
    Rules m_rules;
    TurnSystem m_turnSystem;
    bool m_isInitialized = false;
    ULONG_PTR m_gdiplusToken = 0;
};
