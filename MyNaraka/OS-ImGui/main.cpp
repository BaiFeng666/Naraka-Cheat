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
	bool Box;//����
	bool Bone;//����
	bool Hp;//Ѫ��
	bool Shield;//����
	bool Info;//��Ϣ
	bool Item;//��Ʒ
	bool Building;//����
	bool ZhenDao;//��
	bool FangShi;//�񵶷�ʽ
	bool FangXu;//����
	bool ZhuiZong;//׷��
	float ZhuiZongRadius;//׷�ٷ�Χ
	bool Aim;//����
	float AimValue;//��������
	float AimRadius;//���鷶Χ
	ImColor BoxColor;
	ImColor BoneColor;
	ImColor InfoColor;

	//������ֵ
	MenuInfo() : Box(true), Bone(true), Hp(true), Shield(true), Info(true), Item(true), Building(true), ZhenDao(true), FangShi(true), FangXu(false), ZhuiZong(false), ZhuiZongRadius(2.0f), Aim(false), AimValue(0.9f), AimRadius(200.0f), BoxColor(192, 192, 192), BoneColor(0, 255, 0), InfoColor(192, 192, 192) {}
	MenuInfo(bool inBox, bool inBone, bool inHp, bool inShield, bool inInfo, bool inItem, bool inBuilding, bool inZhenDao, bool inFangShi, bool inFangXu, bool inZhuiZong, float inZhuiZongRadius, bool inAim, float inAimValue, float inAimRadius, ImColor inBoxColor, ImColor inBoneColor, ImColor inInfoColor) : Box(inBox), Bone(inBone), Hp(inHp), Shield(inShield), Info(inInfo), Item(inItem), Building(inBuilding), ZhenDao(inZhenDao), FangShi(inFangShi), FangXu(inFangXu), ZhuiZong(inZhuiZong), ZhuiZongRadius(inZhuiZongRadius), Aim(inAim), AimValue(inAimValue), AimRadius(inAimRadius), BoxColor(inBoxColor), BoneColor(inBoneColor), InfoColor(inInfoColor) {}
};
MenuInfo g_MenuInfo;

