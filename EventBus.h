#pragma once

#include <functional>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>

struct GameEvent
{
    virtual ~GameEvent() = default;
};

struct CardDrawnEvent final : GameEvent
{
    explicit CardDrawnEvent(std::string cardName) : name(std::move(cardName)) {}
    std::string name;
};

struct CardPlayedEvent final : GameEvent
{
    explicit CardPlayedEvent(std::string cardName) : name(std::move(cardName)) {}
    std::string name;
};

struct TerritoryConqueredEvent final : GameEvent
{
    TerritoryConqueredEvent(std::string territoryName, std::string factionName)
        : territory(std::move(territoryName)), faction(std::move(factionName)) {}
    std::string territory;
    std::string faction;
};

struct PlayerDefeatedEvent final : GameEvent
{
    explicit PlayerDefeatedEvent(std::string playerName) : player(std::move(playerName)) {}
    std::string player;
};

class EventBus
{
public:
    template <typename T>
    void Subscribe(std::function<void(const T&)> listener)
    {
        m_listeners[typeid(T)].push_back(
            [listener = std::move(listener)](const GameEvent& event)
            {
                listener(static_cast<const T&>(event));
            });
    }

    template <typename T>
    void Publish(const T& event) const
    {
        const auto found = m_listeners.find(typeid(T));
        if (found == m_listeners.end()) return;
        for (const auto& listener : found->second) listener(event);
    }

private:
    using Listener = std::function<void(const GameEvent&)>;
    std::unordered_map<std::type_index, std::vector<Listener>> m_listeners;
};
