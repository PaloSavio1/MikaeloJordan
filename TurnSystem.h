#pragma once

#include "EventBus.h"
#include "ResourceManager.h"
#include "Rules.h"
#include <functional>
#include <string>
#include <utility>
#include <vector>

enum class TurnPhase
{
    Draw,
    Deploy,
    Action,
    Resolution
};

class TurnSystem
{
public:
    using DrawCards = std::function<std::vector<std::string>()>;
    using PhaseAction = std::function<void()>;
    using CardValidator = std::function<bool(const std::string&)>;

    TurnSystem(EventBus& eventBus, Rules& rules, std::string playerName)
        : m_eventBus(eventBus), m_rules(rules), m_playerName(std::move(playerName)) {}

    void StartTurn() noexcept
    {
        m_currentPhase = TurnPhase::Draw;
        m_turnActive = true;
        m_victory = false;
    }

    bool IsTurnActive() const noexcept { return m_turnActive; }
    TurnPhase CurrentPhase() const noexcept { return m_currentPhase; }
    std::size_t TurnNumber() const noexcept { return m_turnNumber; }
    bool HasWon() const noexcept { return m_victory; }

    void SetDrawAction(DrawCards action) { m_drawAction = std::move(action); }
    void SetDeployAction(PhaseAction action) { m_deployAction = std::move(action); }
    void SetActionPhase(PhaseAction action) { m_actionPhase = std::move(action); }
    void SetResolutionAction(PhaseAction action) { m_resolutionAction = std::move(action); }
    void SetCardValidator(CardValidator validator) { m_cardValidator = std::move(validator); }
    void SetResourceManager(ResourceManager& resourceManager) noexcept { m_resourceManager = &resourceManager; }
    void SetMapTexture(std::string path) { m_mapTexture = std::move(path); }

    bool PlayCard(std::string cardName)
    {
        if (!m_turnActive || m_currentPhase != TurnPhase::Deploy ||
            (m_cardValidator && !m_cardValidator(cardName)))
        {
            return false;
        }

        m_eventBus.Publish(CardPlayedEvent(std::move(cardName)));
        return true;
    }

    void ConquerTerritory(std::string territoryName, std::string factionName)
    {
        if (!m_turnActive || m_currentPhase != TurnPhase::Action) return;

        ++m_controlledTerritories;
        m_eventBus.Publish(TerritoryConqueredEvent(std::move(territoryName), std::move(factionName)));
    }

    void Defeat()
    {
        if (!m_turnActive) return;
        m_turnActive = false;
        m_eventBus.Publish(PlayerDefeatedEvent(m_playerName));
    }

    void SetControlledTerritories(std::size_t count) noexcept
    {
        m_controlledTerritories = count;
    }

    bool NextPhase()
    {
        if (!m_turnActive) return false;

        switch (m_currentPhase)
        {
        case TurnPhase::Draw:
            ExecuteDraw();
            if (!m_turnActive) return false;
            m_currentPhase = TurnPhase::Deploy;
            break;
        case TurnPhase::Deploy:
            if (m_deployAction) m_deployAction();
            m_currentPhase = TurnPhase::Action;
            break;
        case TurnPhase::Action:
            if (m_resourceManager && !m_mapTexture.empty())
            {
                m_resourceManager->loadTexture(m_mapTexture);
            }
            if (m_actionPhase) m_actionPhase();
            m_currentPhase = TurnPhase::Resolution;
            break;
        case TurnPhase::Resolution:
            if (m_resolutionAction) m_resolutionAction();
            CheckRules();
            if (m_turnActive)
            {
                ++m_turnNumber;
                m_currentPhase = TurnPhase::Draw;
            }
            break;
        }
        return m_turnActive;
    }

private:
    void ExecuteDraw()
    {
        if (!m_drawAction) return;

        const std::vector<std::string> cards = m_drawAction();
        if (cards.empty())
        {
            Defeat();
            return;
        }
        for (const std::string& card : cards) m_eventBus.Publish(CardDrawnEvent(card));
        if (m_resourceManager)
        {
            for (const std::string& card : cards) m_resourceManager->loadTexture(card);
        }
    }

    void CheckRules()
    {
        if (m_rules.HasWon(m_controlledTerritories))
        {
            m_victory = true;
            m_turnActive = false;
        }
    }

    EventBus& m_eventBus;
    Rules& m_rules;
    std::string m_playerName;
    TurnPhase m_currentPhase = TurnPhase::Draw;
    std::size_t m_turnNumber = 1;
    std::size_t m_controlledTerritories = 0;
    bool m_turnActive = false;
    bool m_victory = false;
    DrawCards m_drawAction;
    PhaseAction m_deployAction;
    PhaseAction m_actionPhase;
    PhaseAction m_resolutionAction;
    CardValidator m_cardValidator;
    ResourceManager* m_resourceManager = nullptr;
    std::string m_mapTexture;
};
