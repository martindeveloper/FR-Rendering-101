#pragma once

#ifndef APPLICATION_WINDOW_H
#define APPLICATION_WINDOW_H

#include "../../Platform/Platform.h"
#include "../ApplicationWindowProperties.h"

class ApplicationWindow
{
private:
    ApplicationWindowProperties *Properties;

public:
    ApplicationWindow(ApplicationWindowProperties *properties);
    ~ApplicationWindow();

    void OnCreate(HWND windowHandle);
    void OnQuit();
    void OnPaint();
    void OnSetCursor();
};

#endif // APPLICATION_WINDOW_H
