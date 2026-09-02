#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <fstream>
#include <gdiplus.h>
#include <iostream>

class ResourceManager
{
public:
    struct Asset
    {
        std::wstring path;
        std::wstring type;
    };

    static ResourceManager& getInstance() noexcept
    {
        static ResourceManager instance;
        return instance;
    }

    std::shared_ptr<const Asset> loadTexture(const std::string& path)
    {
        std::cout << "Cargando textura: " << path << std::endl;

        std::ifstream file(path, std::ios::binary);
        if (!file.good())
        {
            std::cerr << "ERROR: Archivo no encontrado: " << path << std::endl;
            return nullptr;
        }
        file.close();

        const std::wstring widePath(path.begin(), path.end());
        Gdiplus::Bitmap bitmap(widePath.c_str());
        if (bitmap.GetLastStatus() != Gdiplus::Ok)
        {
            std::cerr << "ERROR: GDI+ no pudo cargar: " << path << std::endl;
            return nullptr;
        }

        std::cout << "Textura cargada exitosamente: " << path << std::endl;
        return Load(std::wstring(path.begin(), path.end()), L"texture");
    }

    std::shared_ptr<const Asset> Load(const std::wstring& path, const std::wstring& type)
    {
        const auto found = m_assets.find(path);
        if (found != m_assets.end()) return found->second;

        auto asset = std::make_shared<Asset>(Asset{ path, type });
        m_assets.emplace(path, asset);
        return asset;
    }

    std::shared_ptr<const Asset> Find(const std::wstring& path) const
    {
        const auto found = m_assets.find(path);
        return found == m_assets.end() ? nullptr : found->second;
    }

    void Clear() noexcept { m_assets.clear(); }
    std::size_t Size() const noexcept { return m_assets.size(); }

private:
    ResourceManager() = default;
    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;

    std::unordered_map<std::wstring, std::shared_ptr<const Asset>> m_assets;
};