VOID SetCarbonFiberStyle() {

	ImGuiStyle& style = ImGui::GetStyle();

	// ������ɫ
	ImVec4* colors = style.Colors;
	colors[ImGuiCol_Text] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f); // �ı���ɫ
	colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f); // �����ı���ɫ
	colors[ImGuiCol_WindowBg] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f); // ���ڱ�����ɫ
	colors[ImGuiCol_Border] = ImVec4(0.90f, 0.90f, 0.90f, 0.65f); // �߿���ɫ
	colors[ImGuiCol_FrameBg] = ImVec4(0.98f, 0.98f, 0.98f, 1.00f); // ��ܱ�����ɫ
	colors[ImGuiCol_TitleBg] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f); // ���ⱳ����ɫ
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.98f, 0.98f, 0.98f, 1.00f); // ������ⱳ����ɫ
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.98f, 0.98f, 0.98f, 0.90f); // �۵����ⱳ����ɫ
	colors[ImGuiCol_Button] = ImVec4(0.95f, 0.95f, 0.95f, 0.60f); // ��ť��ɫ
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.98f, 0.98f, 0.98f, 1.00f); // ��ͣ��ť��ɫ
	colors[ImGuiCol_ButtonActive] = ImVec4(0.90f, 0.90f, 0.90f, 1.00f); // ���ť��ɫ
	colors[ImGuiCol_Header] = ImVec4(0.95f, 0.95f, 0.95f, 0.45f); // ͷ����ɫ
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.98f, 0.98f, 0.98f, 0.80f); // ��ͣͷ����ɫ
	colors[ImGuiCol_HeaderActive] = ImVec4(0.90f, 0.90f, 0.90f, 1.00f); // ����ͷ����ɫ
	colors[ImGuiCol_Separator] = ImVec4(0.90f, 0.90f, 0.90f, 0.50f); // �ָ�����ɫ
	colors[ImGuiCol_SeparatorHovered] = ImVec4(0.95f, 0.95f, 0.95f, 0.78f); // ��ͣ�ָ�����ɫ
	colors[ImGuiCol_SeparatorActive] = ImVec4(0.98f, 0.98f, 0.98f, 1.00f); // ����ָ�����ɫ
	colors[ImGuiCol_ResizeGrip] = ImVec4(0.95f, 0.95f, 0.95f, 0.25f); // �����ֱ���ɫ
	colors[ImGuiCol_ResizeGripHovered] = ImVec4(1.00f, 1.00f, 1.00f, 0.67f); // ��ͣ�����ֱ���ɫ
	colors[ImGuiCol_ResizeGripActive] = ImVec4(0.90f, 0.90f, 0.90f, 0.95f); // ��������ֱ���ɫ
	colors[ImGuiCol_Tab] = ImVec4(0.98f, 0.98f, 0.98f, 0.80f); // ��ǩ��ɫ
	colors[ImGuiCol_TabHovered] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f); // ��ͣ��ǩ��ɫ
	colors[ImGuiCol_TabActive] = ImVec4(0.95f, 0.95f, 0.95f, 1.00f); // �����ǩ��ɫ
	colors[ImGuiCol_TabUnfocused] = ImVec4(0.98f, 0.98f, 0.98f, 0.50f); // δ�۽���ǩ��ɫ
	colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.90f, 0.90f, 0.90f, 0.75f); // δ�۽������ǩ��ɫ

	// ������ʽ����
	style.WindowRounding = 5.0f; // ����Բ��
	//style.FrameRounding = 3.0f; // ���Բ��
	//style.GrabRounding = 2.0f; // ��קԲ��
	style.ScrollbarRounding = 4.0f; // ������Բ��
	style.TabRounding = 4.0f; // ��ǩԲ��
	style.WindowBorderSize = 0.0f; // ���ڱ߿��С
	style.ChildBorderSize = 0.0f; // �Ӵ��ڱ߿��С
	style.PopupBorderSize = 0.0f; // �������ڱ߿��С
	style.FrameBorderSize = 0.0f; // ��ܱ߿��С
	style.TabBorderSize = 0.0f; // ��ǩ�߿��С
	style.WindowTitleAlign = ImVec2(0.5f, 0.5f); // ���ڱ�����뷽ʽ
	style.ItemSpacing = ImVec2(8.0f, 4.0f); // ��֮��ļ��
	style.ItemInnerSpacing = ImVec2(4.0f, 4.0f); // ���ڲ����
	style.IndentSpacing = 20.0f; // ��������
	style.ScrollbarSize = 15.0f; // ��������С
	style.ScrollbarRounding = 9.0f; // ������Բ��
	style.GrabMinSize = 5.0f; // �����ֱ���С�ߴ�
	style.WindowTitleAlign = ImVec2(0.5f, 0.5f); // ���ڱ�����뷽ʽ


	style.FramePadding = ImVec2(1.0f, 1.0f); // ������ѡ���ڱ߾�
	style.FrameRounding = 0.5f; // ���ø�ѡ��߿�Բ�ǰ뾶
	style.GrabRounding = 0.5f; // ���ø�ѡ���ڹ�ѡ��ǵ�Բ�ǰ뾶
	style.WindowMinSize = ImVec2(300, 300);
}
VOID DrawMenu() {
	//�˵���ʽ
	SetCarbonFiberStyle();
	ImGui::SetNextWindowSize(ImVec2(450, 405));
	ImGui::Begin(u8"ATM"); {
		if (ImGui::BeginTabBar("Dominus TabBar")) {
			if (ImGui::BeginTabItem(u8" ͸�� ")) {
				if (ImGui::TreeNode(u8"�������")) {
					if (ImGui::TreeNode(u8"����")) {
						Gui.MyCheckBox3(u8"����", &g_MenuInfo.Box);
						ImGui::SameLine();
						Gui.MyCheckBox3(u8"����", &g_MenuInfo.Bone);
						ImGui::SameLine();
						Gui.MyCheckBox3(u8"����", &g_MenuInfo.Shield);
						ImGui::SameLine();
						Gui.MyCheckBox3(u8"Ѫ��", &g_MenuInfo.Hp);
						ImGui::SameLine();
						Gui.MyCheckBox3(u8"��ϸ��Ϣ", &g_MenuInfo.Info);
						ImGui::TreePop();
					}
					if (ImGui::TreeNode(u8"��ɫ")) {
						ImGui::ColorEdit4(u8"����", (float*)&g_MenuInfo.BoxColor);
						ImGui::ColorEdit4(u8"����", (float*)&g_MenuInfo.BoneColor);
						ImGui::ColorEdit4(u8"��ϸ��Ϣ", (float*)&g_MenuInfo.InfoColor);
						ImGui::TreePop();
					}
					ImGui::TreePop();
				}
				if (ImGui::TreeNode(u8"��������")) {
					if (ImGui::TreeNode(u8"����")) {
						Gui.MyCheckBox3(u8"", &g_MenuInfo.Item);
						ImGui::SameLine();
						Gui.MyCheckBox3(u8"�Ѷ�͸��", &g_MenuInfo.Building);
						ImGui::TreePop();
					}
					ImGui::TreePop();
				}
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem(u8" ������ ")) {
				if (ImGui::TreeNode(u8"����")) {
					Gui.MyCheckBox3(u8"�Զ���", &g_MenuInfo.ZhenDao);
					ImGui::TreePop();
				}
				if (ImGui::TreeNode(u8"����")) {
					if (ImGui::RadioButton(u8"C��", g_MenuInfo.FangShi == true))
						g_MenuInfo.FangShi = true;
					ImGui::SameLine();
					if (ImGui::RadioButton(u8"����", g_MenuInfo.FangShi == false))
						g_MenuInfo.FangShi = false;
					ImGui::TreePop();
				}
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem(u8" �������� ")) {
				if (ImGui::TreeNode(u8"����")) {
					Gui.MyCheckBox3(u8"Ԥ������", &g_MenuInfo.Aim);
					if (g_MenuInfo.Aim) {
						ImGui::SameLine();
						Gui.MyCheckBox3(u8"�ӵ�׷��", &g_MenuInfo.ZhuiZong);
					}
					ImGui::TreePop();
				}
				if (ImGui::TreeNode(u8"����")) {
					float AimValueMin = 0.5f;
					float AimValueMax = 1.5f;
					Gui.SliderScalarEx1(u8"Ԥ����ֵ", ImGuiDataType_Float, &g_MenuInfo.AimValue, &AimValueMin, &AimValueMax, "%.3f", ImGuiSliderFlags_None);
					float AimRadiusMin = 100.0f;
					float AimRadiusMax = 500.0f;
					Gui.SliderScalarEx1(u8"���鷶Χ", ImGuiDataType_Float, &g_MenuInfo.AimRadius, &AimRadiusMin, &AimRadiusMax, "%.3f", ImGuiSliderFlags_None);
					if (g_MenuInfo.Aim) {
						float ZhuiRadiusMin = 2.0f;  // 1
						float ZhuiRadiusMax = 10.0f;  // 10
						Gui.SliderScalarEx1(u8"׷�ٷ�Χ", ImGuiDataType_Float, &g_MenuInfo.ZhuiZongRadius, &ZhuiRadiusMin, &ZhuiRadiusMax, "%.3f", ImGuiSliderFlags_None);
					}
					ImGui::TreePop();
				}
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem(u8" � ")) {
				if (ImGui::TreeNode(u8"����")) {
					Gui.MyCheckBox3(u8"���ܷ���", &g_MenuInfo.FangXu);
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
	//ȡ������,ֱ�Ӽ������ ����=ԭ����
	//Data::GetMatrixData(g_MatrixAddress);
	//��ʼ������ʱ��
	if (GetTickCount64() - g_KeyTime <= 500) {
		return;
	}
	//��ݼ�����
	if (GetAsyncKeyState(VK_END) & 0x8000) {//END�˳�
		ExitProcess(-1);
	}
	if (GetAsyncKeyState(VK_SUBTRACT) & 0x8000) {//׷�ټ�С��Χ
		if (g_MenuInfo.ZhuiZongRadius <= 0.0f) {
			g_MenuInfo.ZhuiZongRadius = 1.0f;//��ֹ����0
		}
		g_MenuInfo.ZhuiZongRadius = g_MenuInfo.ZhuiZongRadius - 1.0f;
		Beep(1000, 120);
		g_KeyTime = GetTickCount64();
		return;
	}
	if (GetAsyncKeyState(VK_ADD) & 0x8000) {//׷�����ӷ�Χ
		if (g_MenuInfo.ZhuiZongRadius > 10.0f) {
			g_MenuInfo.ZhuiZongRadius = 10.0f;//��ֹ����10
		}
		g_MenuInfo.ZhuiZongRadius = g_MenuInfo.ZhuiZongRadius + 1.0f;
		Beep(1000, 120);
		g_KeyTime = GetTickCount64();
		return;
	}

	//����˵��Ҫ���񵶵Ŀ�ݼ�GetAsyncKeyState(VK_END)  
	if (GetAsyncKeyState(VK_F6) & 0x8000) {//�𵶿�ݼ�
		g_MenuInfo.ZhenDao = !g_MenuInfo.ZhenDao;
		Beep(1000, 120);
		g_KeyTime = GetTickCount64();
		return;
	}

	if (GetAsyncKeyState(VK_F7) & 0x8000) {//�����ݼ�
		g_MenuInfo.Aim = !g_MenuInfo.Aim;
		Beep(1000, 120);
		g_KeyTime = GetTickCount64();
		return;
	}

	if (GetAsyncKeyState(VK_F8) & 0x8000) {//׷�ٿ�ݼ�
		g_MenuInfo.ZhuiZong = !g_MenuInfo.ZhuiZong;
		Beep(1000, 120);
		g_KeyTime = GetTickCount64();
		return;
	}


	if (GetAsyncKeyState(VK_F9) & 0x8000) {//�����ݼ�
		g_MenuInfo.FangXu = !g_MenuInfo.FangXu;
		Beep(1000, 120);
		g_KeyTime = GetTickCount64();
		return;
	}



	//Beep(1000, 120); �εο�����

	//�������л���  ȡ������ ��������=�ָ�͸��

	DrawMenu();

}

/*
	NOTICE:
		If need change to internal mode, please define "OSIMGUI_INTERNAL" in Preprocessing, and change the project to DLL.
		Surport for DirectX11 and DirectX9 now.
	��ʾ��
		�����Ҫʹ��internal�汾������Ԥ�������ж��� "OSIMGUI_INTERNAL"�����ҽ���Ŀ�л�ΪDLL��Ŀ��
		֧��DirectX11 & DirectX9��
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