#include "CardInteraction.h"
#include <algorithm>
#include <cmath>

using namespace DirectX;

namespace
{
    constexpr float Pi = 3.14159265358979323846f;
}

XMMATRIX Card::WorldMatrix() const
{
    const float hoverScale = hover ? 1.05f : 1.0f;
    const XMVECTOR p = XMLoadFloat3(&position) + XMVectorSet(0.0f, hover ? 5.0f : 0.0f, (zoom - 1.0f) * 100.0f, 0.0f);
    return XMMatrixScaling(110.0f * hoverScale, 160.0f * hoverScale, 1.0f) *
           XMMatrixRotationRollPitchYaw(pitch, yaw, 0.0f) * XMMatrixTranslationFromVector(p);
}

const std::wstring& Card::Texture(bool back) const
{
    static const std::wstring backTexture = L"Cartas/KcriaGames.png";
    return back ? backTexture : frontTexture;
}

void CardInteraction::SetViewport(int width, int height)
{
    m_width = std::max(width, 1);
    m_height = std::max(height, 1);
}

void CardInteraction::SetCards(std::vector<Card>* cards)
{
    m_cards = cards;
    m_selected = -1;
    m_dragCard = -1;
}

XMMATRIX CardInteraction::View() const
{
    return XMMatrixLookAtLH(XMVectorSet(0, 0, -500, 1), XMVectorZero(), XMVectorSet(0, 1, 0, 0));
}

XMMATRIX CardInteraction::Projection() const
{
    return XMMatrixPerspectiveFovLH(Pi / 3.0f, static_cast<float>(m_width) / m_height, 1.0f, 2000.0f);
}

bool CardInteraction::IsBack(const Card& card) const
{
    return std::cos(card.yaw) < 0.0f;
}

int CardInteraction::Pick(int mouseX, int mouseY) const
{
    if (!m_cards) return -1;
    const float ndcX = 2.0f * mouseX / m_width - 1.0f;
    const float ndcY = 1.0f - 2.0f * mouseY / m_height;
    const XMMATRIX inverseViewProjection = XMMatrixInverse(nullptr, View() * Projection());
    const XMVECTOR origin = XMVector3TransformCoord(XMVectorSet(ndcX, ndcY, 0, 1), inverseViewProjection);
    const XMVECTOR end = XMVector3TransformCoord(XMVectorSet(ndcX, ndcY, 1, 1), inverseViewProjection);
    const XMVECTOR direction = XMVector3Normalize(end - origin);

    int closest = -1;
    float closestDistance = 3.4e38f;
    for (size_t i = 0; i < m_cards->size(); ++i)
    {
        const Card& card = (*m_cards)[i];
        const XMMATRIX inverseWorld = XMMatrixInverse(nullptr, card.WorldMatrix());
        const XMVECTOR localOrigin = XMVector3TransformCoord(origin, inverseWorld);
        const XMVECTOR localEnd = XMVector3TransformCoord(origin + direction, inverseWorld);
        const XMVECTOR localDirection = localEnd - localOrigin;
        const float dz = XMVectorGetZ(localDirection);
        if (std::abs(dz) < 0.00001f) continue;
        const float t = -XMVectorGetZ(localOrigin) / dz;
        if (t < 0.0f) continue;
        const XMVECTOR hit = localOrigin + localDirection * t;
        if (std::abs(XMVectorGetX(hit)) <= 0.5f && std::abs(XMVectorGetY(hit)) <= 0.5f && t < closestDistance)
        {
            closest = static_cast<int>(i);
            closestDistance = t;
        }
    }
    return closest;
}

void CardInteraction::Update(float deltaTime, InputManager& input)
{
    if (!m_cards) return;
    const int hovered = Pick(input.GetMouseX(), input.GetMouseY());
    for (Card& card : *m_cards) card.hover = false;
    if (hovered >= 0) (*m_cards)[hovered].hover = true;

    const bool doubleClick = input.ConsumeDoubleClick(MouseButton::Left);
    if (input.IsMouseButtonPressed(MouseButton::Left) || doubleClick)
    {
        m_selected = Pick(input.GetMouseX(), input.GetMouseY());
        m_dragCard = m_selected;
        m_lastMouseX = input.GetMouseX();
        m_lastMouseY = input.GetMouseY();
        if (m_selected >= 0 && doubleClick)
        {
            Card& card = (*m_cards)[m_selected];
            card.flipping = true;
            card.flipTime = 0.0f;
            card.flipStartYaw = card.yaw;
            card.flipTargetYaw = card.yaw + Pi;
        }
    }
    if (!input.IsMouseButtonDown(MouseButton::Left)) m_dragCard = -1;
    if (m_dragCard >= 0 && input.IsMouseButtonDown(MouseButton::Left))
    {
        Card& card = (*m_cards)[m_dragCard];
        card.yaw += (input.GetMouseX() - m_lastMouseX) * 0.01f;
        card.pitch = std::clamp(card.pitch + (input.GetMouseY() - m_lastMouseY) * 0.01f, -Pi * 0.5f, Pi * 0.5f);
        m_lastMouseX = input.GetMouseX();
        m_lastMouseY = input.GetMouseY();
    }
    if (m_selected >= 0)
    {
        Card& card = (*m_cards)[m_selected];
        card.zoom = std::clamp(card.zoom + input.ConsumeWheelDelta() * 0.001f, 0.5f, 3.0f);
        if (card.flipping)
        {
            card.flipTime += std::max(deltaTime, 0.0f);
            const float t = std::min(card.flipTime / 0.35f, 1.0f);
            const float smooth = t * t * (3.0f - 2.0f * t);
            card.yaw = card.flipStartYaw + (card.flipTargetYaw - card.flipStartYaw) * smooth;
            card.flipping = t < 1.0f;
        }
    }
    else
    {
        input.ConsumeWheelDelta();
    }
}
