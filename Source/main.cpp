#include "Core/Application.h"

#include <Windows.h>
#include <exception>
#include <string>

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int showCommand)
{
    try
    {
        Framework::Application application(instance);
        if (!application.Initialize(showCommand))
        {
            return EXIT_FAILURE;
        }

        return application.Run();
    }
    catch (const std::exception& exception)
    {
        const std::string message = exception.what();
        MessageBoxA(nullptr, message.c_str(), "D3D11Framework fatal error", MB_OK | MB_ICONERROR);
        return EXIT_FAILURE;
    }
}
