#include "pch.h"
#include "App.h"


App::App()
{
    s_pApp = this;
    CPU_CALLBACK_START(OnStart);
    CPU_CALLBACK_UPDATE(OnUpdate);
    CPU_CALLBACK_EXIT(OnExit);
    CPU_CALLBACK_RENDER(OnRender);
}

App::~App()
{
}

void App::OnStart()
{
}

void App::OnUpdate()
{
}

void App::OnExit()
{
}

void App::OnRender(int pass)
{
}

void App::MyPixelShader(cpu_ps_io& io)
{
}
