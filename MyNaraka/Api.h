#pragma once
#include <iostream>
#include <windows.h>

using namespace std;

namespace Api {
	//��������Ȩ��
	BOOL EnableDebugPrivilege();
	//��ȡ���ڴ�С
	VOID GetWindowSize(HWND hwnd, uint32_t& width, uint32_t& height, float& centerX, float& centerY);
	//�жϴ����Ƿ񼤻�
	BOOL WindowActivation(HWND hwnd);
	//�ر�ϵͳDEP����
	BOOL CloseProgram_DEP();
}

