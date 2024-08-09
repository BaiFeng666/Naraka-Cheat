#pragma once
#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
#include "Api.h"
#include "OS-ImGui/OS-ImGui.h"
#include "Offsets.h"

#include <iostream>
#include <windows.h>
#include <chrono>

#define PI 3.1415926535;

struct GameWindow {
	HWND Hwnd;
	uint32_t  Pid;
	GameWindow() : Hwnd(NULL), Pid(NULL) {}
	GameWindow(HWND inHwnd, uint32_t inPid) : Hwnd(inHwnd), Pid(inPid) {}
};

struct ModuleData {
	uint64_t GameAssembly;
	uint64_t UWorld;
	ModuleData() : GameAssembly(NULL), UWorld(NULL) {}
	ModuleData(uint64_t inGameAssembly, uint64_t inUWorld) : GameAssembly(inGameAssembly), UWorld(inUWorld) {}
};

//功能
struct MenuInfo {
	bool Box;//方框
	bool Bone;//骨骼
	bool Hp;//血条
	bool Shield;//护甲
	bool Info;//信息
	bool Item;//物品
	bool Building;//建筑
	bool ZhenDao;//振刀
	bool FangShi;//振刀方式
	bool FangXu;//放蓄
	bool ZhuiZong;//追踪
	float ZhuiZongRadius;//追踪范围
	bool Aim;//自瞄
	float AimValue;//自瞄向量
	float AimRadius;//自瞄范围
	ImColor BoxColor;
	ImColor BoneColor;
	ImColor InfoColor;
	
	//开关数值
	MenuInfo() : Box(true), Bone(true), Hp(true), Shield(true), Info(true), Item(true), Building(true), ZhenDao(true), FangShi(true), FangXu(false), ZhuiZong(false), ZhuiZongRadius(2.0f), Aim(false), AimValue(0.9f), AimRadius(200.0f), BoxColor(192, 192, 192), BoneColor(0, 255, 0), InfoColor(192, 192, 192) {}
	MenuInfo(bool inBox, bool inBone, bool inHp, bool inShield, bool inInfo, bool inItem, bool inBuilding, bool inZhenDao, bool inFangShi, bool inFangXu,bool inZhuiZong, float inZhuiZongRadius, bool inAim, float inAimValue, float inAimRadius, ImColor inBoxColor, ImColor inBoneColor, ImColor inInfoColor) : Box(inBox), Bone(inBone), Hp(inHp), Shield(inShield), Info(inInfo), Item(inItem), Building(inBuilding), ZhenDao(inZhenDao), FangShi(inFangShi), FangXu(inFangXu), ZhuiZong(inZhuiZong), ZhuiZongRadius(inZhuiZongRadius), Aim(inAim), AimValue(inAimValue), AimRadius(inAimRadius), BoxColor(inBoxColor), BoneColor(inBoneColor), InfoColor(inInfoColor) {}
};
struct Vector2 {
	float x, y;
	Vector2() : x(0.0f), y(0.0f) {}
	Vector2(float inX, float inY) : x(inX), y(inY) {}
};

struct Vector3 {
	
	float x, y, z;
	Vector3() : x(0.0f), y(0.0f), z(0.0f) {}
	Vector3(float inX, float inY, float inZ) : x(inX), y(inY), z(inZ) {}
};



struct Vector4 {
	float x, y, z, w;
	Vector4() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}
	Vector4(float inX, float inY, float inZ, float inW) : x(inX), y(inY), z(inZ), w(inW) {}
};



struct Rotation {
	float Pitch;
	float Yaw;
	float Roll;
	Rotation() : Pitch(0.0f), Yaw(0.0f), Roll(0.0f) {}
	Rotation(float inPitch, float inYaw, float inRoll) : Pitch(inPitch), Yaw(inYaw), Roll(inRoll) {}
};

struct PropertyData {
	uint32_t CurHp;
	uint32_t HpMax;
	uint32_t CurShield;
	uint32_t ShieldMax;
	PropertyData() : CurHp(0), HpMax(0), CurShield(0), ShieldMax(0) {}
	PropertyData(uint32_t inCurHp, uint32_t inHpMax, uint32_t inCurShield, uint32_t inShieldMax) : CurHp(inCurHp), HpMax(inHpMax), CurShield(inCurShield), ShieldMax(inShieldMax) {}
};

struct DataInfo {
	string Name;
	ImColor Color;
	DataInfo() : Name("Item"), Color(IM_COL32(255, 255, 255, 255)) {}
};

namespace Aim
{
	uint64_t Knifevtable;
	uint64_t FlushAddr;
}

GameWindow g_Window;
ModuleData g_Module;
MenuInfo g_MenuInfo;

uint64_t g_LocalPlayerAddress, g_MatrixAddress;
FLOAT g_MatrixData[4][4];
vector<uint64_t> g_PlayerArray;
BOOL g_PlayerRefresh;
uint64_t g_KeyTime;