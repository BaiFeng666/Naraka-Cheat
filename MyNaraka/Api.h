#pragma once
#include <iostream>
#include <windows.h>

using namespace std;

namespace Api {
	//进程提升权限
	BOOL EnableDebugPrivilege();
	//获取窗口大小
	VOID GetWindowSize(HWND hwnd, uint32_t& width, uint32_t& height, float& centerX, float& centerY);
	//判断窗口是否激活
	BOOL WindowActivation(HWND hwnd);
	//关闭系统DEP保护
	BOOL CloseProgram_DEP();
}

