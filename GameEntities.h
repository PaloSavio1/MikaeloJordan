#pragma once

#include "Entity.h"
#include "Faction.h"
#include <string>
#include <utility>

class Player final : public Entity
{
public:
    explicit Player(std::string name) : m_name(std::move(name)) {}

    const std::string& Name() const noexcept { return m_name; }
    Faction* GetFaction() const noexcept { return m_faction; }
    void SetFaction(Faction* faction) noexcept { m_faction = faction; }
    bool IsDefeated() const noexcept { return m_defeated; }
    void Defeat() noexcept { m_defeated = true; }

    void update() override {}
    void render() override {}

private:
    std::string m_name;
    Faction* m_faction = nullptr;
    bool m_defeated = false;
};

class Territory final : public Entity
{
public:
    explicit Territory(std::string name) : m_name(std::move(name)) {}

    const std::string& Name() const noexcept { return m_name; }
    Faction* Owner() const noexcept { return m_owner; }
    void SetOwner(Faction* owner) noexcept { m_owner = owner; }

    void update() override {}
    void render() override {}

private:
    std::string m_name;
    Faction* m_owner = nullptr;
};
