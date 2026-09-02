#pragma once

#include <cstddef>

class Rules
{
public:
    explicit Rules(std::size_t territoriesToWin = 3) noexcept
        : m_territoriesToWin(territoriesToWin) {}

    bool HasWon(std::size_t controlledTerritories) const noexcept
    {
        return controlledTerritories >= m_territoriesToWin;
    }

    int ApplyModifiers(int value, int bonus, int penalty) const noexcept
    {
        return value + bonus - penalty;
    }

    std::size_t TerritoriesToWin() const noexcept { return m_territoriesToWin; }
    void SetTerritoriesToWin(std::size_t value) noexcept { m_territoriesToWin = value; }

private:
    std::size_t m_territoriesToWin;
};
