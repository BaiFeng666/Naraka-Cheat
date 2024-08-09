#include "OS-ImGui_Base.h"

/****************************************************
* Copyright (C)	: Liv
* @file			: OS-ImGui_Base.cpp
* @author		: Liv
* @email		: 1319923129@qq.com
* @version		: 1.1
* @date			: 2024/4/4 13:59
****************************************************/


namespace OSImGui
{
    bool OSImGui_Base::InitImGui(ID3D11Device* device, ID3D11DeviceContext* device_context)
    {
        ImGui::CreateContext(); // 创建ImGui上下文
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\msyhbd.ttc", 18.0f, nullptr, io.Fonts->GetGlyphRangesChineseFull()); // 添加字体，18.0f是菜单的字体大小
        ImGui::StyleColorsDark(); // 设置ImGui样式为暗色
        io.LogFilename = nullptr; // 禁用日志文件

        if (!ImGui_ImplWin32_Init(Window.hWnd)) // 初始化Win32平台
            throw OSException("ImGui_ImplWin32_Init() call failed."); // 抛出异常
        if (!ImGui_ImplDX11_Init(device, device_context)) // 初始化DirectX 11
            throw OSException("ImGui_ImplDX11_Init() call failed."); // 抛出异常

        return true; // 返回初始化成功
    }

    void OSImGui_Base::CleanImGui()
    {
        ImGui_ImplDX11_Shutdown(); // 关闭DirectX 11
        ImGui_ImplWin32_Shutdown(); // 关闭Win32平台
        ImGui::DestroyContext(); // 销毁ImGui上下文

        if (g_Device.g_pd3dDevice)
            g_Device.CleanupDeviceD3D(); // 清理DirectX设备

        // 仅在外部模式下销毁窗口
        if (Window.hWnd && !Window.Name.empty())
        {
            DestroyWindow(Window.hWnd); // 销毁窗口
            UnregisterClassA(Window.ClassName.c_str(), Window.hInstance); // 注销窗口类
        }
    }

    std::wstring OSImGui_Base::StringToWstring(std::string& str)
    {
        std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
        return converter.from_bytes(str); // 将字符串从UTF-8转换为宽字符串
    }

}
