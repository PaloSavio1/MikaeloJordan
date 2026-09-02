#include "Win32Application.h"
#include "Game.h"
#include <cstdio>
#include <iostream>
#include <memory>

// Función principal de Windows
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)
{
    // ============================================
    // 1. CONFIGURACIÓN DE CONSOLA (MODO DEBUG)
    // ============================================
    #ifdef _DEBUG
    if (AllocConsole())
    {
        FILE* fDummy;
        freopen_s(&fDummy, "CONOUT$", "w", stdout);
        freopen_s(&fDummy, "CONOUT$", "w", stderr);
        SetConsoleTitle(L"Debug Console - N.C. - Núcleo de Cristal");
        std::cout << "========================================" << std::endl;
        std::cout << "   N.C. - NUCLEO DE CRISTAL" << std::endl;
        std::cout << "   Version: 0.1.0" << std::endl;
        std::cout << "   Starting..." << std::endl;
        std::cout << "========================================" << std::endl << std::endl;
    }
    #endif

    // ============================================
    // 2. CONFIGURACIÓN DEL JUEGO
    // ============================================
    const wchar_t* WINDOW_TITLE = L"N.C. - Núcleo de Cristal";
    const int WINDOW_WIDTH = 1024;
    const int WINDOW_HEIGHT = 768;

    auto app = std::make_unique<Win32Application>();

    // ============================================
    // 3. INICIALIZACIÓN
    // ============================================
    std::cout << "[INFO] Inicializando aplicación..." << std::endl;

    if (!app->Initialize(hInstance, WINDOW_TITLE, WINDOW_WIDTH, WINDOW_HEIGHT))
    {
        std::cerr << "[ERROR] Falló la inicialización de la aplicación!" << std::endl;
        MessageBox(nullptr,
            L"No se pudo inicializar la aplicación.\n"
            L"Por favor, verifica los requisitos del sistema.",
            L"Error de Inicialización",
            MB_OK | MB_ICONERROR);
        return -1;
    }

    std::cout << "[INFO] Aplicación inicializada correctamente" << std::endl;
    std::cout << "[INFO] Ventana creada: " << WINDOW_WIDTH << "x" << WINDOW_HEIGHT << std::endl;

    // ============================================
    // 4. BUCLE PRINCIPAL DEL JUEGO
    // ============================================
    std::cout << "[INFO] Iniciando bucle principal del juego..." << std::endl;

    int exitCode = app->Run();

    // ============================================
    // 5. LIMPIEZA Y CIERRE
    // ============================================
    std::cout << "[INFO] Cerrando aplicación..." << std::endl;
    std::cout << "[INFO] Código de salida: " << exitCode << std::endl;

    #ifdef _DEBUG
    std::cout << "\nPresiona ENTER para cerrar..." << std::endl;
    std::cin.get();
    #endif

    return exitCode;
}