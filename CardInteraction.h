#pragma once

#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <DirectXMath.h>
#include <string>
#include <vector>
#include "Entity.h"
#include "InputManager.h"

struct Card : public Entity
{
    std::wstring frontTexture;
    DirectX::XMFLOAT3 position{};
    float yaw = 0.0f;
    float pitch = 0.0f;
    float zoom = 1.0f;
    bool hover = false;
    bool flipping = false;
    float flipStartYaw = 0.0f;
    float flipTargetYaw = 0.0f;
    float flipTime = 0.0f;

    DirectX::XMMATRIX WorldMatrix() const;
    const std::wstring& Texture(bool back) const;

    void update() override {}
    void render() override {}
};

class CardInteraction
{
public:
    void SetViewport(int width, int height);
    void SetCards(std::vector<Card>* cards);
    void Update(float deltaTime, InputManager& input);
    int Pick(int mouseX, int mouseY) const;
    int Selected() const { return m_selected; }
    bool IsBack(const Card& card) const;

private:
    DirectX::XMMATRIX View() const;
    DirectX::XMMATRIX Projection() const;
    std::vector<Card>* m_cards = nullptr;
    int m_width = 1;
    int m_height = 1;
    int m_selected = -1;
    int m_dragCard = -1;
    int m_lastMouseX = 0;
    int m_lastMouseY = 0;
};
