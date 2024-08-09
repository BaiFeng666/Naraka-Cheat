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
        ImGui::CreateContext(); // ����ImGui������
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\msyhbd.ttc", 18.0f, nullptr, io.Fonts->GetGlyphRangesChineseFull()); // ������壬18.0f�ǲ˵��������С
        ImGui::StyleColorsDark(); // ����ImGui��ʽΪ��ɫ
        io.LogFilename = nullptr; // ������־�ļ�

        if (!ImGui_ImplWin32_Init(Window.hWnd)) // ��ʼ��Win32ƽ̨
            throw OSException("ImGui_ImplWin32_Init() call failed."); // �׳��쳣
        if (!ImGui_ImplDX11_Init(device, device_context)) // ��ʼ��DirectX 11
            throw OSException("ImGui_ImplDX11_Init() call failed."); // �׳��쳣

        return true; // ���س�ʼ���ɹ�
    }

    void OSImGui_Base::CleanImGui()
    {
        ImGui_ImplDX11_Shutdown(); // �ر�DirectX 11
        ImGui_ImplWin32_Shutdown(); // �ر�Win32ƽ̨
        ImGui::DestroyContext(); // ����ImGui������

        if (g_Device.g_pd3dDevice)
            g_Device.CleanupDeviceD3D(); // ����DirectX�豸

        // �����ⲿģʽ�����ٴ���
        if (Window.hWnd && !Window.Name.empty())
        {
            DestroyWindow(Window.hWnd); // ���ٴ���
            UnregisterClassA(Window.ClassName.c_str(), Window.hInstance); // ע��������
        }
    }

    std::wstring OSImGui_Base::StringToWstring(std::string& str)
    {
        std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
        return converter.from_bytes(str); // ���ַ�����UTF-8ת��Ϊ���ַ���
    }

}
