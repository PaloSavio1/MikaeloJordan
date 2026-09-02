#pragma once

#include <string>
#include <unordered_set>
#include <utility>

class Faction
{
public:
    Faction() = default;
    explicit Faction(std::string name) : m_name(std::move(name)) {}

    const std::string& Name() const noexcept { return m_name; }
    void SetName(std::string name) { m_name = std::move(name); }

    int Advantage() const noexcept { return m_advantage; }
    int Disadvantage() const noexcept { return m_disadvantage; }
    void SetModifiers(int advantage, int disadvantage) noexcept
    {
        m_advantage = advantage;
        m_disadvantage = disadvantage;
    }

    void AddTerritory(std::string territory) { m_territories.insert(std::move(territory)); }
    void RemoveTerritory(const std::string& territory) { m_territories.erase(territory); }
    bool Controls(const std::string& territory) const
    {
        return m_territories.find(territory) != m_territories.end();
    }
    std::size_t TerritoryCount() const noexcept { return m_territories.size(); }

private:
    std::string m_name;
    int m_advantage = 0;
    int m_disadvantage = 0;
    std::unordered_set<std::string> m_territories;
};
