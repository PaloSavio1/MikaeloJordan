#include "Win32Application.h"
#include "Game.h"
#include <cstdio>

// Función para crear una instancia del juego
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)
{
    // Desactivar buffer de consola para mejor rendimiento
    // (Opcional - si quieres ver outputs de debug)
    AllocConsole();
    FILE* fDummy;
    freopen_s(&fDummy, "CONOUT$", "w", stdout);
    freopen_s(&fDummy, "CONOUT$", "w", stderr);
    std::cout << "Game starting..." << std::endl;
    
    Win32Application app;
    
    if (!app.Initialize(hInstance, L"Mi Juego Win32 - VS2022", 800, 600))
    {
        MessageBox(nullptr, L"Failed to initialize application!", L"Error", MB_OK | MB_ICONERROR);
        return -1;
    }
    
    return app.Run();
}