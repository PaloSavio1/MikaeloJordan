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
        auto findResourceDirectory = [](const std::filesystem::path& directoryName)
        {
            std::filesystem::path directory = std::filesystem::current_path();
            for (int level = 0; level < 5; ++level)
            {
                const std::filesystem::path candidate = directory / directoryName;
                if (std::filesystem::exists(candidate) && std::filesystem::is_directory(candidate))
                {
                    return candidate;
                }

                if (directory == directory.root_path())
                {
                    break;
                }
                directory = directory.parent_path();
            }
            return std::filesystem::path{};
        };

        const std::filesystem::path cardsDirectory = findResourceDirectory(L"cartas");
        const std::filesystem::path mapsDirectory = findResourceDirectory(L"Maps");
        std::cout << "Directorio actual: " << std::filesystem::current_path().string() << std::endl;
        std::cout << "Directorio de cartas: "
                  << (cardsDirectory.empty() ? "no encontrado" : cardsDirectory.string()) << std::endl;
        std::cout << "Directorio de mapas: "
                  << (mapsDirectory.empty() ? "no encontrado" : mapsDirectory.string()) << std::endl;
        if (cardsDirectory.empty())
        {
            std::cerr << "ERROR: No se encontro la carpeta cartas" << std::endl;
        }
        if (mapsDirectory.empty())
        {
            std::cerr << "ERROR: No se encontro la carpeta Maps" << std::endl;
        }

        if (!cardsDirectory.empty())
        {
            for (const auto& file : std::filesystem::directory_iterator(cardsDirectory))
            {
                const std::wstring extension = file.path().extension().wstring();
                if (extension == L".png" || extension == L".jpg" || extension == L".jpeg")
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
        if (!mapsDirectory.empty())
        {
            for (const auto& file : std::filesystem::directory_iterator(mapsDirectory))
            {
                const std::wstring extension = file.path().extension().wstring();
                if (extension == L".png" || extension == L".jpg" || extension == L".jpeg")
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

        if (m_turnSystem.CurrentPhase() == TurnPhase::Deploy)
        {
            const bool left = m_input->IsKeyPressed(KeyCode::A) || m_input->IsKeyPressed(KeyCode::Left);
            const bool right = m_input->IsKeyPressed(KeyCode::D) || m_input->IsKeyPressed(KeyCode::Right);
            const bool up = m_input->IsKeyPressed(KeyCode::W) || m_input->IsKeyPressed(KeyCode::Up);
            const bool down = m_input->IsKeyPressed(KeyCode::S) || m_input->IsKeyPressed(KeyCode::Down);
            const bool confirm = m_input->IsKeyPressed(KeyCode::Enter);

            if (m_interactionMode == InteractionMode::Hand)
            {
                if (left) MoveHandCursor(-1);
                if (right) MoveHandCursor(1);
                if (confirm && m_handCursor >= 0)
                {
                    m_selectedCard = m_handCursor;
                    m_interactionMode = InteractionMode::Board;
                    m_boardRow = 0;
                    m_boardColumn = 0;
                    std::cout << "Carta seleccionada: " << m_selectedCard << '\n';
                }
            }
            else
            {
                if (left) m_boardColumn = std::max(m_boardColumn - 1, 0);
                if (right) m_boardColumn = std::min(m_boardColumn + 1, boardColumns - 1);
                if (up) m_boardRow = std::max(m_boardRow - 1, 0);
                if (down) m_boardRow = std::min(m_boardRow + 1, boardRows - 1);
                if (confirm) PlaceSelectedCard();
                if (m_input->IsKeyPressed(KeyCode::Escape))
                {
                    m_selectedCard = -1;
                    m_interactionMode = InteractionMode::Hand;
                }
            }
        }

        if (m_input->IsMouseButtonPressed(MouseButton::Left))
        {
            std::cout << "Mouse clicked at: " << m_input->GetMouseX()
                      << ", " << m_input->GetMouseY() << '\n';
        }

        if (m_input->IsKeyPressed(KeyCode::Escape) && m_interactionMode == InteractionMode::Hand)
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

        HDC windowDc = GetDC(m_window);
        if (!windowDc)
        {
            return;
        }

        HDC backBufferDc = CreateCompatibleDC(windowDc);
        HBITMAP backBufferBitmap = CreateCompatibleBitmap(windowDc, m_width, m_height);
        if (!backBufferDc || !backBufferBitmap)
        {
            if (backBufferDc) DeleteDC(backBufferDc);
            if (backBufferBitmap) DeleteObject(backBufferBitmap);
            ReleaseDC(m_window, windowDc);
            return;
        }

        HGDIOBJ previousBitmap = SelectObject(backBufferDc, backBufferBitmap);
        HDC hdc = backBufferDc;
        RECT rect{};
        GetClientRect(m_window, &rect);
        HBRUSH brush = CreateSolidBrush(RGB(0, 0, 0));
        HPEN pen = CreatePen(PS_SOLID, 2, RGB(255, 255, 255));

        if (brush && pen)
        {
            FillRect(hdc, &rect, brush);
            HGDIOBJ oldPen = SelectObject(hdc, pen);

            constexpr int mapWidth = 700;
            constexpr int mapHeight = 450;
            constexpr int mapTop = 35;
            constexpr int cardWidth = 140;
            constexpr int cardHeight = 200;
            constexpr int cardGap = 15;
            constexpr int handTop = mapTop + mapHeight + 25;

            const int centerX = m_width / 2;
            const int mapLeft = centerX - mapWidth / 2;
            if (!m_mapTexture.empty())
            {
                const std::wstring imagePath(m_mapTexture.begin(), m_mapTexture.end());
                Gdiplus::Image image(imagePath.c_str());
                if (image.GetLastStatus() == Gdiplus::Ok)
                {
                    Gdiplus::Graphics graphics(hdc);
                    graphics.DrawImage(&image, mapLeft, mapTop, mapWidth, mapHeight);
                }
            }

            if (!m_cards.empty())
            {
                int cardsInHand = 0;
                for (const Card& card : m_cards)
                {
                    if (card.inHand) ++cardsInHand;
                }
                const int totalWidth = cardsInHand * cardWidth +
                    std::max(cardsInHand - 1, 0) * cardGap;
                const int handLeft = std::max((m_width - totalWidth) / 2, 10);
                Gdiplus::Graphics graphics(hdc);
                int handPosition = 0;

                for (std::size_t i = 0; i < m_cards.size(); ++i)
                {
                    const Card& card = m_cards[i];
                    if (!card.inHand || card.frontTexture.empty()) continue;

                    Gdiplus::Image image(card.frontTexture.c_str());
                    if (image.GetLastStatus() != Gdiplus::Ok) continue;

                    const int cardLeft = handLeft + handPosition * (cardWidth + cardGap);
                    graphics.DrawImage(&image, cardLeft, handTop, cardWidth, cardHeight);

                    if (static_cast<int>(i) == m_selectedCard ||
                        (m_interactionMode == InteractionMode::Hand && static_cast<int>(i) == m_handCursor))
                    {
                        Gdiplus::Pen selectionPen(Gdiplus::Color(255, 255, 215, 0), 4.0f);
                        graphics.DrawRectangle(&selectionPen, cardLeft - 2, handTop - 2,
                            cardWidth + 4, cardHeight + 4);
                    }
                    ++handPosition;
                }
            }

            if (!m_boardCards.empty())
            {
                constexpr int boardOriginX = 470;
                constexpr int boardOriginY = 100;
                constexpr int boardCellWidth = 70;
                constexpr int boardCellHeight = 90;
                constexpr int placedWidth = 60;
                constexpr int placedHeight = 75;
                Gdiplus::Graphics graphics(hdc);

                for (const BoardCard& placed : m_boardCards)
                {
                    const Card& card = m_cards[placed.cardIndex];
                    Gdiplus::Image image(card.frontTexture.c_str());
                    if (image.GetLastStatus() != Gdiplus::Ok) continue;
                    const int x = boardOriginX + placed.column * boardCellWidth +
                        (boardCellWidth - placedWidth) / 2;
                    const int y = boardOriginY + placed.row * boardCellHeight +
                        (boardCellHeight - placedHeight) / 2;
                    graphics.DrawImage(&image, x, y, placedWidth, placedHeight);
                }
            }

            if (m_interactionMode == InteractionMode::Board)
            {
                constexpr int boardOriginX = 470;
                constexpr int boardOriginY = 100;
                constexpr int boardCellWidth = 70;
                constexpr int boardCellHeight = 90;
                Gdiplus::Graphics graphics(hdc);
                Gdiplus::Pen cursorPen(Gdiplus::Color(255, 80, 180, 255), 4.0f);
                graphics.DrawRectangle(&cursorPen,
                    boardOriginX + m_boardColumn * boardCellWidth,
                    boardOriginY + m_boardRow * boardCellHeight,
                    boardCellWidth, boardCellHeight);
            }

            std::wstring phaseText = L"Fase: ";
            switch (m_turnSystem.CurrentPhase())
            {
            case TurnPhase::Draw: phaseText += L"DRAW"; break;
            case TurnPhase::Deploy: phaseText += L"DEPLOY"; break;
            case TurnPhase::Action: phaseText += L"ACTION"; break;
            case TurnPhase::Resolution: phaseText += L"RESOLUTION"; break;
            }
            phaseText += m_interactionMode == InteractionMode::Hand
                ? L"  |  MANO"
                : L"  |  TABLERO";
            Gdiplus::Graphics graphics(hdc);
            Gdiplus::Font font(L"Arial", 18.0f);
            Gdiplus::SolidBrush textBrush(Gdiplus::Color(255, 255, 255, 255));
            graphics.DrawString(phaseText.c_str(), -1, &font,
                Gdiplus::PointF(20.0f, 10.0f), &textBrush);

             SelectObject(hdc, oldPen);
        }

        if (pen) DeleteObject(pen);
        if (brush) DeleteObject(brush);

        BitBlt(windowDc, 0, 0, m_width, m_height,
            backBufferDc, 0, 0, SRCCOPY);

        SelectObject(backBufferDc, previousBitmap);
        DeleteObject(backBufferBitmap);
        DeleteDC(backBufferDc);
        ReleaseDC(m_window, windowDc);
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
    static constexpr int boardRows = 2;
    static constexpr int boardColumns = 3;

    enum class InteractionMode
    {
        Hand,
        Board
    };

    struct BoardCard
    {
        std::size_t cardIndex;
        int row;
        int column;
    };

    void MoveHandCursor(int direction)
    {
        if (m_cards.empty()) return;
        int index = m_handCursor;
        do
        {
            index += direction;
            if (index < 0) index = static_cast<int>(m_cards.size()) - 1;
            if (index >= static_cast<int>(m_cards.size())) index = 0;
        } while (!m_cards[index].inHand && index != m_handCursor);

        if (m_cards[index].inHand) m_handCursor = index;
        std::cout << "Cursor de mano: " << m_handCursor << '\n';
    }

    void PlaceSelectedCard()
    {
        if (m_selectedCard < 0 || m_selectedCard >= static_cast<int>(m_cards.size()) ||
            !m_cards[m_selectedCard].inHand)
        {
            return;
        }

        for (const BoardCard& placed : m_boardCards)
        {
            if (placed.row == m_boardRow && placed.column == m_boardColumn) return;
        }

        m_cards[m_selectedCard].inHand = false;
        m_boardCards.push_back({ static_cast<std::size_t>(m_selectedCard), m_boardRow, m_boardColumn });
        std::cout << "Carta colocada: " << m_selectedCard << " en ["
                  << m_boardRow << "," << m_boardColumn << "]\n";
        m_selectedCard = -1;
        m_interactionMode = InteractionMode::Hand;
        MoveHandCursor(1);
    }

private:
    HWND m_window = nullptr;
    InputManager* m_input = nullptr;
    int m_width = 0;
    int m_height = 0;
    InteractionMode m_interactionMode = InteractionMode::Hand;
    int m_handCursor = 0;
    int m_selectedCard = -1;
    int m_boardRow = 0;
    int m_boardColumn = 0;
    std::vector<Card> m_cards;
    std::vector<BoardCard> m_boardCards;
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
