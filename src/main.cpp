#include <exception>
#include <iostream>

#include "app/Application.h"

int main()
{
    try
    {
        Application app;
        return app.Run();
    }
    catch (const std::exception& exception)
    {
        std::cerr << "Fatal error: " << exception.what() << std::endl;
        return 1;
    }
    catch (...)
    {
        std::cerr << "Fatal error: unknown exception." << std::endl;
        return 1;
    }
}