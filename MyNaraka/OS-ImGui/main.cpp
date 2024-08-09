#include <iostream>
#include "OS-ImGui.h"
#include <chrono>

void DrawCallBack()
{
	ImGui::Begin("Menu");
	{
		ImGui::Text("This is a text.");
		if (ImGui::Button("Quit"))
		{
			Gui.Quit();
			//...
		}
		static bool a = false, b = false, c = false, d = false;
		static float Value = 0;
		float min = 0, max = 100;
		Gui.MyCheckBox("CheckBox1", &a);
		Gui.MyCheckBox2("CheckBox2", &b);
		Gui.MyCheckBox3("CheckBox3", &c);
		Gui.MyCheckBox4("CheckBox4", &d);
		Gui.SliderScalarEx1("[Slider]", ImGuiDataType_Float, &Value, &min, &max, "%.1f", ImGuiSliderFlags_None);
	}ImGui::End();

	Gui.ShadowRectFilled({ 50,50 }, { 100,100 }, ImColor(220, 190, 99, 255), ImColor(50, 50, 50, 255), 9, { 0,0 }, 10);
	Gui.ShadowCircle({ 200,200 }, 30, ImColor(220, 190, 99, 255), ImColor(50, 50, 50, 255), 9, { 0,0 });

	//...
}

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
	MenuInfo(bool inBox, bool inBone, bool inHp, bool inShield, bool inInfo, bool inItem, bool inBuilding, bool inZhenDao, bool inFangShi, bool inFangXu, bool inZhuiZong, float inZhuiZongRadius, bool inAim, float inAimValue, float inAimRadius, ImColor inBoxColor, ImColor inBoneColor, ImColor inInfoColor) : Box(inBox), Bone(inBone), Hp(inHp), Shield(inShield), Info(inInfo), Item(inItem), Building(inBuilding), ZhenDao(inZhenDao), FangShi(inFangShi), FangXu(inFangXu), ZhuiZong(inZhuiZong), ZhuiZongRadius(inZhuiZongRadius), Aim(inAim), AimValue(inAimValue), AimRadius(inAimRadius), BoxColor(inBoxColor), BoneColor(inBoneColor), InfoColor(inInfoColor) {}
};
MenuInfo g_MenuInfo;

