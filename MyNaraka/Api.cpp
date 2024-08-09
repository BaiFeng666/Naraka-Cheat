#include "Api.h"

BOOL Api::EnableDebugPrivilege() {
    // 获取进程令牌
    HANDLE hToken;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
        return false;
    }

    // 查找调试特权的LUID
    LUID sedebugnameValue;
    if (!LookupPrivilegeValue(nullptr, SE_DEBUG_NAME, &sedebugnameValue)) {
        CloseHandle(hToken);
        return false;
    }

    // 启用调试特权
    TOKEN_PRIVILEGES tkp;
    tkp.PrivilegeCount = 1;
    tkp.Privileges[0].Luid = sedebugnameValue;
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    if (!AdjustTokenPrivileges(hToken, FALSE, &tkp, sizeof(TOKEN_PRIVILEGES), nullptr, nullptr)) {
        CloseHandle(hToken);
        return false;
    }

    // 关闭句柄并返回成功
    CloseHandle(hToken);
    return true;
}

VOID Api::GetWindowSize(HWND hwnd, uint32_t& width, uint32_t& height, float& centerX, float& centerY) {
    RECT clientRect = { 0 };
    if (GetClientRect(hwnd, &clientRect)) {
        width = clientRect.right;
        height = clientRect.bottom;
        centerX = static_cast<float>(width) / 2.0f;
        centerY = static_cast<float>(height) / 2.0f;
    }
}

BOOL Api::WindowActivation(HWND hwnd) {
    WINDOWINFO windowInfo = { sizeof(WINDOWINFO) };
    if (GetWindowInfo(hwnd, &windowInfo)) {
        return windowInfo.dwWindowStatus == 1;
    }
    return false;
}

BOOL Api::CloseProgram_DEP() {
    DEP_SYSTEM_POLICY_TYPE systemDepPolicy = GetSystemDEPPolicy();
    if (systemDepPolicy >= DEPPolicyOptIn) {
        return SetProcessDEPPolicy(0);
    }
    return false;
}