VOID SetCarbonFiberStyle() {

	ImGuiStyle& style = ImGui::GetStyle();

	// 设置颜色
	ImVec4* colors = style.Colors;
	colors[ImGuiCol_Text] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f); // 文本颜色
	colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f); // 禁用文本颜色
	colors[ImGuiCol_WindowBg] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f); // 窗口背景颜色
	colors[ImGuiCol_Border] = ImVec4(0.90f, 0.90f, 0.90f, 0.65f); // 边框颜色
	colors[ImGuiCol_FrameBg] = ImVec4(0.98f, 0.98f, 0.98f, 1.00f); // 框架背景颜色
	colors[ImGuiCol_TitleBg] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f); // 标题背景颜色
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.98f, 0.98f, 0.98f, 1.00f); // 激活标题背景颜色
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.98f, 0.98f, 0.98f, 0.90f); // 折叠标题背景颜色
	colors[ImGuiCol_Button] = ImVec4(0.95f, 0.95f, 0.95f, 0.60f); // 按钮颜色
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.98f, 0.98f, 0.98f, 1.00f); // 悬停按钮颜色
	colors[ImGuiCol_ButtonActive] = ImVec4(0.90f, 0.90f, 0.90f, 1.00f); // 激活按钮颜色
	colors[ImGuiCol_Header] = ImVec4(0.95f, 0.95f, 0.95f, 0.45f); // 头部颜色
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.98f, 0.98f, 0.98f, 0.80f); // 悬停头部颜色
	colors[ImGuiCol_HeaderActive] = ImVec4(0.90f, 0.90f, 0.90f, 1.00f); // 激活头部颜色
	colors[ImGuiCol_Separator] = ImVec4(0.90f, 0.90f, 0.90f, 0.50f); // 分隔符颜色
	colors[ImGuiCol_SeparatorHovered] = ImVec4(0.95f, 0.95f, 0.95f, 0.78f); // 悬停分隔符颜色
	colors[ImGuiCol_SeparatorActive] = ImVec4(0.98f, 0.98f, 0.98f, 1.00f); // 激活分隔符颜色
	colors[ImGuiCol_ResizeGrip] = ImVec4(0.95f, 0.95f, 0.95f, 0.25f); // 调整手柄颜色
	colors[ImGuiCol_ResizeGripHovered] = ImVec4(1.00f, 1.00f, 1.00f, 0.67f); // 悬停调整手柄颜色
	colors[ImGuiCol_ResizeGripActive] = ImVec4(0.90f, 0.90f, 0.90f, 0.95f); // 激活调整手柄颜色
	colors[ImGuiCol_Tab] = ImVec4(0.98f, 0.98f, 0.98f, 0.80f); // 标签颜色
	colors[ImGuiCol_TabHovered] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f); // 悬停标签颜色
	colors[ImGuiCol_TabActive] = ImVec4(0.95f, 0.95f, 0.95f, 1.00f); // 激活标签颜色
	colors[ImGuiCol_TabUnfocused] = ImVec4(0.98f, 0.98f, 0.98f, 0.50f); // 未聚焦标签颜色
	colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.90f, 0.90f, 0.90f, 0.75f); // 未聚焦激活标签颜色

	// 调整样式属性
	style.WindowRounding = 5.0f; // 窗口圆角
	//style.FrameRounding = 3.0f; // 框架圆角
	//style.GrabRounding = 2.0f; // 拖拽圆角
	style.ScrollbarRounding = 4.0f; // 滚动条圆角
	style.TabRounding = 4.0f; // 标签圆角
	style.WindowBorderSize = 0.0f; // 窗口边框大小
	style.ChildBorderSize = 0.0f; // 子窗口边框大小
	style.PopupBorderSize = 0.0f; // 弹出窗口边框大小
	style.FrameBorderSize = 0.0f; // 框架边框大小
	style.TabBorderSize = 0.0f; // 标签边框大小
	style.WindowTitleAlign = ImVec2(0.5f, 0.5f); // 窗口标题对齐方式
	style.ItemSpacing = ImVec2(8.0f, 4.0f); // 项之间的间距
	style.ItemInnerSpacing = ImVec2(4.0f, 4.0f); // 项内部间距
	style.IndentSpacing = 20.0f; // 缩进距离
	style.ScrollbarSize = 15.0f; // 滚动条大小
	style.ScrollbarRounding = 9.0f; // 滚动条圆角
	style.GrabMinSize = 5.0f; // 调整手柄最小尺寸
	style.WindowTitleAlign = ImVec2(0.5f, 0.5f); // 窗口标题对齐方式


	style.FramePadding = ImVec2(1.0f, 1.0f); // 调整复选框内边距
	style.FrameRounding = 0.5f; // 设置复选框边框圆角半径
	style.GrabRounding = 0.5f; // 设置复选框内勾选标记的圆角半径
	style.WindowMinSize = ImVec2(300, 300);
}
VOID DrawMenu() {
	//菜单样式
	SetCarbonFiberStyle();
	ImGui::SetNextWindowSize(ImVec2(450, 405));
	ImGui::Begin(u8"ATM"); {
		if (ImGui::BeginTabBar("Dominus TabBar")) {
			if (ImGui::BeginTabItem(u8" 透视 ")) {
				if (ImGui::TreeNode(u8"玩家配置")) {
					if (ImGui::TreeNode(u8"功能")) {
						Gui.MyCheckBox3(u8"方框", &g_MenuInfo.Box);
						ImGui::SameLine();
						Gui.MyCheckBox3(u8"骨骼", &g_MenuInfo.Bone);
						ImGui::SameLine();
						Gui.MyCheckBox3(u8"护甲", &g_MenuInfo.Shield);
						ImGui::SameLine();
						Gui.MyCheckBox3(u8"血量", &g_MenuInfo.Hp);
						ImGui::SameLine();
						Gui.MyCheckBox3(u8"详细信息", &g_MenuInfo.Info);
						ImGui::TreePop();
					}
					if (ImGui::TreeNode(u8"颜色")) {
						ImGui::ColorEdit4(u8"方框", (float*)&g_MenuInfo.BoxColor);
						ImGui::ColorEdit4(u8"骨骼", (float*)&g_MenuInfo.BoneColor);
						ImGui::ColorEdit4(u8"详细信息", (float*)&g_MenuInfo.InfoColor);
						ImGui::TreePop();
					}
					ImGui::TreePop();
				}
				if (ImGui::TreeNode(u8"物资配置")) {
					if (ImGui::TreeNode(u8"功能")) {
						Gui.MyCheckBox3(u8"", &g_MenuInfo.Item);
						ImGui::SameLine();
						Gui.MyCheckBox3(u8"堆堆透视", &g_MenuInfo.Building);
						ImGui::TreePop();
					}
					ImGui::TreePop();
				}
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem(u8" 振刀设置 ")) {
				if (ImGui::TreeNode(u8"功能")) {
					Gui.MyCheckBox3(u8"自动振刀", &g_MenuInfo.ZhenDao);
					ImGui::TreePop();
				}
				if (ImGui::TreeNode(u8"调节")) {
					if (ImGui::RadioButton(u8"C振", g_MenuInfo.FangShi == true))
						g_MenuInfo.FangShi = true;
					ImGui::SameLine();
					if (ImGui::RadioButton(u8"跳振", g_MenuInfo.FangShi == false))
						g_MenuInfo.FangShi = false;
					ImGui::TreePop();
				}
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem(u8" 自瞄设置 ")) {
				if (ImGui::TreeNode(u8"功能")) {
					Gui.MyCheckBox3(u8"预判自瞄", &g_MenuInfo.Aim);
					if (g_MenuInfo.Aim) {
						ImGui::SameLine();
						Gui.MyCheckBox3(u8"子弹追踪", &g_MenuInfo.ZhuiZong);
					}
					ImGui::TreePop();
				}
				if (ImGui::TreeNode(u8"调节")) {
					float AimValueMin = 0.5f;
					float AimValueMax = 1.5f;
					Gui.SliderScalarEx1(u8"预判数值", ImGuiDataType_Float, &g_MenuInfo.AimValue, &AimValueMin, &AimValueMax, "%.3f", ImGuiSliderFlags_None);
					float AimRadiusMin = 100.0f;
					float AimRadiusMax = 500.0f;
					Gui.SliderScalarEx1(u8"自瞄范围", ImGuiDataType_Float, &g_MenuInfo.AimRadius, &AimRadiusMin, &AimRadiusMax, "%.3f", ImGuiSliderFlags_None);
					if (g_MenuInfo.Aim) {
						float ZhuiRadiusMin = 2.0f;  // 1
						float ZhuiRadiusMax = 10.0f;  // 10
						Gui.SliderScalarEx1(u8"追踪范围", ImGuiDataType_Float, &g_MenuInfo.ZhuiZongRadius, &ZhuiRadiusMin, &ZhuiRadiusMax, "%.3f", ImGuiSliderFlags_None);
					}
					ImGui::TreePop();
				}
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem(u8" 活化 ")) {
				if (ImGui::TreeNode(u8"功能")) {
					Gui.MyCheckBox3(u8"智能放蓄", &g_MenuInfo.FangXu);
					ImGui::TreePop();
				}
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
	}ImGui::End();
}
uint64_t g_KeyTime;

VOID DrawRun() {
	//取消绘制,直接计算矩阵 屏蔽=原来的
	//Data::GetMatrixData(g_MatrixAddress);
	//初始化按键时间
	if (GetTickCount64() - g_KeyTime <= 500) {
		return;
	}
	//快捷键开关
	if (GetAsyncKeyState(VK_END) & 0x8000) {//END退出
		ExitProcess(-1);
	}
	if (GetAsyncKeyState(VK_SUBTRACT) & 0x8000) {//追踪减小范围
		if (g_MenuInfo.ZhuiZongRadius <= 0.0f) {
			g_MenuInfo.ZhuiZongRadius = 1.0f;//防止低于0
		}
		g_MenuInfo.ZhuiZongRadius = g_MenuInfo.ZhuiZongRadius - 1.0f;
		Beep(1000, 120);
		g_KeyTime = GetTickCount64();
		return;
	}
	if (GetAsyncKeyState(VK_ADD) & 0x8000) {//追踪增加范围
		if (g_MenuInfo.ZhuiZongRadius > 10.0f) {
			g_MenuInfo.ZhuiZongRadius = 10.0f;//防止高于10
		}
		g_MenuInfo.ZhuiZongRadius = g_MenuInfo.ZhuiZongRadius + 1.0f;
		Beep(1000, 120);
		g_KeyTime = GetTickCount64();
		return;
	}

	//比如说你要加振刀的快捷键GetAsyncKeyState(VK_END)  
	if (GetAsyncKeyState(VK_F6) & 0x8000) {//震刀快捷键
		g_MenuInfo.ZhenDao = !g_MenuInfo.ZhenDao;
		Beep(1000, 120);
		g_KeyTime = GetTickCount64();
		return;
	}

	if (GetAsyncKeyState(VK_F7) & 0x8000) {//自瞄快捷键
		g_MenuInfo.Aim = !g_MenuInfo.Aim;
		Beep(1000, 120);
		g_KeyTime = GetTickCount64();
		return;
	}

	if (GetAsyncKeyState(VK_F8) & 0x8000) {//追踪快捷键
		g_MenuInfo.ZhuiZong = !g_MenuInfo.ZhuiZong;
		Beep(1000, 120);
		g_KeyTime = GetTickCount64();
		return;
	}


	if (GetAsyncKeyState(VK_F9) & 0x8000) {//放蓄快捷键
		g_MenuInfo.FangXu = !g_MenuInfo.FangXu;
		Beep(1000, 120);
		g_KeyTime = GetTickCount64();
		return;
	}



	//Beep(1000, 120); 滴滴开启声

	//带着所有绘制  取消屏蔽 上面屏蔽=恢复透视

	DrawMenu();

}

/*
	NOTICE:
		If need change to internal mode, please define "OSIMGUI_INTERNAL" in Preprocessing, and change the project to DLL.
		Surport for DirectX11 and DirectX9 now.
	提示：
		如果需要使用internal版本，请在预处理器中定义 "OSIMGUI_INTERNAL"，并且将项目切换为DLL项目。
		支持DirectX11 & DirectX9。
*/

#ifdef OSIMGUI_INTERNAL

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		// Entry
		Gui.Start(hModule, DrawCallBack,OSImGui::DirectXType::AUTO);
	}
	return TRUE;
}

#endif

#ifndef OSIMGUI_INTERNAL

int main()
{
	try {
		//Gui.NewWindow("WindowName", Vec2(500, 500), DrawCallBack,true );
		Gui.AttachAnotherWindow("Naraka","", DrawRun);
	}
	catch (OSImGui::OSException& e)
	{
		std::cout << e.what() << std::endl;
	}

	system("pause");
	return 0;
}

#endif