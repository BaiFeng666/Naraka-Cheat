#include "Naraka.h"
#include "BFDrvUtils.h"

BFDrv Drv;

BYTE OBytes[1] = { 0 };
BYTE newBytes[1] = { 0 };
namespace keyMsg {

	void KeyDown(char key)
	{
		Drv.B_KeyCtl(key, KeyStatus::DOWN);
	}

	void KeyUp(char key)
	{
		Drv.B_KeyCtl(key, KeyStatus::UP);
	}

	void RightDown()
	{
		Drv.B_MouseCtl(MouseKey::RIGHT, MouseStatus::DOWN);
	}

	void RightUp()
	{
		Drv.B_MouseCtl(MouseKey::RIGHT, MouseStatus::UP);
	}

	void LeftDown()
	{
		Drv.B_MouseCtl(MouseKey::LEFT, MouseStatus::DOWN);
	}

	void LeftUp()
	{
		Drv.B_MouseCtl(MouseKey::LEFT, MouseStatus::UP);
	}
}

namespace Memory {

	// 获取模块地址
	uint64_t GetModuleAddress(const char* moduleName)
	{
		if (g_Window.Pid <= 0)
		{
			return 0;
		}
		return Drv.B_GetMoudleBaseAddress(moduleName, NULL);
	}

	// 读取字节
	BOOL ReadByte(ULONG64 address, size_t size, void* buffer)
	{
		return Drv.B_ReadMemory(address, buffer, size, RWMode::MmCopy);
	}

	// 写入字节
	BOOL WriteByte(uint64_t address, uint32_t size, PVOID buffer)
	{
		return Drv.B_WriteMemory(address, buffer, size, RWMode::MmCopy);
	}

	// 读取内存
	template<typename T>
	T ReadMemory(uint64_t address) 
	{
		T buff{};
		Drv.B_ReadMemory(address, &buff, sizeof(T), RWMode::MmCopy);
		return buff;
	}

	// 写入内存
	template<typename T>
	BOOL WriteMemory(uint64_t address, T buffer) {
		if (address <= 0) {
			return false;
		}
		return Drv.B_WriteMem(address, &buffer, sizeof(buffer), RWMode::Ke);
	}

	// 分配内存
	uint64_t AllocMemory(uint32_t  size) {
		if (g_Window.Pid <= 0) {
			return 0;
		}
		return Drv.B_AllocMemory(size);
	}
}

namespace Kernel {

	HANDLE m_hTimerQueue = nullptr;
	HANDLE m_hTimerQueueTimer = nullptr;
	void _SafeCreateThread(WAITORTIMERCALLBACK callBack) {
		if (!CreateTimerQueueTimer(&m_hTimerQueueTimer, nullptr, callBack, nullptr, 0, 0, WT_EXECUTEDEFAULT)) {
			m_hTimerQueueTimer = nullptr;
		}
	}
}

namespace Data {

	//取矩阵地址
	uint64_t GetMatrixAddress(uint64_t WorldLevel2) {
		uint64_t Buffer = Memory::ReadMemory<uint64_t>(WorldLevel2 + SDK::世矩三级);
		Buffer = Memory::ReadMemory<uint64_t>(Buffer + SDK::世矩四级);
		Buffer = Memory::ReadMemory<uint64_t>(Buffer + SDK::世矩五级);
		return Buffer + SDK::世矩六级;
	}

	//取矩阵数据
	VOID GetMatrixData(uint64_t MatrixAddr) {
		for (int i = 0; i < 4; ++i) {
			for (int j = 0; j < 4; ++j) {
				g_MatrixData[i][j] = Memory::ReadMemory<FLOAT>(MatrixAddr + static_cast<size_t>(i) * 16 + static_cast<size_t>(j) * 4);
			}
		}
	}

	//取遍历地址
	uint64_t GetCountArray(uint64_t WorldAddress) {
		uint64_t Buffer = Memory::ReadMemory<uint64_t>(WorldAddress + SDK::遍历一级);
		Buffer = Memory::ReadMemory<uint64_t>(Buffer + SDK::遍历二级);
		Buffer = Memory::ReadMemory<uint64_t>(Buffer + SDK::遍历三级);
		return Buffer;
	}

	//取队伍编号
	uint32_t GetPlayerTeam(uint64_t PlayerAddress) {
		uint64_t Buffer = Memory::ReadMemory<uint64_t>(PlayerAddress + SDK::对象一级);
		uint32_t ReturnData = Memory::ReadMemory<uint32_t>(Buffer + SDK::队伍偏移);
		return ReturnData;
	}

	//取对象指针
	uint64_t GetPlayerEntity(uint64_t PlayerAddress) {
		uint64_t Buffer = Memory::ReadMemory<uint64_t>(PlayerAddress + SDK::指针一级);
		Buffer = Memory::ReadMemory<uint64_t>(Buffer + SDK::指针二级);
		Buffer = Memory::ReadMemory<uint64_t>(Buffer + SDK::指针三级);
		return Buffer;
	}

	//取对象坐标
	Vector3 GetPlayerPosition(uint64_t PlayerEntity) {
		Vector3 Pos;
		uint64_t Buffer = Memory::ReadMemory<uint64_t>(PlayerEntity + SDK::指针四级);
		Buffer = Memory::ReadMemory<uint64_t>(Buffer + SDK::指针五级);
		Buffer = Memory::ReadMemory<uint64_t>(Buffer + SDK::指针六级);
		if (Buffer != 0) {
			Pos.x = Memory::ReadMemory<FLOAT>(Buffer + SDK::坐标X轴);
			Pos.y = Memory::ReadMemory<FLOAT>(Buffer + SDK::坐标Z轴);
			Pos.z = Memory::ReadMemory<FLOAT>(Buffer + SDK::坐标Y轴);
		}
		return Pos;
	}

	//取解密编号
	PropertyData GetPropertyDataID(uint64_t PlayerAddress) {
		PropertyData Prop;
		uint64_t Data = Memory::ReadMemory<uint64_t>(PlayerAddress + 0x10);
		Data = Memory::ReadMemory<uint64_t>(Data + 0x10);
		Data = Memory::ReadMemory<uint64_t>(Data + 0x18);
		uint32_t DictCount = Memory::ReadMemory<uint32_t>(Data + 0x18);
		uint32_t Index = 0;
		for (uint32_t i = 0; i < DictCount; i++) {
			uint64_t ID1 = Data + 0x28 + static_cast<uint64_t>(i) * 0x10;
			uint64_t ID2 = ID1 + 4;
			uint32_t DataID = Memory::ReadMemory<uint32_t>(ID1);
			uint32_t TempID = Memory::ReadMemory<uint32_t>(ID2);
			if (DataID == 11001) { Prop.CurHp = TempID; Index += 1; }
			if (DataID == 10001) { Prop.HpMax = TempID; Index += 1; }
			if (DataID == 11024) { Prop.CurShield = TempID; Index += 1; }
			if (DataID == 10107) { Prop.ShieldMax = TempID; Index += 1; }
			if (Index >= 4) { break; }
		}
		return Prop;
	}

	//取人物解密
	PropertyData GetPropertyData(uint64_t PlayerAddress) {
		PropertyData Prop;
		uint64_t Rcx = Memory::ReadMemory<uint64_t>(PlayerAddress + 0xA8);
		PropertyData ID = GetPropertyDataID(Rcx);
		uint64_t Data = Memory::ReadMemory<uint64_t>(Rcx + 0x10);
		Data = Memory::ReadMemory<uint64_t>(Data + 0x28);
		Data = Memory::ReadMemory<uint64_t>(Data + 0x10);
		if (Data) {
			Prop.CurHp = Memory::ReadMemory<uint32_t>(Data + 0x20 + ID.CurHp * 4ull);
			Prop.HpMax = Memory::ReadMemory<uint32_t>(Data + 0x20 + ID.HpMax * 4ull);
			Prop.CurShield = Memory::ReadMemory<uint32_t>(Data + 0x20 + ID.CurShield * 4ull);
			Prop.ShieldMax = Memory::ReadMemory<uint32_t>(Data + 0x20 + ID.ShieldMax * 4ull);
		}
		return Prop;
	}

	//取对象坐标
	Vector3 GetPosition(uint64_t PlayerEntity) {
		Vector3 Pos;
		uint64_t Buffer = Memory::ReadMemory<uint64_t>(Memory::ReadMemory<uint64_t>(Memory::ReadMemory<uint64_t>(PlayerEntity + SDK::指针四级) + SDK::指针五级) + SDK::指针六级);
		//if (Buffer != 0) {
		//	Memory::ReadByte(Buffer + SDK::坐标X轴, sizeof(Pos), &Pos);
		//}
		Pos = Memory::ReadMemory<Vector3>(Buffer + SDK::坐标X轴);

		return Pos;
	}

	//取对象名称
	wstring GetPlayerName(uint64_t PlayerAddress) {
		wchar_t ReturnData[64] = { NULL };
		uint64_t Buffer = Memory::ReadMemory<uint64_t>(PlayerAddress + SDK::对象一级);
		Buffer = Memory::ReadMemory<uint64_t>(Buffer + SDK::名称一级);
		Memory::ReadByte(Buffer + SDK::名称二级, sizeof(ReturnData), ReturnData);
		return wstring(ReturnData);
	}

	//取英雄编号
	uint32_t GetHeroId(uint64_t PlayerAddress) {
		uint64_t Buffer = Memory::ReadMemory<uint64_t>(PlayerAddress + SDK::对象一级);
		uint32_t ReturnData = Memory::ReadMemory<uint32_t>(Buffer + SDK::职业偏移);
		return ReturnData;
	}

	//取对象英雄
	wstring GetPlayerHeroName(uint64_t PlayerAddress) {
		wstring HeroName;
		uint32_t HeroId = GetHeroId(PlayerAddress);
		switch (HeroId) {
		case 1000001:
			HeroName = L"胡桃";
			break;
		case 1000003:
			HeroName = L"宁红叶";
			break;
		case 1000004:
			HeroName = L"迦南";
			break;
		case 1000005:
			HeroName = L"特木尔";
			break;
		case 1000006:
			HeroName = L"季沧海";
			break;
		case 1000007:	case 1000008:
			HeroName = L"天海";
			break;
		case 1000009:
			HeroName = L"妖刀姬";
			break;
		case 1000010:
			HeroName = L"崔三娘";
			break;
		case 1000011:	case 1000012:
			HeroName = L"岳山";
			break;
		case 1000013:
			HeroName = L"无尘";
			break;
		case 1000015:
			HeroName = L"顾清寒";
			break;
		case 1000016:
			HeroName = L"武田信忠";
			break;
		case 1000017:
			HeroName = L"殷紫萍";
			break;
		case 1000018:	case 1000019:
			HeroName = L"沈妙";
			break;
		case 1000020:
			HeroName = L"胡为";
			break;
		case 1000021:
			HeroName = L"季莹莹";
			break;
		case 1000022:
			HeroName = L"玉玲珑";
			break;
		case 1000023:
			HeroName = L"哈迪";
			break;
		case 1000024:
			HeroName = L"魏轻";
			break;
		default:
			HeroName = L"未知: " + std::to_wstring(HeroId);
			break;
		}
		return HeroName;
	}

	//判断是否人机
	BOOL GetRoBot(uint64_t PlayerAddress) {
		uint64_t Buffer = Memory::ReadMemory<uint64_t>(PlayerAddress + SDK::对象一级);
		return Memory::ReadMemory<uint32_t>(Buffer + SDK::人机偏移) > 0;
	}

	//取物品名称
	DataInfo GetItemName(uint32_t ItemID) {
		DataInfo m_ItemInfo;
		switch (ItemID) {
			//case 3005001:
			//	m_ItemInfo.Name = u8"白甲";
			//	m_ItemInfo.Color = IM_COL32(173, 173, 173, 255);//白品质
			//	break;
		case 3005002:
			m_ItemInfo.Name = u8"蓝甲";
			m_ItemInfo.Color = IM_COL32(103, 142, 255, 255);//蓝品质
			break;
		case 3005003:
			m_ItemInfo.Name = u8"紫甲";
			m_ItemInfo.Color = IM_COL32(192, 122, 248, 255);//紫品质
			break;
		case 3005004:
			m_ItemInfo.Name = u8"金甲";
			m_ItemInfo.Color = IM_COL32(246, 192, 66, 255);//金品质
			break;
		case 3005005:
			m_ItemInfo.Name = u8"红甲";
			m_ItemInfo.Color = IM_COL32(255, 68, 68, 255);//红品质
			break;
		case 3010005:
			m_ItemInfo.Name = u8"萤火虫";
			m_ItemInfo.Color = IM_COL32(103, 142, 255, 255);
			break;
		case 3010007:
			m_ItemInfo.Name = u8"蒲公英";
			m_ItemInfo.Color = IM_COL32(103, 142, 255, 255);
			break;
		case 3010050:
			m_ItemInfo.Name = u8"沙梨圣果";
			m_ItemInfo.Color = IM_COL32(103, 142, 255, 255);
			break;
		case 3010008:
			m_ItemInfo.Name = u8"刺梨";
			m_ItemInfo.Color = IM_COL32(103, 142, 255, 255);
			break;
		case 3010009:
			m_ItemInfo.Name = u8"沙叻";
			m_ItemInfo.Color = IM_COL32(103, 142, 255, 255);
			break;
			//case 3002200:
			//	m_ItemInfo.Name = u8"横刀";
			//	m_ItemInfo.Color = IM_COL32(173, 173, 173, 255);
			//	break;
		case 3002210:
			m_ItemInfo.Name = u8"横刀";
			m_ItemInfo.Color = IM_COL32(103, 142, 255, 255);
			break;
		case 3002220:
			m_ItemInfo.Name = u8"横刀";
			m_ItemInfo.Color = IM_COL32(192, 122, 248, 255);
			break;
		case 3002230:
			m_ItemInfo.Name = u8"横刀";
			m_ItemInfo.Color = IM_COL32(246, 192, 66, 255);
			break;
			//case 3000001:
			//	m_ItemInfo.Name = u8"长剑";
			//	m_ItemInfo.Color = IM_COL32(173, 173, 173, 255);
			//	break;
		case 3000011:
			m_ItemInfo.Name = u8"长剑";
			m_ItemInfo.Color = IM_COL32(103, 142, 255, 255);
			break;
		case 3000021:
			m_ItemInfo.Name = u8"长剑";
			m_ItemInfo.Color = IM_COL32(192, 122, 248, 255);
			break;
		case 3000031:
			m_ItemInfo.Name = u8"长剑";
			m_ItemInfo.Color = IM_COL32(246, 192, 66, 255);
			break;
			//case 3000300:
			//	m_ItemInfo.Name = u8"太刀";
			//	m_ItemInfo.Color = IM_COL32(173, 173, 173, 255);
			//	break;
		case 3000310:
			m_ItemInfo.Name = u8"太刀";
			m_ItemInfo.Color = IM_COL32(103, 142, 255, 255);
			break;
		case 3000320:
			m_ItemInfo.Name = u8"太刀";
			m_ItemInfo.Color = IM_COL32(192, 122, 248, 255);
			break;
		case 3000330:
			m_ItemInfo.Name = u8"太刀";
			m_ItemInfo.Color = IM_COL32(246, 192, 66, 255);
			break;
			//case 3000400:
			//	m_ItemInfo.Name = u8"阔刀";
			//	m_ItemInfo.Color = IM_COL32(173, 173, 173, 255);
			//	break;
		case 3000410:
			m_ItemInfo.Name = u8"阔刀";
			m_ItemInfo.Color = IM_COL32(103, 142, 255, 255);
			break;
		case 3000420:
			m_ItemInfo.Name = u8"阔刀";
			m_ItemInfo.Color = IM_COL32(192, 122, 248, 255);
			break;
		case 3000430:
			m_ItemInfo.Name = u8"阔刀";
			m_ItemInfo.Color = IM_COL32(246, 192, 66, 255);
			break;
			//case 3001700:
			//	m_ItemInfo.Name = u8"斩马刀";
			//	m_ItemInfo.Color = IM_COL32(173, 173, 173, 255);
			//	break;
		case 3001710:
			m_ItemInfo.Name = u8"斩马刀";
			m_ItemInfo.Color = IM_COL32(103, 142, 255, 255);
			break;
		case 3001720:
			m_ItemInfo.Name = u8"斩马刀";
			m_ItemInfo.Color = IM_COL32(192, 122, 248, 255);
			break;
		case 3001730:
			m_ItemInfo.Name = u8"斩马刀";
			m_ItemInfo.Color = IM_COL32(246, 192, 66, 255);
			break;
			//case 3000200:
			//	m_ItemInfo.Name = u8"枪";
			//	m_ItemInfo.Color = IM_COL32(173, 173, 173, 255);
			//	break;
		case 3000210:
			m_ItemInfo.Name = u8"枪";
			m_ItemInfo.Color = IM_COL32(103, 142, 255, 255);
			break;
		case 3000220:
			m_ItemInfo.Name = u8"枪";
			m_ItemInfo.Color = IM_COL32(192, 122, 248, 255);
			break;
		case 3000230:
			m_ItemInfo.Name = u8"枪";
			m_ItemInfo.Color = IM_COL32(246, 192, 66, 255);
			break;
			//case 3001800:
			//	m_ItemInfo.Name = u8"棍";
			//	m_ItemInfo.Color = IM_COL32(173, 173, 173, 255);
			//	break;
		case 3001810:
			m_ItemInfo.Name = u8"棍";
			m_ItemInfo.Color = IM_COL32(103, 142, 255, 255);
			break;
		case 3001820:
			m_ItemInfo.Name = u8"棍";
			m_ItemInfo.Color = IM_COL32(192, 122, 248, 255);
			break;
		case 3001830:
			m_ItemInfo.Name = u8"棍";
			m_ItemInfo.Color = IM_COL32(246, 192, 66, 255);
			break;
			//case 3000100:
			//	m_ItemInfo.Name = u8"匕首";
			//	m_ItemInfo.Color = IM_COL32(173, 173, 173, 255);
			//	break;
		case 3000110:
			m_ItemInfo.Name = u8"匕首";
			m_ItemInfo.Color = IM_COL32(103, 142, 255, 255);
			break;
		case 3000120:
			m_ItemInfo.Name = u8"匕首";
			m_ItemInfo.Color = IM_COL32(192, 122, 248, 255);
			break;
		case 3000130:
			m_ItemInfo.Name = u8"匕首";
			m_ItemInfo.Color = IM_COL32(246, 192, 66, 255);
			break;
			//case 3002000:
			//	m_ItemInfo.Name = u8"扇";
			//	m_ItemInfo.Color = IM_COL32(173, 173, 173, 255);
			//	break;
		case 3002010:
			m_ItemInfo.Name = u8"扇";
			m_ItemInfo.Color = IM_COL32(103, 142, 255, 255);
			break;
		case 3002020:
			m_ItemInfo.Name = u8"扇";
			m_ItemInfo.Color = IM_COL32(192, 122, 248, 255);
			break;
		case 3002030:
			m_ItemInfo.Name = u8"扇";
			m_ItemInfo.Color = IM_COL32(246, 192, 66, 255);
			break;
			//case 3001500:
			//	m_ItemInfo.Name = u8"双节棍";
			//	m_ItemInfo.Color = IM_COL32(173, 173, 173, 255);
			//	break;
		case 3001510:
			m_ItemInfo.Name = u8"双节棍";
			m_ItemInfo.Color = IM_COL32(103, 142, 255, 255);
			break;
		case 3001520:
			m_ItemInfo.Name = u8"双节棍";
			m_ItemInfo.Color = IM_COL32(192, 122, 248, 255);
			break;
		case 3001530:
			m_ItemInfo.Name = u8"双节棍";
			m_ItemInfo.Color = IM_COL32(246, 192, 66, 255);
			break;
			//case 3001600:
			//	m_ItemInfo.Name = u8"双刀";
			//	m_ItemInfo.Color = IM_COL32(173, 173, 173, 255);
			//	break;
		case 3001610:
			m_ItemInfo.Name = u8"双刀";
			m_ItemInfo.Color = IM_COL32(103, 142, 255, 255);
			break;
		case 3001620:
			m_ItemInfo.Name = u8"双刀";
			m_ItemInfo.Color = IM_COL32(192, 122, 248, 255);
			break;
		case 3001630:
			m_ItemInfo.Name = u8"双刀";
			m_ItemInfo.Color = IM_COL32(246, 192, 66, 255);
			break;
			//case 3001900:
			//	m_ItemInfo.Name = u8"双戟";
			//	m_ItemInfo.Color = IM_COL32(173, 173, 173, 255);
			//	break;
		case 3001910:
			m_ItemInfo.Name = u8"双戟";
			m_ItemInfo.Color = IM_COL32(103, 142, 255, 255);
			break;
		case 3001920:
			m_ItemInfo.Name = u8"双戟";
			m_ItemInfo.Color = IM_COL32(192, 122, 248, 255);
			break;
		case 3001930:
			m_ItemInfo.Name = u8"双戟";
			m_ItemInfo.Color = IM_COL32(246, 192, 66, 255);
			break;
		case 3001300:
			m_ItemInfo.Name = u8"万刃轮";
			m_ItemInfo.Color = IM_COL32(173, 173, 173, 255);
			break;
		case 3001310:
			m_ItemInfo.Name = u8"万刃轮";
			m_ItemInfo.Color = IM_COL32(103, 142, 255, 255);
			break;
		case 3001320:
			m_ItemInfo.Name = u8"万刃轮";
			m_ItemInfo.Color = IM_COL32(192, 122, 248, 255);
			break;
		case 3001330:
			m_ItemInfo.Name = u8"万刃轮";
			m_ItemInfo.Color = IM_COL32(246, 192, 66, 255);
			break;
			//case 3000700:
			//	m_ItemInfo.Name = u8"连弩";
			//	m_ItemInfo.Color = IM_COL32(173, 173, 173, 255);
			//	break;
		case 3000710:
			m_ItemInfo.Name = u8"连弩";
			m_ItemInfo.Color = IM_COL32(103, 142, 255, 255);
			break;
		case 3000720:
			m_ItemInfo.Name = u8"连弩";
			m_ItemInfo.Color = IM_COL32(192, 122, 248, 255);
			break;
		case 3000730:
			m_ItemInfo.Name = u8"连弩";
			m_ItemInfo.Color = IM_COL32(246, 192, 66, 255);
			break;
			//case 3001200:
			//	m_ItemInfo.Name = u8"鸟铳";
			//	m_ItemInfo.Color = IM_COL32(173, 173, 173, 255);
			//	break;
		case 3001210:
			m_ItemInfo.Name = u8"鸟铳";
			m_ItemInfo.Color = IM_COL32(103, 142, 255, 255);
			break;
		case 3001220:
			m_ItemInfo.Name = u8"鸟铳";
			m_ItemInfo.Color = IM_COL32(192, 122, 248, 255);
			break;
		case 3001230:
			m_ItemInfo.Name = u8"鸟铳";
			m_ItemInfo.Color = IM_COL32(246, 192, 66, 255);
			break;
			//case 3000800:
			//	m_ItemInfo.Name = u8"火炮";
			//	m_ItemInfo.Color = IM_COL32(173, 173, 173, 255);
			//	break;
		case 3000810:
			m_ItemInfo.Name = u8"火炮";
			m_ItemInfo.Color = IM_COL32(103, 142, 255, 255);
			break;
		case 3000820:
			m_ItemInfo.Name = u8"火炮";
			m_ItemInfo.Color = IM_COL32(192, 122, 248, 255);
			break;
		case 3000830:
			m_ItemInfo.Name = u8"火炮";
			m_ItemInfo.Color = IM_COL32(246, 192, 66, 255);
			break;
			//case 3001100:
			//	m_ItemInfo.Name = u8"五眼铳";
			//	m_ItemInfo.Color = IM_COL32(173, 173, 173, 255);
			//	break;
		case 3001110:
			m_ItemInfo.Name = u8"五眼铳";
			m_ItemInfo.Color = IM_COL32(103, 142, 255, 255);
			break;
		case 3001120:
			m_ItemInfo.Name = u8"五眼铳";
			m_ItemInfo.Color = IM_COL32(192, 122, 248, 255);
			break;
		case 3001130:
			m_ItemInfo.Name = u8"五眼铳";
			m_ItemInfo.Color = IM_COL32(246, 192, 66, 255);
			break;
			//case 3000501:
			//	m_ItemInfo.Name = u8"弓箭";
			//	m_ItemInfo.Color = IM_COL32(173, 173, 173, 255);
			//	break;
		case 3000511:
			m_ItemInfo.Name = u8"弓箭";
			m_ItemInfo.Color = IM_COL32(103, 142, 255, 255);
			break;
		case 3000521:
			m_ItemInfo.Name = u8"弓箭";
			m_ItemInfo.Color = IM_COL32(192, 122, 248, 255);
			break;
		case 3000531:
			m_ItemInfo.Name = u8"弓箭";
			m_ItemInfo.Color = IM_COL32(246, 192, 66, 255);
			break;
		case 3001000:
			m_ItemInfo.Name = u8"一窝蜂";
			m_ItemInfo.Color = IM_COL32(173, 173, 173, 255);
			break;
		case 3001010:
			m_ItemInfo.Name = u8"一窝蜂";
			m_ItemInfo.Color = IM_COL32(103, 142, 255, 255);
			break;
		case 3001020:
			m_ItemInfo.Name = u8"一窝蜂";
			m_ItemInfo.Color = IM_COL32(192, 122, 248, 255);
			break;
		case 3001030:
			m_ItemInfo.Name = u8"一窝蜂";
			m_ItemInfo.Color = IM_COL32(246, 192, 66, 255);
			break;
		case 3000900:
			m_ItemInfo.Name = u8"喷火筒";
			m_ItemInfo.Color = IM_COL32(173, 173, 173, 255);
			break;
		case 3000910:
			m_ItemInfo.Name = u8"喷火筒";
			m_ItemInfo.Color = IM_COL32(103, 142, 255, 255);
			break;
		case 3000920:
			m_ItemInfo.Name = u8"喷火筒";
			m_ItemInfo.Color = IM_COL32(192, 122, 248, 255);
			break;
		case 3000930:
			m_ItemInfo.Name = u8"喷火筒";
			m_ItemInfo.Color = IM_COL32(246, 192, 66, 255);
			break;
			//case 3010004:
			//	m_ItemInfo.Name = u8"凝血丸";
			//	m_ItemInfo.Color = IM_COL32(173, 173, 173, 255);
			//	break;
		case 3010006:
			m_ItemInfo.Name = u8"大包凝血丸";
			m_ItemInfo.Color = IM_COL32(245, 61, 0, 255);
			break;
			//case 3010023:
			//	m_ItemInfo.Name = u8"护甲粉末";
			//	m_ItemInfo.Color = IM_COL32(173, 173, 173, 255);
			//	break;
		case 3010024:
			m_ItemInfo.Name = u8"高级护甲粉末";
			m_ItemInfo.Color = IM_COL32(245, 61, 0, 255);
			break;
			//case 3010018:
			//	m_ItemInfo.Name = u8"武备匣";
			//	m_ItemInfo.Color = IM_COL32(173, 173, 173, 255);
			//	break;
		case 3010020:
			m_ItemInfo.Name = u8"飞索线轴";
			m_ItemInfo.Color = IM_COL32(103, 142, 255, 255);
			break;
		case 3010053:
			m_ItemInfo.Name = u8"锻造锤";
			m_ItemInfo.Color = IM_COL32(103, 142, 255, 255);
			break;
		case 3040019:
			m_ItemInfo.Name = u8"晶石粉末";
			m_ItemInfo.Color = IM_COL32(192, 122, 248, 255);
			break;
		case 3040020:
			m_ItemInfo.Name = u8"地脉晶石";
			m_ItemInfo.Color = IM_COL32(192, 122, 248, 255);
			break;
		case 3020156:
			m_ItemInfo.Name = u8"续命术";
			m_ItemInfo.Color = IM_COL32(246, 192, 66, 255);
			break;
		case 3020164:
			m_ItemInfo.Name = u8"召雷术";
			m_ItemInfo.Color = IM_COL32(246, 192, 66, 255);
			break;
		case 3020079:
			m_ItemInfo.Name = u8"毕工淌";
			m_ItemInfo.Color = IM_COL32(246, 192, 66, 255);
			break;
		case 3020116:
			m_ItemInfo.Name = u8"魂燃一线";
			m_ItemInfo.Color = IM_COL32(246, 192, 66, 255);
			break;
		case 3020124:
			m_ItemInfo.Name = u8"抽芯补天";
			m_ItemInfo.Color = IM_COL32(246, 192, 66, 255);
			break;
		case 3020151:
			m_ItemInfo.Name = u8"灵光一现";
			m_ItemInfo.Color = IM_COL32(246, 192, 66, 255);
			break;
		case 3020150:
			m_ItemInfo.Name = u8"万夫莫敌";
			m_ItemInfo.Color = IM_COL32(192, 122, 248, 255);
			break;
		case 3020157:
			m_ItemInfo.Name = u8"炼丹术";
			m_ItemInfo.Color = IM_COL32(192, 122, 248, 255);
			break;
		case 3020114:
			m_ItemInfo.Name = u8"天赐·安神";
			m_ItemInfo.Color = IM_COL32(192, 122, 248, 255);
			break;
		case 3020077:
			m_ItemInfo.Name = u8"猫足&壁虎游墙";
			m_ItemInfo.Color = IM_COL32(192, 122, 248, 255);
			break;
		case 3020078:
			m_ItemInfo.Name = u8"幻形淌&虚影步";
			m_ItemInfo.Color = IM_COL32(192, 122, 248, 255);
			break;
		case 3020174:
			m_ItemInfo.Name = u8"五毒洒";
			m_ItemInfo.Color = IM_COL32(192, 122, 248, 255);
			break;
		case 3020163:
			m_ItemInfo.Name = u8"流星飞坠";
			m_ItemInfo.Color = IM_COL32(192, 122, 248, 255);
			break;
		case 3020213:
			m_ItemInfo.Name = u8"甲爆";
			m_ItemInfo.Color = IM_COL32(192, 122, 248, 255);
			break;
		case 3020212:
			m_ItemInfo.Name = u8"金刚指";
			m_ItemInfo.Color = IM_COL32(192, 122, 248, 255);
			break;
		case 3020215:
			m_ItemInfo.Name = u8"召还术";
			m_ItemInfo.Color = IM_COL32(192, 122, 248, 255);
			break;
		case 3020131:
			m_ItemInfo.Name = u8"吐纳术";
			m_ItemInfo.Color = IM_COL32(192, 122, 248, 255);
			break;
		case 3020072:
			m_ItemInfo.Name = u8"地堂霸脚";
			m_ItemInfo.Color = IM_COL32(192, 122, 248, 255);
			break;
		case 3020073:
			m_ItemInfo.Name = u8"凌波";
			m_ItemInfo.Color = IM_COL32(192, 122, 248, 255);
			break;
		case 3020074:
			m_ItemInfo.Name = u8"完璧";
			m_ItemInfo.Color = IM_COL32(192, 122, 248, 255);
			break;
		case 3020075:
			m_ItemInfo.Name = u8"光佑";
			m_ItemInfo.Color = IM_COL32(192, 122, 248, 255);
			break;
		case 3020155:
			m_ItemInfo.Name = u8"吸星术";
			m_ItemInfo.Color = IM_COL32(192, 122, 248, 255);
			break;
		case 3020221:
			m_ItemInfo.Name = u8"极光碎云闪";
			m_ItemInfo.Color = IM_COL32(246, 192, 66, 255);
			break;
		case 3020084:
			m_ItemInfo.Name = u8"噬魂斩";
			m_ItemInfo.Color = IM_COL32(246, 192, 66, 255);
			break;
		case 3020127:
			m_ItemInfo.Name = u8"惊雷十劫";
			m_ItemInfo.Color = IM_COL32(246, 192, 66, 255);
			break;
		case 3020222:
			m_ItemInfo.Name = u8"破千军";
			m_ItemInfo.Color = IM_COL32(192, 122, 248, 255);
			break;
		case 3020085:
			m_ItemInfo.Name = u8"青鬼改";
			m_ItemInfo.Color = IM_COL32(192, 122, 248, 255);
			break;
		case 3020218:
			m_ItemInfo.Name = u8"青鬼";
			m_ItemInfo.Color = IM_COL32(192, 122, 248, 255);
			break;
		case 3020094:
			m_ItemInfo.Name = u8"夺魂·单刃";
			m_ItemInfo.Color = IM_COL32(192, 122, 248, 255);
			break;
		case 3020197:
			m_ItemInfo.Name = u8"湮沙灭空斩";
			m_ItemInfo.Color = IM_COL32(255, 68, 68, 255);
			break;
		case 3020132:
			m_ItemInfo.Name = u8"巽风震雷刀";
			m_ItemInfo.Color = IM_COL32(246, 192, 66, 255);
			break;
		case 3020193:
			m_ItemInfo.Name = u8"过关斩将";
			m_ItemInfo.Color = IM_COL32(246, 192, 66, 255);
			break;
		case 3020152:
			m_ItemInfo.Name = u8"风雷步·烈火斩";
			m_ItemInfo.Color = IM_COL32(192, 122, 248, 255);
			break;
		case 3020128:
			m_ItemInfo.Name = u8"霸主";
			m_ItemInfo.Color = IM_COL32(192, 122, 248, 255);
			break;
		case 3020095:
			m_ItemInfo.Name = u8"夺魂·重刃";
			m_ItemInfo.Color = IM_COL32(192, 122, 248, 255);
			break;
		case 3020090:
			m_ItemInfo.Name = u8"裂空";
			m_ItemInfo.Color = IM_COL32(192, 122, 248, 255);
			break;
		case 3020192:
			m_ItemInfo.Name = u8"炽焰斩";
			m_ItemInfo.Color = IM_COL32(192, 122, 248, 255);
			break;
		case 3020207:
			m_ItemInfo.Name = u8"阴烛堕丹翎";
			m_ItemInfo.Color = IM_COL32(255, 68, 68, 255);
			break;
		case 3020185:
			m_ItemInfo.Name = u8"七星夺窍";
			m_ItemInfo.Color = IM_COL32(246, 192, 66, 255);
			break;
		case 3020091:
			m_ItemInfo.Name = u8"凤凰羽";
			m_ItemInfo.Color = IM_COL32(246, 192, 66, 255);
			break;
		case 3020092:
			m_ItemInfo.Name = u8"蓝月改";
			m_ItemInfo.Color = IM_COL32(192, 122, 248, 255);
			break;
		case 3020217:
			m_ItemInfo.Name = u8"蓝月";
			m_ItemInfo.Color = IM_COL32(192, 122, 248, 255);
			break;
		case 3020096:
			m_ItemInfo.Name = u8"夺魂·长剑";
			m_ItemInfo.Color = IM_COL32(192, 122, 248, 255);
			break;
		case 3020145:
			m_ItemInfo.Name = u8"风火穿心脚";
			m_ItemInfo.Color = IM_COL32(246, 192, 66, 255);
			break;
		case 3020138:
			m_ItemInfo.Name = u8"狂浪怒涛";
			m_ItemInfo.Color = IM_COL32(246, 192, 66, 255);
			break;
		case 3020188:
			m_ItemInfo.Name = u8"五情七灭阵";
			m_ItemInfo.Color = IM_COL32(246, 192, 66, 255);
			break;
		case 3020195:
			m_ItemInfo.Name = u8"定海针·镇地撑天";
			m_ItemInfo.Color = IM_COL32(246, 192, 66, 255);
			break;
		case 3020133:
			m_ItemInfo.Name = u8"大圣游&风卷云残";
			m_ItemInfo.Color = IM_COL32(192, 122, 248, 255);
			break;
		case 3020137:
			m_ItemInfo.Name = u8"双环扫";
			m_ItemInfo.Color = IM_COL32(192, 122, 248, 255);
			break;
		case 3020220:
			m_ItemInfo.Name = u8"武道·六合枪";
			m_ItemInfo.Color = IM_COL32(192, 122, 248, 255);
			break;
		case 3020224:
			m_ItemInfo.Name = u8"武道·少林棍";
			m_ItemInfo.Color = IM_COL32(192, 122, 248, 255);
			break;
		case 3020194:
			m_ItemInfo.Name = u8"捣海棍";
			m_ItemInfo.Color = IM_COL32(192, 122, 248, 255);
			break;
		case 3020135:
			m_ItemInfo.Name = u8"夺魂·长兵器";
			m_ItemInfo.Color = IM_COL32(192, 122, 248, 255);
			break;
		case 3020187:
			m_ItemInfo.Name = u8"捅劲";
			m_ItemInfo.Color = IM_COL32(192, 122, 248, 255);
			break;
		case 3020198:
			m_ItemInfo.Name = u8"绝痕瞬影霎";
			m_ItemInfo.Color = IM_COL32(255, 68, 68, 255);
			break;
		case 3020142:
			m_ItemInfo.Name = u8"鬼哭神嚎";
			m_ItemInfo.Color = IM_COL32(246, 192, 66, 255);
			break;
		case 3020177:
			m_ItemInfo.Name = u8"亢龙有悔";
			m_ItemInfo.Color = IM_COL32(246, 192, 66, 255);
			break;
		case 3020211:
			m_ItemInfo.Name = u8"缠龙奔野";
			m_ItemInfo.Color = IM_COL32(246, 192, 66, 255);
			break;
		case 3020139:
			m_ItemInfo.Name = u8"鬼刃暗扎";
			m_ItemInfo.Color = IM_COL32(192, 122, 248, 255);
			break;
		case 3020208:
			m_ItemInfo.Name = u8"双开圆";
			m_ItemInfo.Color = IM_COL32(192, 122, 248, 255);
			break;
		case 3020141:
			m_ItemInfo.Name = u8"幽冥步";
			m_ItemInfo.Color = IM_COL32(192, 122, 248, 255);
			break;
		case 3020140:
			m_ItemInfo.Name = u8"荆轲献匕";
			m_ItemInfo.Color = IM_COL32(192, 122, 248, 255);
			break;
		case 3020209:
			m_ItemInfo.Name = u8"三风摆";
			m_ItemInfo.Color = IM_COL32(192, 122, 248, 255);
			break;
		case 3020210:
			m_ItemInfo.Name = u8"鬼返";
			m_ItemInfo.Color = IM_COL32(192, 122, 248, 255);
			break;
		case 3020159:
			m_ItemInfo.Name = u8"夺魂·短兵器";
			m_ItemInfo.Color = IM_COL32(192, 122, 248, 255);
			break;
		case 3020158:
			m_ItemInfo.Name = u8"虎啸龙咆";
			m_ItemInfo.Color = IM_COL32(246, 192, 66, 255);
			break;
		case 3020143:
			m_ItemInfo.Name = u8"机触&破门闩";
			m_ItemInfo.Color = IM_COL32(192, 122, 248, 255);
			break;
		case 3020144:
			m_ItemInfo.Name = u8"飞踢";
			m_ItemInfo.Color = IM_COL32(192, 122, 248, 255);
			break;
		case 3020146:
			m_ItemInfo.Name = u8"扬鞭劲";
			m_ItemInfo.Color = IM_COL32(192, 122, 248, 255);
			break;
		case 3020161:
			m_ItemInfo.Name = u8"夺魂·双节棍";
			m_ItemInfo.Color = IM_COL32(192, 122, 248, 255);
			break;
		case 3020196:
			m_ItemInfo.Name = u8"炙魄灼魂刀";
			m_ItemInfo.Color = IM_COL32(255, 68, 68, 255);
			break;
		case 3020219:
			m_ItemInfo.Name = u8"武道·八斩刀";
			m_ItemInfo.Color = IM_COL32(246, 192, 66, 255);
			break;
		case 3020183:
			m_ItemInfo.Name = u8"乾坤日月斩";
			m_ItemInfo.Color = IM_COL32(246, 192, 66, 255);
			break;
		case 3020206:
			m_ItemInfo.Name = u8"战龙在天";
			m_ItemInfo.Color = IM_COL32(246, 192, 66, 255);
			break;
		case 3020179:
			m_ItemInfo.Name = u8"分水斩";
			m_ItemInfo.Color = IM_COL32(192, 122, 248, 255);
			break;
		case 3020205:
			m_ItemInfo.Name = u8"勾旋斩";
			m_ItemInfo.Color = IM_COL32(192, 122, 248, 255);
			break;
		case 3020181:
			m_ItemInfo.Name = u8"铁马残红";
			m_ItemInfo.Color = IM_COL32(192, 122, 248, 255);
			break;
		case 3020199:
			m_ItemInfo.Name = u8"蛟龙入海";
			m_ItemInfo.Color = IM_COL32(192, 122, 248, 255);
			break;
		case 3020180:
			m_ItemInfo.Name = u8"夺魂·双持";
			m_ItemInfo.Color = IM_COL32(192, 122, 248, 255);
			break;
		case 3020173:
			m_ItemInfo.Name = u8"强袭·离火式";
			m_ItemInfo.Color = IM_COL32(192, 122, 248, 255);
			break;
		case 3020182:
			m_ItemInfo.Name = u8"易武·万华";
			m_ItemInfo.Color = IM_COL32(192, 122, 248, 255);
			break;
		case 3020101:
			m_ItemInfo.Name = u8"爆裂箭";
			m_ItemInfo.Color = IM_COL32(246, 192, 66, 255);
			break;
		case 3020129:
			m_ItemInfo.Name = u8"火箭&锁羚羊";
			m_ItemInfo.Color = IM_COL32(192, 122, 248, 255);
			break;
		case 3020098:
			m_ItemInfo.Name = u8"散射";
			m_ItemInfo.Color = IM_COL32(192, 122, 248, 255);
			break;
		case 3020153:
			m_ItemInfo.Name = u8"自疗弩&治疗箭";
			m_ItemInfo.Color = IM_COL32(192, 122, 248, 255);
			break;
		case 3020105:
			m_ItemInfo.Name = u8"毒箭&断筋弩";
			m_ItemInfo.Color = IM_COL32(192, 122, 248, 255);
			break;
		case 3020122:
			m_ItemInfo.Name = u8"火龙炮";
			m_ItemInfo.Color = IM_COL32(246, 192, 66, 255);
			break;
		case 3020117:
			m_ItemInfo.Name = u8"爆风炮&燃烧";
			m_ItemInfo.Color = IM_COL32(192, 122, 248, 255);
			break;
		case 3020106:
			m_ItemInfo.Name = u8"连珠炮&反弹";
			m_ItemInfo.Color = IM_COL32(192, 122, 248, 255);
			break;
		case 3020121:
			m_ItemInfo.Name = u8"御地雷";
			m_ItemInfo.Color = IM_COL32(246, 192, 66, 255);
			break;
		case 3020123:
			m_ItemInfo.Name = u8"快蓄&流天雷";
			m_ItemInfo.Color = IM_COL32(192, 122, 248, 255);
			break;
		case 3020110:
			m_ItemInfo.Name = u8"穿甲弹";
			m_ItemInfo.Color = IM_COL32(246, 192, 66, 255);
			break;
		case 3020130:
			m_ItemInfo.Name = u8"缓射&落金乌";
			m_ItemInfo.Color = IM_COL32(192, 122, 248, 255);
			break;
		case 3020216:
			m_ItemInfo.Name = u8"小彩头";
			m_ItemInfo.Color = IM_COL32(192, 122, 248, 255);
			break;
			//case 3020000:
			//	m_ItemInfo.Name = u8"小魂玉·体力";
			//	m_ItemInfo.Color = IM_COL32(173, 173, 173, 255);
			//	break;
		case 3020001:
			m_ItemInfo.Name = u8"大魂玉·体力";
			m_ItemInfo.Color = IM_COL32(103, 142, 255, 255);
			break;
		case 3020002:
			m_ItemInfo.Name = u8"纯净魂玉·体力";
			m_ItemInfo.Color = IM_COL32(192, 122, 248, 255);
			break;
		case 3020003:
			m_ItemInfo.Name = u8"完美魂玉·体力";
			m_ItemInfo.Color = IM_COL32(246, 192, 66, 255);
			break;
			//case 3020010:
			//	m_ItemInfo.Name = u8"小魂玉·攻击";
			//	m_ItemInfo.Color = IM_COL32(173, 173, 173, 255);
			//	break;
		case 3020011:
			m_ItemInfo.Name = u8"大魂玉·攻击";
			m_ItemInfo.Color = IM_COL32(103, 142, 255, 255);
			break;
		case 3020012:
			m_ItemInfo.Name = u8"纯净魂玉·攻击";
			m_ItemInfo.Color = IM_COL32(192, 122, 248, 255);
			break;
		case 3020013:
			m_ItemInfo.Name = u8"完美魂玉·攻击";
			m_ItemInfo.Color = IM_COL32(246, 192, 66, 255);
			break;
			//case 3020050:
			//	m_ItemInfo.Name = u8"小魂玉·近抗";
			//	m_ItemInfo.Color = IM_COL32(173, 173, 173, 255);
			//	break;
		case 3020051:
			m_ItemInfo.Name = u8"大魂玉·近抗";
			m_ItemInfo.Color = IM_COL32(103, 142, 255, 255);
			break;
		case 3020052:
			m_ItemInfo.Name = u8"纯净魂玉·近抗";
			m_ItemInfo.Color = IM_COL32(192, 122, 248, 255);
			break;
		case 3020053:
			m_ItemInfo.Name = u8"完美魂玉·近抗";
			m_ItemInfo.Color = IM_COL32(246, 192, 66, 255);
			break;
			//case 3020060:
			//	m_ItemInfo.Name = u8"小魂玉·远抗";
			//	m_ItemInfo.Color = IM_COL32(173, 173, 173, 255);
			//	break;
		case 3020061:
			m_ItemInfo.Name = u8"大魂玉·远抗";
			m_ItemInfo.Color = IM_COL32(103, 142, 255, 255);
			break;
		case 3020062:
			m_ItemInfo.Name = u8"纯净魂玉·远抗";
			m_ItemInfo.Color = IM_COL32(192, 122, 248, 255);
			break;
		case 3020063:
			m_ItemInfo.Name = u8"完美魂玉·远抗";
			m_ItemInfo.Color = IM_COL32(246, 192, 66, 255);
			break;
			//case 3040001:
			//	m_ItemInfo.Name = u8"暗潮币";
			//	m_ItemInfo.Color = IM_COL32(0, 184, 245, 255);
			//	break;
		case 3040002:
			m_ItemInfo.Name = u8"魂冢";
			m_ItemInfo.Color = IM_COL32(255, 0, 0, 255);
			break;
		case 3010001:
			m_ItemInfo.Name = u8"魂玉扩容背包";
			m_ItemInfo.Color = IM_COL32(245, 61, 0, 255);
			break;
		case 3010002:
			m_ItemInfo.Name = u8"武器扩容背包";
			m_ItemInfo.Color = IM_COL32(245, 61, 0, 255);
			break;
		case 3010003:
			m_ItemInfo.Name = u8"道具扩容背包";
			m_ItemInfo.Color = IM_COL32(245, 61, 0, 255);
			break;
		case 3010082:
			m_ItemInfo.Name = u8"宝窟之钥·龟兹";
			m_ItemInfo.Color = IM_COL32(246, 192, 66, 255);
			break;
		case 3010028:
			m_ItemInfo.Name = u8"地煞符·初级";
			m_ItemInfo.Color = IM_COL32(103, 142, 255, 255);
			break;
		case 3010029:
			m_ItemInfo.Name = u8"地煞符·中级";
			m_ItemInfo.Color = IM_COL32(192, 122, 248, 255);
			break;
		case 3010030:
			m_ItemInfo.Name = u8"地煞符·高级";
			m_ItemInfo.Color = IM_COL32(246, 192, 66, 255);
			break;
		default:
			break;
		}
		return m_ItemInfo;
	}

	//取建筑名称
	DataInfo GetBuildingName(uint32_t BuildingID) {
		DataInfo m_ItemInfo;
		switch (BuildingID) {
		case 4000000:
			m_ItemInfo.Name = u8"返魂台";
			m_ItemInfo.Color = IM_COL32(192, 192, 192, 255);
			break;
		case 4000001:
			m_ItemInfo.Name = u8"冶炼台";
			m_ItemInfo.Color = IM_COL32(192, 192, 192, 255);
			break;
		case 4000004:
			m_ItemInfo.Name = u8"黑市商人";
			m_ItemInfo.Color = IM_COL32(192, 192, 192, 255);
			break;
		case 4000002:	case 4000100:	case 4000101:	case 4000102:	case 4000103:
		case 4000104:	case 4000200:	case 4000250:	case 4000251:	case 4000252:
		case 4000253:	case 4000254:	case 4000255:	case 4002001:	case 4002101:
		case 4002103:	case 4002108:	case 4002906:	case 4002907:	case 4002928:
			m_ItemInfo.Name = u8"货郎";
			m_ItemInfo.Color = IM_COL32(192, 192, 192, 255);
			break;
		case 4000003:	case 4000007:	case 4000016:	case 4000019:	case 4002004:
		case 4002105:	case 4002900:	case 4002901:	case 4002920:	case 4002923:
		case 4002931:	case 4002935:	case 4002937:	case 4002200:	case 4002203:
		case 4002208:	case 4002949:	case 4002946:	case 4002940:	case 4002942:
		case 4002979:	case 4002976:	case 4002964:	case 4002961:	case 4002969:
		case 4002972:	case 4002984:	case 4002987:
			m_ItemInfo.Name = u8"绿堆";
			m_ItemInfo.Color = IM_COL32(61, 245, 0, 255);
			break;
		case 4000005:	case 4000008:	case 4000014:	case 4000017:	case 4002002:
		case 4002106:	case 4002921:	case 4002924:	case 4002926:	case 4002929:
		case 4002201:	case 4002204:	case 4002206:	case 4002950:	case 4002947:
		case 4002980:	case 4002977:	case 4002965:	case 4002962:	case 4002967:
		case 4002970:	case 4002982:	case 4002985:
			m_ItemInfo.Name = u8"蓝堆";
			m_ItemInfo.Color = IM_COL32(0, 184, 245, 255);
			break;
		case 4000006:	case 4000009:	case 4000015:	case 4000018:	case 4000203:
		case 4000204:	case 4002003:	case 4002107:	case 4002902:	case 4002903:
		case 4002922:	case 4002925:	case 4002927:	case 4002930:	case 4002936:
		case 4002938:	case 4002202:	case 4002205:	case 4002207:	case 4002951:
		case 4002948:	case 4002939:	case 4002941:	case 4002981:	case 4002978:
		case 4002966:	case 4002963:	case 4002968:	case 4002971:	case 4002983:
		case 4002986:
			m_ItemInfo.Name = u8"金堆";
			m_ItemInfo.Color = IM_COL32(255, 209, 26, 255);
			break;
		case 4000010:	case 4000011:
			m_ItemInfo.Name = u8"任务宝箱";
			m_ItemInfo.Color = IM_COL32(192, 192, 192, 255);
			break;
		case 4000020:	case 4000021:	case 4000022:	case 4000023:	case 4000024:
		case 4002904:	case 4002905:	case 4000025:	case 4000026:	case 4002005:
			m_ItemInfo.Name = u8"土地庙";
			m_ItemInfo.Color = IM_COL32(192, 192, 192, 255);
			break;
		case 4000110:	case 4000111:	case 4000112:	case 4000113:	case 4000114:
		case 4000115:	case 4000116:	case 4000117:	case 4000118:	case 4000119:
		case 4000120:	case 4000121:	case 4000122:	case 4000123:	case 4000124:
		case 4000125:	case 4000130:	case 4000131:	case 4000201:	case 4002000:
		case 4002006:	case 4002007:	case 4002008:	case 4002009:	case 4002102:
		case 4002908:	case 4002909:	case 4002910:	case 4002911:	case 4002213:
		case 4002212:	case 4000163:	case 4000165:
			m_ItemInfo.Name = u8"飓风客";
			m_ItemInfo.Color = IM_COL32(255, 215, 0, 255);
			break;
		case 4000202:	case 4001000:
			m_ItemInfo.Name = u8"灵魂宝箱";
			m_ItemInfo.Color = IM_COL32(192, 192, 192, 255);
			break;
		case 4000997:
			m_ItemInfo.Name = u8"叫阵卷轴";
			m_ItemInfo.Color = IM_COL32(255, 165, 0, 255);
			break;
		case 4000998:
			m_ItemInfo.Name = u8"追击卷轴";
			m_ItemInfo.Color = IM_COL32(255, 165, 0, 255);
			break;
		case 4000999:
			m_ItemInfo.Name = u8"任务卷轴";
			m_ItemInfo.Color = IM_COL32(204, 71, 75, 255);
			break;
		case 4002100:
			m_ItemInfo.Name = u8"银蛋";
			m_ItemInfo.Color = IM_COL32(192, 192, 192, 255);
			break;
		case 4002104:
			m_ItemInfo.Name = u8"金蛋";
			m_ItemInfo.Color = IM_COL32(255, 215, 0, 255);
			break;
		default:
			break;
		}
		return m_ItemInfo;
	}

	//取物品坐标
	Vector3 GetItemPos(uint64_t CoordinatePointer) {
		Vector3 Pos;

		Memory::ReadByte(CoordinatePointer, sizeof(Pos), &Pos);
		return Pos;
	}

	//取武器编号
	uint32_t GetWeaponId(uint64_t PlayerAddress) {
		uint64_t Buffer = Memory::ReadMemory<uint64_t>(PlayerAddress + SDK::武器属性);
		uint32_t ReturnData = Memory::ReadMemory<uint32_t>(Buffer + SDK::武器编号);
		return ReturnData;
	}

	//取振刀状态
	uint32_t GetZhenDaoAction(uint64_t PlayerAddress) {
		uint64_t Buffer = Memory::ReadMemory<uint64_t>(PlayerAddress + SDK::状态一级);
		Buffer = Memory::ReadMemory<uint64_t>(Buffer + SDK::状态二级);
		uint32_t ReturnData = Memory::ReadMemory<uint32_t>(Buffer + SDK::振刀动作);
		return ReturnData;
	}

	//取攻击动作
	uint32_t GetAttackAction(uint64_t PlayerAddress) {
		uint64_t Buffer = Memory::ReadMemory<uint64_t>(PlayerAddress + SDK::状态一级);
		Buffer = Memory::ReadMemory<uint64_t>(Buffer + SDK::状态二级);
		uint32_t ReturnData = Memory::ReadMemory<uint32_t>(Buffer + SDK::攻击动作);
		return ReturnData;
	}

	//取僵直状态
	BOOL GetStiffStatus(uint64_t PlayerAddress) {
		uint64_t Buffer = Memory::ReadMemory<uint64_t>(PlayerAddress + SDK::状态一级);
		Buffer = Memory::ReadMemory<uint64_t>(Buffer + SDK::状态二级);
		float ActionTime = Memory::ReadMemory<float>(Buffer + SDK::行动时间);
		float MotionTime = Memory::ReadMemory<float>(Buffer + SDK::动作时间);
		float EffectiveTime = MotionTime - ActionTime;
		return EffectiveTime > 0.0f && MotionTime != 0.0f;
	}

	//取蓄力时间
	float GetChargeTime(uint64_t PlayerAddress) {
		uint64_t ActorKit = Memory::ReadMemory<uint64_t>(PlayerAddress + SDK::蓄时一级);
		if (ActorKit > 1000) {
			uint64_t BackingField = Memory::ReadMemory<uint64_t>(ActorKit + SDK::蓄时二级);
			if (BackingField > 1000) {
				uint64_t FloatParamsContainer = Memory::ReadMemory<uint64_t>(BackingField + SDK::蓄时三级);
				if (FloatParamsContainer > 1000) {
					uint64_t ValueList = Memory::ReadMemory<uint64_t>(FloatParamsContainer + SDK::蓄时四级);
					if (ValueList > 1000) {
						return(Memory::ReadMemory<float>(Memory::ReadMemory<uint64_t>(ValueList + SDK::蓄时五级) + SDK::蓄时六级));
					}
				}
			}
		}
		return 0.0f;
	}

	//取蓄力状态
	BOOL GetXuliAfter(uint64_t Object) {
		if (Object == g_LocalPlayerAddress) {
			uint64_t Buffer = Memory::ReadMemory<uint64_t>(Object + SDK::组合一级);
			Buffer = Memory::ReadMemory<uint64_t>(Buffer + SDK::组合二级);
			uint32_t ReturnData = Memory::ReadMemory<uint32_t>(Buffer + SDK::蓄力层级);
			return ReturnData != NULL;
		}
		return GetChargeTime(Object) > 0.51f;
	}

	//取武器速度
	float GetWeaponSpeed(uint32_t WeaponId) {
		switch (WeaponId) {
		case 113:
			return 150.0f;  // 鸟铳
		case 104:
			return (GetChargeTime(g_LocalPlayerAddress) >= 0.8f) ? 150.0f : 110.0f;
		case 108:
			return 150.0f;
		case 109:
			return 40.0f;
		case 112:
			return 150.0f;
		case 110:
			return 25.0f;
		case 111:
			return 40.0f;
		default:
			return 100.0f;
		}
	}
	//取对象指针
	uint64_t GetRootComponent(uint64_t PlayerEntity) {
		uint64_t Buffer = Memory::ReadMemory<uint64_t>(PlayerEntity + SDK::指针四级);
		Buffer = Memory::ReadMemory<uint64_t>(Buffer + SDK::指针五级);
		Buffer = Memory::ReadMemory<uint64_t>(Buffer + SDK::指针六级);
		return Buffer;
	}

	//取对象状态
	uint32_t GetStatus(uint64_t PlayerAddress) {
		uint64_t Buffer = Memory::ReadMemory<uint64_t>(PlayerAddress + SDK::状态一级);
		Buffer = Memory::ReadMemory<uint64_t>(Buffer + SDK::状态二级);
		uint32_t ReturnData = Memory::ReadMemory<uint32_t>(Buffer + SDK::当前状态);
		return ReturnData;
	}

	//取攻击范围
	float GetAttackRange(uint64_t PlayerAddress) {
		uint64_t Buffer = Memory::ReadMemory<uint64_t>(PlayerAddress + SDK::状态一级);
		Buffer = Memory::ReadMemory<uint64_t>(Buffer + SDK::状态二级);
		float ReturnData = Memory::ReadMemory<float>(Buffer + SDK::攻击范围);
		return ReturnData;
	}

	//取招式文本
	wstring GetActionName(uint64_t PlayerAddress) {
		wchar_t ReturnData[128] = { NULL };
		uint64_t Buffer = Memory::ReadMemory<uint64_t>(PlayerAddress + SDK::招式一级) + SDK::招式二级;
		Buffer = Memory::ReadMemory<uint64_t>(Buffer) + SDK::招式三级;
		Buffer = Memory::ReadMemory<uint64_t>(Buffer) + SDK::招式四级;
		Buffer = Memory::ReadMemory<uint64_t>(Buffer) + SDK::招式五级;
		Buffer = Memory::ReadMemory<uint64_t>(Buffer) + SDK::招式六级;
		Buffer = Memory::ReadMemory<uint64_t>(Buffer) + SDK::招式七级;
		Buffer = Memory::ReadMemory<uint64_t>(Buffer) + SDK::招式八级;
		Memory::ReadByte(Buffer, sizeof(ReturnData), &ReturnData);
		return wstring(ReturnData);
	}

	//取招式标识
	uint32_t GetActionId(uint64_t PlayerAddress) {
		uint64_t Buffer = Memory::ReadMemory<uint64_t>(PlayerAddress + SDK::对象一级);
		uint32_t ReturnData = Memory::ReadMemory<uint32_t>(Buffer + SDK::招式标识);
		return ReturnData;
	}
}

namespace Comp {

	float GetPlayerDistance(Vector3 dPos, Vector3 mPos) {
		float deltaX = dPos.x - mPos.x;
		float deltaY = dPos.y - mPos.y;
		float deltaZ = dPos.z - mPos.z;
		return sqrt(deltaX * deltaX + deltaY * deltaY + deltaZ * deltaZ);
	}
	double GetitemDistance(Vector3 dPos, Vector3 mPos) {
		double deltaX = dPos.x - mPos.x;
		double deltaY = dPos.y - mPos.y;
		double deltaZ = dPos.z - mPos.z;
		return sqrt(deltaX * deltaX + deltaY * deltaY + deltaZ * deltaZ);
	}
	float GetDistance(float ReticleX, float ReticleY, float PlayerX, float PlayerY) {
		float x = ReticleX - PlayerX;
		float y = ReticleY - PlayerY;
		return sqrt(x * x + y * y);
	}

	//屏幕2d世界
	BOOL WorldToScreen2D(const Vector3& worldPos, Vector2& outScreenPos) {
		float View = 0.f;
		float SightX = Gui.Window.Size.x / 2, SightY = Gui.Window.Size.y / 2;
		View = g_MatrixData[0][3] * worldPos.x + g_MatrixData[1][3] * worldPos.y + g_MatrixData[2][3] * worldPos.z + g_MatrixData[3][3];

		if (View <= 0.01f) {
			return false;
		}
		outScreenPos.x = SightX + (g_MatrixData[0][0] * worldPos.x + g_MatrixData[1][0] * worldPos.y + g_MatrixData[2][0] * worldPos.z + g_MatrixData[3][0]) / View * SightX;
		outScreenPos.y = SightY - (g_MatrixData[0][1] * worldPos.x + g_MatrixData[1][1] * worldPos.y + g_MatrixData[2][1] * worldPos.z + g_MatrixData[3][1]) / View * SightY;
		return true;
	}

	//屏幕3d世界
	BOOL WorldToScreen3D(const Vector3& worldPos, Vector4& outScreenPos) {
		Vector2 footScreenPos;
		Vector3 headPos;
		Vector2 headScreenPos;
		if (WorldToScreen2D(worldPos, footScreenPos)) {
			headPos = worldPos;
			headPos.y = worldPos.y + 1.9f;

			if (WorldToScreen2D(headPos, headScreenPos)) {
				float height = footScreenPos.y - headScreenPos.y;
				float width = height / 4;
				outScreenPos.x = footScreenPos.x - width;
				outScreenPos.y = headScreenPos.y;
				outScreenPos.z = width * 2;
				outScreenPos.w = height;


				return true;
			}
		}
		return false;
	}
}

namespace Hex {
	std::string n2hexstr(long long w, size_t hex_len) {
		static const char* digits = "0123456789ABCDEF";
		std::string rc(hex_len, '0');
		for (size_t i = 0, j = (hex_len - 1) * 4; i < hex_len; ++i, j -= 4)
			rc[i] = digits[(w >> j) & 0x0f];
		return rc;
	}
	BYTE* HexChangeByte(std::string CurString, bool Flag, int StrLen, PBYTE zz)
	{
		std::string aa;
		BYTE ReByte[4] = { 0 };
		//cout << "要转换的字符串:" << CurString << endl;
		//BYTE* ReByte = new BYTE;
		int Pos = 0;
		long temporary = 0;
		int n = 0;
		int size = 0;

		aa = CurString;

		if (strlen(aa.c_str()) % 2 != 0)
			aa = "0" + aa;

		//cout << "加0后的字符串" << aa << endl;
		//cout << strlen(aa.c_str()) << endl;
		//cout << hex << aa << endl;
		//cout << hex<< aa.substr(0, 2) << endl;

		//cout << strlen(aa.c_str()) - 1 << endl;

		for (int cc = (int)(strlen(aa.c_str()) - 1); cc >= 1; cc -= 2)
		{
			//temporary = hex2num(aa.substr(Pos, 2));
			zz[size] = { (BYTE)stoul(aa.substr(Pos, 2), nullptr, 16) };
			//printf("%X, ", zz[size]);
			size++;
			Pos += 2;
		}

		if (Flag)
		{
			if (size < StrLen)
			{
				int a = 0;
				for (int i = size; i >= 0; i--)
				{
					zz[StrLen - a] = zz[i];
					a++;
				}
				for (int i = 0; i < StrLen - size; i++)
				{
					zz[i] = { 0 };
				}
				//for (int i = size; i < StrLen; i++)
				//{
				//	zz[i] = { 0 };
				//}
			}
		}
		return zz;
	}
}
namespace Hook {

	VOID KnifeStartHook() {
		Aim::FlushAddr = Memory::AllocMemory(0x1000);
		if (!Aim::FlushAddr) { return; }

		Aim::Knifevtable = Memory::ReadMemory<uint64_t>(g_Module.GameAssembly + SDK::InitActorKitRefer) + 0x198;

		BYTE Char[8] = { 0 };
		Hex::HexChangeByte(Hex::n2hexstr(g_Module.GameAssembly + SDK::世界地址, 16Ui64), true, 8, Char);

		BYTE Refer[8] = { 0 };
		Hex::HexChangeByte(Hex::n2hexstr(g_Module.GameAssembly + SDK::InitActorKitRefer, 16Ui64), true, 8, Refer);

		BYTE ShellCode[] =
		{
			0x41,0x81,0xF9,0x7E,0x29,0x25,0xAF,
			0x75,0x52,
			0x48,0x83,0xFA,0x00,
			0x75,0x4C,
			0x48,0xA1,Char[7], Char[6], Char[5], Char[4], Char[3], Char[2], Char[1], Char[0],
			0x48,0x8B,0x80,0xB8,0x00,0x00,0x00,
			0x48,0x8B,0x40,0x08,
			0x48,0x8B,0x40,0x18,
			0x48,0x8B,0x80,SDK::ActorModel_actorKit, 0x00, 0x00,0x00,
			0x48,0x8B,0x80,SDK::m_ActorModel_ActorKitBreakData, 0x00, 0x00,0x00,
			0xC7,0x40,0x10,0x00,0x30,0x00,0x00,
			0x48,0xA1,Refer[7], Refer[6], Refer[5], Refer[4], Refer[3], Refer[2], Refer[1], Refer[0],
			0x48,0x8D,0x80,0x98,0x01,0x00,0x00,
			0x4C,0x8B,0x25,0x0D,0x00,0x00,0x00,
			0x4C,0x89,0x20,
			0x4D,0x31,0xE4,
			0xFF,0x35,0x01,0x00,0x00,0x00,
			0xC3
		};

		Memory::WriteByte(Aim::FlushAddr, sizeof(ShellCode), ShellCode);
		Memory::WriteMemory<uint64_t>(Aim::FlushAddr + 0x62, g_Module.GameAssembly + SDK::FlushState);
	}

	VOID HookZhenDao(BOOL Mode, BOOL Special) {
		if (Special) {
			Memory::WriteMemory<uint64_t>(Aim::Knifevtable, Aim::FlushAddr);
		}
		if (Mode) {
			keyMsg::KeyDown('C');
		}
		else {
			keyMsg::KeyDown(VK_SPACE);
		}
		Sleep(10);
		keyMsg::RightDown();
		keyMsg::LeftDown();
		Sleep(30);
		if (Mode) {
			keyMsg::KeyUp('C');
		}
		else {
			keyMsg::KeyUp(VK_SPACE);
		}
		keyMsg::RightUp();
		keyMsg::LeftUp();
		if (Special) {
			Memory::WriteMemory<uint64_t>(Aim::Knifevtable, Aim::FlushAddr);
		}
	}
}

namespace Face {

	float ConverseAngle(float x) {
		float tmp = 0.0f;
		tmp = (int(x + 180) % 360 + 180) % 360;
		return tmp;
	}

	Rotation QuatToRot(Vector4 Quaternion) {
		Rotation rot;
		float qzsqr = 0.0f, T0 = 0.0f, T1 = 0.0f, T2 = 0.0f, T3 = 0.0f, T4 = 0.0f;

		qzsqr = Quaternion.z * Quaternion.z;
		T0 = -2 * (qzsqr + Quaternion.w * Quaternion.w) + 1;
		T1 = 2 * (Quaternion.y * Quaternion.z + Quaternion.x * Quaternion.w);
		T2 = -2 * (Quaternion.y * Quaternion.w - Quaternion.x * Quaternion.z);
		T3 = 2 * (Quaternion.z * Quaternion.w + Quaternion.x * Quaternion.y);
		T4 = -2 * (Quaternion.y * Quaternion.y + qzsqr) + 1;

		T2 = T2 > 1 ? 1 : T2;
		T2 = T2 < -1 ? -1 : T2;

		rot.Yaw = atan2(T1, T0) * 180.0F / PI;
		rot.Roll = atan2(T3, T4) * 180.0F / PI;
		rot.Pitch = asin(T2) * 180.0F / PI;
		return rot;
	}

	Vector4 GetRootQuaternion(uint64_t RootComponent) {

		Vector4 Temp;
		Temp.x = Memory::ReadMemory<float>(RootComponent + 0xA0 + 0);
		Temp.y = Memory::ReadMemory<float>(RootComponent + 0xA0 + 4);
		Temp.z = Memory::ReadMemory<float>(RootComponent + 0xA0 + 8);
		Temp.w = Memory::ReadMemory<float>(RootComponent + 0xA0 + 12);
		return Temp;
	}

	float GetDirection(uint64_t RootComponent) {

		Vector4 Quat;
		Rotation Rot;

		Quat = GetRootQuaternion(RootComponent);
		Quat.x = 0.F;
		Quat.z = 0.F;
		Rot = QuatToRot(Quat);
		Rot.Pitch = Rot.Pitch * -1;
		if (abs(Quat.y) >= 0.7f) {
			Rot.Pitch = 180.0f - Rot.Pitch;
		}
		return ConverseAngle(Rot.Pitch);
	}

	Vector3 FindLookAtVector(Vector3 v1, Vector3 v2) {

		Vector3 ret;
		float distance = 0.0f;
		ret.x = v2.x - v1.x;
		ret.y = v2.y - v1.y;
		ret.z = v2.z - v1.z;
		distance = sqrt(ret.x * ret.x + ret.y * ret.y + ret.z * ret.z);
		ret.x = ret.x / distance;
		ret.y = ret.y / distance;
		ret.z = ret.z / distance;
		return ret;
	}

	float VectorToRotationYaw(Vector3 Vector) {
		float ret = 0.0f;
		ret = atan2(Vector.x, Vector.y) * 360 / 6.2831852;
		if (ret < 0) {
			ret = ret + 180;
		}
		if (Vector.x < 0) {
			ret = ret + 180;
		}
		return ret;
	}

	float AngularDifference(FLOAT a, FLOAT b) {
		float phi = 0.0f, result = 0.0f;
		phi = int(abs(b - a)) % 360;
		result = phi > 180 ? 360 - phi : phi;
		return result;
	}
}

namespace Judg {

	BOOL JudgeSky(wstring ActionName) {
		if (ActionName.find(L"jump") != wstring::npos) {
			return true;
		}
		if (ActionName.find(L"airhike") != wstring::npos) {
			return true;
		}
		if (ActionName.find(L"glide") != wstring::npos) {
			return true;
		}
		if (ActionName.find(L"walk_wall") != wstring::npos) {
			return true;
		}
		if (ActionName.find(L"fly") != wstring::npos) {
			return true;
		}
		return false;
	}

	BOOL JudgeMeleeWeapon(uint32_t WeaponId) {
		if (WeaponId == 101) {
			return true;
		}
		if (WeaponId == 102) {
			return true;
		}
		if (WeaponId == 103) {
			return true;
		}
		if (WeaponId == 105) {
			return true;
		}
		if (WeaponId == 106) {
			return true;
		}
		if (WeaponId == 116) {
			return true;
		}
		if (WeaponId == 118) {
			return true;
		}
		if (WeaponId == 119) {
			return true;
		}
		if (WeaponId == 120) {
			return true;
		}
		if (WeaponId == 121) {
			return true;
		}
		if (WeaponId == 122) {
			return true;
		}
		if (WeaponId == 123) {
			return true;
		}
		if (WeaponId == 124) {
			return true;
		}
		if (WeaponId == 207) {
			return true;
		}
		if (WeaponId == 208) {
			return true;
		}
		return false;
	}

	BOOL JudgeStatus(uint32_t StatusId) {
		if (StatusId == 30) {
			return false;
		}
		if (StatusId == 31) {
			return false;
		}
		if (StatusId == 35) {
			return false;
		}
		if (StatusId == 50) {
			return false;
		}
		if (StatusId == 51) {
			return false;
		}
		if (StatusId == 52) {
			return false;
		}
		if (StatusId == 53) {
			return false;
		}
		if (StatusId == 55) {
			return false;
		}
		if (StatusId == 59) {
			return false;
		}
		if (StatusId == 80) {
			return false;
		}
		return true;
	}

	BOOL JudgeZhenDao(wstring ActionName) {
		if (ActionName.find(L"shock") != wstring::npos) {
			return false;
		}
		if (ActionName.find(L"emptystep") != wstring::npos) {
			return false;
		}
		if (ActionName.find(L"bounce") != wstring::npos) {
			return false;
		}
		return true;
	}

	BOOL JudgeYpAction(wstring ActionName) {
		if (ActionName.find(L"heavy_soul_01_charge") != wstring::npos) {//虎啸龙袍
			return true;
		}
		if (ActionName.find(L"heavy_charge_soul_01") != wstring::npos) {//噬魂斩
			return true;
		}
		if (ActionName.find(L"light_charge_soul_01") != wstring::npos) {//噬魂斩
			return true;
		}
		if (ActionName.find(L"fan_flashstep_attack_light_02_pre") != wstring::npos) {//扇鬼反
			return true;
		}
		return false;
	}

	BOOL JudgeSkillRecovery(wstring ActionName) {
		return ActionName.find(L"recover") != wstring::npos;
	}

	BOOL JudgeLongFlash(wstring ActionName) {
		return ActionName.find(L"dodge") != wstring::npos && ActionName.find(L"sprint_attack") == wstring::npos;
	}

	BOOL JudgeShortFlash(wstring ActionName) {
		return ActionName.find(L"dodge") != wstring::npos;
	}
}

namespace Logc {

	BOOL LogicName(wstring PlayerActionName, wstring ActionName) {
		return PlayerActionName == L"male_" + ActionName || PlayerActionName == L"female_" + ActionName;
	}

	BOOL LogicNcZhenDao(uint32_t WeaponId, uint32_t Status, wstring ActionName, float AttackRange, float Distance, float Direction, BOOL& Special, uint64_t PlayerAddress) {
		if (WeaponId == 101 && (Status == 20 || Status == 22) && Direction <= AttackRange / 1.8f) {//长剑
			if (Distance <= 5.0f) {
				if (LogicName(ActionName, L"sw_attack_light_03")) {//长剑左三连
					Special = false;
					return true;
				}
			}
			if (Distance <= 5.0f) {
				if (LogicName(ActionName, L"sw_attack_heavy_03")) {//长剑右三连
					Special = false;
					return true;
				}
			}
			if (Distance <= 16.0f) {
				if (LogicName(ActionName, L"sw_attack_light_copy_03")) {//长剑左蓝月_一段
					if (Distance < 5) {
						Special = true;
						return true;
					}
					Sleep(Direction * 15);
					Special = false;
					return true;
				}
			}
			if (Distance <= 16.0f) {
				if (LogicName(ActionName, L"sw_attack_light_copy_03_02")) {//长剑左蓝月_二段
					if (Distance < 5) {
						Special = true;
						return true;
					}
					Sleep(Direction * 15);
					Special = false;
					return true;
				}
			}
			if (Distance <= 16.0f) {
				if (LogicName(ActionName, L"sw_attack_heavy_copy_03")) {//长剑右蓝月_一段
					if (Distance < 5) {
						Special = true;
						return true;
					}
					Sleep(Direction * 15);
					Special = false;
					return true;
				}
			}
			if (Distance <= 16.0f) {
				if (LogicName(ActionName, L"sw_attack_heavy_copy_03_02")) {//长剑右蓝月_二段
					if (Distance < 5) {
						Special = true;
						return true;
					}
					Sleep(Direction * 15);
					Special = false;
					return true;
				}
			}
			if (Distance <= 11.0f) {
				if (LogicName(ActionName, L"sw_attack_hold_light_01")) {//长剑左蓄力_一段
					if (Distance < 5) {
						Special = true;
						return true;
					}
					Sleep(Direction * 10);
					Special = false;
					return true;
				}
			}
			if (Distance <= 11.0f) {
				if (LogicName(ActionName, L"sw_attack_hold_heavy_01")) {//长剑右蓄力_一段
					if (Distance < 5) {
						Special = true;
						return true;
					}
					Sleep(Direction * 10);
					Special = false;
					return true;
				}
			}
			if (Distance <= 11.0f) {
				if (LogicName(ActionName, L"sw_attack_hold_light_02")) {//长剑左蓄力_二段
					if (Distance < 5) {
						Special = true;
						return true;
					}
					Sleep(Direction * 10);
					Special = false;
					return true;
				}
			}
			if (Distance <= 11.0f) {
				if (LogicName(ActionName, L"sw_attack_hold_heavy_02")) {//长剑右蓄力_二段
					if (Distance < 5) {
						Special = true;
						return true;
					}
					Sleep(Direction * 10);
					Special = false;
					return true;
				}
			}
			if (Distance <= 12.0f) {
				if (LogicName(ActionName, L"sw_attack_hold_light_soul_01")) {//长剑左凤凰羽_一段
					if (Distance < 5) {
						Special = true;
						return true;
					}
					Sleep(Direction * 10);
					Special = false;
					return true;
				}
			}
			if (Distance <= 12.0f) {
				if (LogicName(ActionName, L"sw_attack_hold_light_soul_02")) {//长剑左凤凰羽_二段
					if (Distance < 5) {
						Special = true;
						return true;
					}
					Sleep(Direction * 10);
					Special = false;
					return true;
				}
			}
			if (Distance <= 12.0f) {
				if (LogicName(ActionName, L"sw_attack_hold_heavy_soul_01")) {//长剑右凤凰羽_一段
					if (Distance < 5) {
						Special = true;
						return true;
					}
					Sleep(Direction * 10);
					Special = false;
					return true;
				}
			}
			if (Distance <= 12.0f) {
				if (LogicName(ActionName, L"sw_attack_hold_heavy_soul_02")) {//长剑右凤凰羽_二段
					if (Distance < 5) {
						Special = true;
						return true;
					}
					Sleep(Direction * 10);
					Special = false;
					return true;
				}
			}
			if (Distance <= 12.0f) {
				if (LogicName(ActionName, L"sw_attack_light_soul_04")) {//长剑七星夺窍
					Special = true;
					return true;
				}
			}
			return false;
		}
		if (WeaponId == 102 && (Status == 20 || Status == 22) && Direction <= AttackRange / 1.8f) {//太刀
			if (Distance <= 5.0f) {
				if (LogicName(ActionName, L"katana_attack_light_03")) {//太刀左三连
					Special = false;
					return true;
				}
			}
			if (Distance <= 5.0f) {
				if (LogicName(ActionName, L"katana_attack_heavy_03")) {//太刀右三连
					Special = false;
					return true;
				}
			}
			if (Distance <= 9.0f) {
				if (LogicName(ActionName, L"katana_attack_light_soul_03")) {//太刀左键三连_青鬼
					Special = false;
					return true;
				}
			}
			if (Distance <= 6.0f) {
				if (LogicName(ActionName, L"katana_attack_heavy_soul_03")) {//太刀右键三连_青鬼
					Special = false;
					return true;
				}
			}
			if (Distance <= 6.0f) {
				if (LogicName(ActionName, L"katana_attack_hold_light_01")) {//太刀左键蓄力1段
					Special = true;
					return true;
				}
			}
			if (Distance <= 6.0f) {
				if (LogicName(ActionName, L"katana_attack_hold_light_02")) {//太刀左键蓄力2段
					Special = true;
					return true;
				}
			}
			if (Distance <= 6.0f) {
				if (LogicName(ActionName, L"katana_attack_hold_heavy_01")) {//太刀右键蓄力1段
					Special = true;
					return true;
				}
			}
			if (Distance <= 6.0f) {
				if (LogicName(ActionName, L"katana_attack_hold_heavy_02")) {//太刀右键蓄力2段
					Special = true;
					return true;
				}
			}
			if (Distance <= 8.0f) {
				if (LogicName(ActionName, L"katana_attack_light_05")) {//太刀左蓄力追击
					Special = true;
					return true;
				}
			}
			if (Distance <= 9.0f) {
				if (LogicName(ActionName, L"katana_attack_heavy_09")) {//太刀右蓄力追击
					Special = true;
					return true;
				}
			}
			if (Distance <= 15.0f) {
				if (LogicName(ActionName, L"katana_attack_heavy_soul_09")) {//太刀右键惊雷十劫
					Special = true;
					return true;
				}
			}
			return false;
		}
		if (WeaponId == 103 && (Status == 20 || Status == 22) && Direction <= AttackRange / 1.8f) {//阔刀
			if (Distance <= 6.0f) {
				if (LogicName(ActionName, L"blade_attack_light_02")) {//阔刀左键二连
					Sleep(150);
					Special = false;
					return true;
				}
			}
			if (Distance <= 5.0f) {
				if (LogicName(ActionName, L"blade_attack_heavy_02")) {//阔刀右键二连
					Sleep(150);
					Special = false;
					return true;
				}
			}
			if (Distance <= 6.0f) {
				if (LogicName(ActionName, L"blade_attack_light_06")) {//阔刀攻击_右接左
					Sleep(150);
					Special = false;
					return true;
				}
			}
			if (Distance <= 6.0f) {
				if (LogicName(ActionName, L"blade_attack_light_05")) {//阔刀攻击_右接左接左
					Sleep(150);
					Special = false;
					return true;
				}
			}
			if (Distance <= 6.0f) {
				if (LogicName(ActionName, L"blade_attack_light_07")) {//阔刀攻击_左接右接左
					Sleep(150);
					Special = false;
					return true;
				}
			}
			if (Distance <= 9.0f) {
				if (LogicName(ActionName, L"blade_attack_hold_light_01")) {//阔刀左蓄力一段
					Sleep(150);
					Special = false;
					return true;
				}
			}
			if (Distance <= 7.0f) {
				if (LogicName(ActionName, L"blade_attack_hold_light_02")) {//阔刀左蓄力二段
					Sleep(150);
					Special = false;
					return true;
				}
			}
			if (Distance <= 9.0f) {
				if (LogicName(ActionName, L"blade_attack_hold_light_soul_01")) {//阔刀左蓄力一段_烈火斩
					Sleep(150);
					Special = false;
					return true;
				}
			}
			if (Distance <= 7.0f) {
				if (LogicName(ActionName, L"blade_attack_hold_light_copy_02")) {//阔刀左蓄力二段_烈火斩
					Sleep(150);
					Special = false;
					return true;
				}
			}
			if (Distance <= 7.0f) {
				if (LogicName(ActionName, L"blade_attack_hold_light_03")) {//阔刀左蓄力三段
					Special = true;
					return true;
				}
			}

			if (Distance <= 6.0f) {
				if (LogicName(ActionName, L"blade_attack_hold_light_soul_03_1")) {//阔刀左键蓄力4段_巽风震雷刀1段
					Special = true;
					return true;
				}
			}
			if (Distance <= 6.0f) {
				if (LogicName(ActionName, L"blade_attack_hold_light_soul_03_2")) {//阔刀左键蓄力4段_巽风震雷刀2段
					Special = true;
					return true;
				}
			}
			if (Distance <= 6.0f) {
				if (LogicName(ActionName, L"blade_attack_hold_light_soul_03_3")) {//阔刀左键蓄力4段_巽风震雷刀4段
					Special = true;
					return true;
				}
			}
			if (Distance <= 6.0f) {
				if (LogicName(ActionName, L"blade_attack_hold_light_soul_03_4")) {//阔刀左键蓄力4段_巽风震雷刀4段
					Special = true;
					return true;
				}
			}
			if (Distance <= 7.0f) {
				if (LogicName(ActionName, L"blade_attack_heavy_05")) {//阔刀右蓄力一段
					if (Distance <= 4.5f) {
						Special = true;
						return true;
					}
					Sleep(Distance * 7);
					Special = false;
					return true;
				}
			}
			if (Distance <= 7.0f) {
				if (LogicName(ActionName, L"blade_attack_heavy_06")) {//阔刀右蓄力一段_滑步
					if (Distance <= 4.5f) {
						Special = true;
						return true;
					}
					Sleep(Distance * 7);
					Special = false;
					return true;
				}
			}
			if (Distance <= 7.0f) {
				if (LogicName(ActionName, L"blade_attack_heavy_05_2")) {//阔刀右蓄力二段
					if (Distance <= 4.5f) {
						Special = true;
						return true;
					}
					Sleep(Distance * 7);
					Special = false;
					return true;
				}
			}
			if (Distance <= 7.0f) {
				if (LogicName(ActionName, L"blade_attack_heavy_06_2")) {//阔刀右蓄力二段_滑步
					if (Distance <= 4.5f) {
						Special = true;
						return true;
					}
					Sleep(Distance * 7);
					Special = false;
					return true;
				}
			}
			if (Distance <= 7.0f) {
				if (LogicName(ActionName, L"blade_attack_heavy_05_3")) {//阔刀右蓄力三段
					if (Distance <= 4.5f) {
						Special = true;
						return true;
					}
					Sleep(Distance * 7);
					Special = false;
					return true;
				}
			}
			if (Distance <= 7.0f) {
				if (LogicName(ActionName, L"blade_attack_heavy_06_3")) {//阔刀右蓄力三段_滑步
					if (Distance <= 4.5f) {
						Special = true;
						return true;
					}
					Sleep(Distance * 7);
					Special = false;
					return true;
				}
			}
			if (Distance <= 7.0f) {
				if (LogicName(ActionName, L"blade_attack_heavy_copy_05")) {//阔刀右蓄力一段_裂空
					if (Distance <= 4.5f) {
						Special = true;
						return true;
					}
					Sleep(Distance * 7);
					Special = false;
					return true;
				}
			}
			if (Distance <= 7.0f) {
				if (LogicName(ActionName, L"blade_attack_heavy_copy_06")) {//阔刀右蓄力一段_裂空_滑步
					if (Distance <= 4.5f) {
						Special = true;
						return true;
					}
					Sleep(Distance * 7);
					Special = false;
					return true;
				}
			}
			if (Distance <= 7.0f) {
				if (LogicName(ActionName, L"blade_attack_heavy_copy_05_2")) {//阔刀右蓄力二段_裂空
					if (Distance <= 4.5f) {
						Special = true;
						return true;
					}
					Sleep(Distance * 7);
					Special = false;
					return true;
				}
			}
			if (Distance <= 7.0f) {
				if (LogicName(ActionName, L"blade_attack_heavy_copy_06_2")) {//阔刀右蓄力二段_裂空_滑步
					if (Distance <= 4.5f) {
						Special = true;
						return true;
					}
					Sleep(Distance * 7);
					Special = false;
					return true;
				}
			}
			if (Distance <= 7.0f) {
				if (LogicName(ActionName, L"blade_attack_heavy_copy_05_3")) {//阔刀右蓄力三段_裂空
					if (Distance <= 4.5f) {
						Special = true;
						return true;
					}
					Sleep(Distance * 7);
					Special = false;
					return true;
				}
			}
			if (Distance <= 7.0f) {
				if (LogicName(ActionName, L"blade_attack_heavy_copy_06_3")) {//阔刀右蓄力三段_裂空_滑步
					if (Distance <= 4.5f) {
						Special = true;
						return true;
					}
					Sleep(Distance * 7);
					Special = false;
					return true;
				}
			}
			if (Distance <= 6.0f) {
				if (LogicName(ActionName, L"blade_attack_heavy_03")) {//阔刀攻击_左接右
					Special = true;
					return true;
				}
			}
			if (Distance <= 6.0f) {
				if (LogicName(ActionName, L"blade_attack_heavy_04")) {//阔刀攻击_左接右接右
					Sleep(150);
					Special = false;
					return true;
				}
			}
			if (Distance <= 9.0f) {
				if (LogicName(ActionName, L"blade_attack_heavy_08")) {//阔刀攻击_左接右接右_一段
					if (Distance <= 4.5f) {
						Sleep(150);
						Special = false;
						return true;
					}
					Sleep(Distance * 3);
					Special = false;
					return true;
				}
			}
			if (Distance <= 9.0f) {
				if (LogicName(ActionName, L"blade_attack_heavy_08_2")) {//阔刀攻击_左接右接右_二段
					if (Distance <= 4.5f) {
						Sleep(150);
						Special = false;
						return true;
					}
					Sleep(Distance * 3);
					Special = false;
					return true;
				}
			}
			if (Distance <= 9.0f) {
				if (LogicName(ActionName, L"blade_attack_heavy_08_3")) {//阔刀攻击_左接右接右_三段
					if (Distance <= 4.5f) {
						Sleep(150);
						Special = false;
						return true;
					}
					Sleep(Distance * 3);
					Special = false;
					return true;
				}
			}
			if (Distance <= 9.0f) {
				if (LogicName(ActionName, L"blade_attack_heavy_copy_08")) {//阔刀攻击_左接右接右_一段_裂空
					if (Distance <= 4.5f) {
						Sleep(150);
						Special = false;
						return true;
					}
					Sleep(Distance * 3);
					Special = false;
					return true;
				}
			}
			if (Distance <= 9.0f) {
				if (LogicName(ActionName, L"blade_attack_heavy_copy_08_2")) {//阔刀攻击_左接右接右_二段_裂空
					if (Distance <= 4.5f) {
						Sleep(150);
						Special = false;
						return true;
					}
					Sleep(Distance * 3);
					Special = false;
					return true;
				}
			}
			if (Distance <= 9.0f) {
				if (LogicName(ActionName, L"blade_attack_heavy_copy_08_3")) {//阔刀攻击_左接右接右_三段_裂空
					if (Distance <= 4.5f) {
						Sleep(150);
						Special = false;
						return true;
					}
					Sleep(Distance * 3);
					Special = false;
					return true;
				}
			}
			if (Distance <= 9.0f) {
				if (LogicName(ActionName, L"blade_attack_heavy_07")) {//阔刀攻击_左接右接左接右_一段
					if (Distance <= 4.5f) {
						Sleep(150);
						Special = false;
						return true;
					}
					Sleep(Distance * 3);
					Special = false;
					return true;
				}
			}
			if (Distance <= 9.0f) {
				if (LogicName(ActionName, L"blade_attack_heavy_07_2")) {//阔刀攻击_左接右接左接右_二段
					if (Distance <= 4.5f) {
						Sleep(150);
						Special = false;
						return true;
					}
					Sleep(Distance * 3);
					Special = false;
					return true;
				}
			}
			if (Distance <= 9.0f) {
				if (LogicName(ActionName, L"blade_attack_heavy_07_3")) {//阔刀攻击_左接右接左接右_三段
					if (Distance <= 4.5f) {
						Sleep(150);
						Special = false;
						return true;
					}
					Sleep(Distance * 3);
					Special = false;
					return true;
				}
			}
			if (Distance <= 9.0f) {
				if (LogicName(ActionName, L"blade_attack_heavy_copy_07")) {//阔刀攻击_左接右接左接右_一段_裂空
					if (Distance <= 4.5f) {
						Sleep(150);
						Special = false;
						return true;
					}
					Sleep(Distance * 3);
					Special = false;
					return true;
				}
			}
			if (Distance <= 9.0f) {
				if (LogicName(ActionName, L"blade_attack_heavy_copy_07_2")) {//阔刀攻击_左接右接左接右_二段_裂空
					if (Distance <= 4.5f) {
						Sleep(150);
						Special = false;
						return true;
					}
					Sleep(Distance * 3);
					Special = false;
					return true;
				}
			}
			if (Distance <= 9.0f) {
				if (LogicName(ActionName, L"blade_attack_heavy_copy_07_3")) {//阔刀攻击_左接右接左接右_三段_裂空
					if (Distance <= 4.5f) {
						Sleep(150);
						Special = false;
						return true;
					}
					Sleep(Distance * 3);
					Special = false;
					return true;
				}
			}
			if (Distance <= 8.0f) {
				if (LogicName(ActionName, L"blade_attack_heavy_copy_07_3")) {//阔刀斩马刀同源蓄力
					Special = true;
					return true;
				}
			}
			if (Distance <= 8.0f) {
				if (LogicName(ActionName, L"blade_origin_attack_hold_light_01")) {//阔刀斩马刀同源蓄力
					Special = true;
					return true;
				}
			}
			return false;
		}
		if (WeaponId == 105 && (Status == 20 || Status == 22) && Direction <= AttackRange / 1.8f) {//长枪
			if (Distance <= 6.0f) {
				if (LogicName(ActionName, L"spear_attack_light_03")) {//长枪左三连
					Special = false;
					return true;
				}
			}
			if (Distance <= 6.0f) {
				if (LogicName(ActionName, L"spear_attack_light_soul_03")) {//长枪左三连_双环扫
					Special = false;
					return true;
				}
			}
			if (Distance <= 7.0f) {
				if (LogicName(ActionName, L"spear_attack_heavy_03")) {//长枪右三连
					Sleep(180);
					Special = false;
					return true;
				}
			}
			if (Distance <= 7.0f) {
				if (LogicName(ActionName, L"spear_attack_heavy_soul_03")) {//长枪右三连_风卷云残
					Sleep(580);
					Special = false;
					return true;
				}
			}
			if (Distance <= 6.0f) {
				if (LogicName(ActionName, L"spear_attack_hold_light_01")) {//长枪左蓄力
					Special = false;
					return true;
				}
			}
			if (Distance <= 7.0f) {
				if (LogicName(ActionName, L"spear_attack_hold_heavy_01")) {//长枪左蓄力
					Special = false;
					return true;
				}
			}
			if (Distance <= 6.0f) {
				if (LogicName(ActionName, L"spear_attack_hold_heavy_soul_01")) {//长枪右蓄力_大圣游
					Special = false;
					return true;
				}
			}
			if (Distance <= 11.0f) {
				if (LogicName(ActionName, L"spear_attack_hold_light_03")) {//长枪左龙王
					if (Distance <= 6.0f) {
						Special = false;
						return true;
					}
					Sleep(Distance * 3);
					Special = false;
					return true;
				}
			}
			if (Distance <= 11.0f) {
				if (LogicName(ActionName, L"spear_attack_hold_heavy_03")) {//长枪右龙王
					if (Distance <= 6.0f) {
						Special = false;
						return true;
					}
					Sleep(Distance * 3);
					Special = false;
					return true;
				}
			}
			if (Distance <= 13.0f) {
				if (LogicName(ActionName, L"spear_attack_hold_light_03_soul_01")) {//长枪左_风火穿心脚一段
					if (Distance <= 6.0f) {
						Special = false;
						return true;
					}
					Sleep(Distance * 3);
					Special = false;
					return true;
				}
			}
			if (Distance <= 13.0f) {
				if (LogicName(ActionName, L"spear_attack_hold_light_03_soul_02")) {//长枪左_风火穿心脚二段
					if (Distance <= 6.0f) {
						Special = false;
						return true;
					}
					Sleep(Distance * 3);
					Special = false;
					return true;
				}
			}
			if (Distance <= 13.0f) {
				if (LogicName(ActionName, L"spear_attack_hold_heavy_03_soul_01")) {//长枪左_风火穿心脚一段
					if (Distance <= 6.0f) {
						Special = false;
						return true;
					}
					Sleep(Distance * 3);
					Special = false;
					return true;
				}
			}
			if (Distance <= 13.0f) {
				if (LogicName(ActionName, L"spear_attack_hold_heavy_03_soul_02")) {//长枪左_风火穿心脚二段
					if (Distance <= 6.0f) {
						Special = false;
						return true;
					}
					Sleep(Distance * 3);
					Special = false;
					return true;
				}
			}
			if (Distance <= 8.0f) {
				if (LogicName(ActionName, L"spear_origin_attack_hold_light_03")) {//长枪长棍同源技能
					Special = true;
					return true;
				}
			}
			if (Distance <= 8.0f) {
				if (LogicName(ActionName, L"spear_origin_attack_hold_light_02")) {//长枪长棍同源技能
					Special = true;
					return true;
				}
			}
			if (Distance <= 8.0f) {
				if (LogicName(ActionName, L"spear_origin_attack_hold_light_01")) {//长枪长棍同源技能
					Special = true;
					return true;
				}
			}
			if (Distance <= 8.0f) {
				if (LogicName(ActionName, L"spear_attack_hold_light_soul_01")) {//长枪六合枪
					Special = true;
					return true;
				}
			}
			return false;
		}
		if (WeaponId == 106 && (Status == 20 || Status == 22) && Direction <= AttackRange / 1.8f) {//匕首
			if (Distance <= 6.0f) {
				if (LogicName(ActionName, L"dagger_attack_light_03")) {//匕首左三连
					Special = false;
					return true;
				}
			}
			if (Distance <= 6.0f) {
				if (LogicName(ActionName, L"dagger_attack_heavy_03")) {//匕首右三连
					Special = false;
					return true;
				}
			}
			if (Distance <= 6.0f) {
				if (LogicName(ActionName, L"dagger_flashstep_attack_light_01")) {//匕首鬼反断
					Special = false;
					return true;
				}
			}
			if (Distance <= 7.0f) {
				if (LogicName(ActionName, L"dagger_attack_hold_light_01")) {//匕首左键蓄力
					Special = true;
					return true;
				}
			}
			if (Distance <= 5.0f) {
				if (LogicName(ActionName, L"dagger_attack_hold_heavy_02")) {//匕首右键蓄力
					Special = true;
					return true;
				}
			}
			if (Distance <= 9.0f) {
				if (LogicName(ActionName, L"dagger_attack_hold_heavy_soul_01")) {//匕首右键蓄力_鬼刃暗扎
					Special = true;
					return true;
				}
			}
			if (Distance <= 9.0f) {
				if (LogicName(ActionName, L"dagger_flashjump_attack_soul_01")) {//匕首鬼哭神嚎
					Special = true;
					return true;
				}
			}
			if (Distance <= 8.0f) {
				if (LogicName(ActionName, L"dagger_attack_hold_heavy_01")) {//匕首闪步左键_荆轲献匕
					Special = true;
					return true;
				}
			}
			if (Distance <= 8.0f) {
				if (LogicName(ActionName, L"dagger_attack_hold_light_soul_02")) {//匕首亢龙有悔
					Special = true;
					return true;
				}
			}
			return false;
		}
		if (WeaponId == 116 && (Status == 20 || Status == 22) && Direction <= AttackRange / 1.8f) {//双截棍
			if (Distance <= 6.0f) {
				if (LogicName(ActionName, L"nunchucks_attack_light_03")) {//双截棍左三连
					Special = false;
					return true;
				}
			}
			if (Distance <= 6.0f) {
				if (LogicName(ActionName, L"nunchucks_attack_heavy_03")) {//双截棍右三连
					Special = false;
					return true;
				}
			}
			if (Distance <= 7.0f) {
				if (LogicName(ActionName, L"nunchucks_attack_light_03_soul_01")) {//双截棍左三连_飞踢
					Special = false;
					return true;
				}
			}
			if (Distance <= 7.0f) {
				if (LogicName(ActionName, L"nunchucks_attack_hold_light_01")) {//双截棍左蓄力
					Special = true;
					return true;
				}
			}
			if (Distance <= 6.0f) {
				if (LogicName(ActionName, L"nunchucks_attack_heavy_enhance_01")) {//双截棍右蓄力
					Special = true;
					return true;
				}
			}
			if (Distance <= 6.0f) {
				if (LogicName(ActionName, L"nunchucks_attack_hold_light_soul_01")) {//双截棍左蓄扬鞭劲
					Special = true;
					return true;
				}
			}
			if (Distance <= 9.0f) {
				if (LogicName(ActionName, L"nunchucks_attack_hold_heavy_01")) {//双截棍龙虎功
					Special = true;
					return true;
				}
			}
			return false;
		}
		if (WeaponId == 118 && (Status == 20 || Status == 22) && Direction <= AttackRange / 1.8f) {//双刀
			if (Distance <= 6.0f) {
				if (LogicName(ActionName, L"twinblades_attack_light_03")) {//双刀左普攻三连
					Special = false;
					return true;
				}
			}
			if (Distance <= 6.0f) {
				if (LogicName(ActionName, L"twinblades_attack_heavy_03")) {//双刀右普攻三连
					Special = false;
					return true;
				}
			}
			if (Distance <= 5.0f) {
				if (LogicName(ActionName, L"twinblades_attack_light_03_soul")) {//双刀魂玉分水斩
					Special = true;
					return true;
				}
			}
			if (Distance <= 5.0f) {
				if (LogicName(ActionName, L"twinblades_attack_hold_light_01")) {//双刀左蓄力
					Special = true;
					return true;
				}
			}
			if (Distance <= 5.0f) {
				if (LogicName(ActionName, L"twinblades_attack_hold_light_01_soul")) {//双刀魂玉铁马残红
					Special = true;
					return true;
				}
			}
			if (Distance <= 5.0f) {
				if (LogicName(ActionName, L"twinblades_attack_hold_heavy_01")) {//双刀右蓄力一段
					Special = true;
					return true;
				}
			}
			if (Distance <= 5.0f) {
				if (LogicName(ActionName, L"twinblades_attack_hold_heavy_02")) {//双刀右蓄力二段
					Special = true;
					return true;
				}
			}
			if (Distance <= 6.0f) {
				if (LogicName(ActionName, L"twinblades_attack_heavy_04")) {//双刀蓄力右下批
					Special = false;
					return true;
				}
			}
			if (Distance <= 6.0f) {
				if (LogicName(ActionName, L"twinblades_attack_heavy_06")) {//双刀蓄力右下批
					Special = false;
					return true;
				}
			}
			if (Distance <= 15.0f) {
				if (LogicName(ActionName, L"twinblades_attack_hold_heavy_01_soul")) {//双刀魂玉乾坤日月斩
					Special = true;
					return true;
				}
			}
			if (Distance <= 6.0f) {
				if (LogicName(ActionName, L"twinblades_attack_light_soul_09")) {//双刀蓄力右武道八斩刀
					Special = true;
					return true;
				}
			}
			return false;
		}
		if (WeaponId == 120 && (Status == 20 || Status == 22) && Direction <= AttackRange / 1.8f) {//长棍
			if (Distance <= 6.0f) {
				if (LogicName(ActionName, L"rod_attack_light_03")) {//长棍左三连
					Special = false;
					return true;
				}
			}
			if (Distance <= 6.0f) {
				if (LogicName(ActionName, L"rod_attack_light_soul_03")) {//长棍左三连_双环扫
					Special = false;
					return true;
				}
			}
			if (Distance <= 7.0f) {
				if (LogicName(ActionName, L"rod_attack_heavy_03")) {//长棍右三连
					Sleep(180);
					Special = false;
					return true;
				}
			}
			if (Distance <= 8.0f) {
				if (LogicName(ActionName, L"rod_attack_hold_light_01")) {//长棍左蓄力
					Special = true;
					return true;
				}
			}
			if (Distance <= 8.0f) {
				if (LogicName(ActionName, L"rod_attack_hold_light_soul_01")) {//长棍左蓄劲捅
					Special = true;
					return true;
				}
			}
			if (Distance <= 8.0f) {
				if (LogicName(ActionName, L"rod_attack_hold_heavy_01")) {//长棍右蓄力
					Special = true;
					return true;
				}
			}
			if (Distance <= 9.0f) {
				if (LogicName(ActionName, L"rod_attack_hold_light_03")) {//长棍左右龙王
					Special = true;
					return true;
				}
			}
			if (Distance <= 8.0f) {
				if (LogicName(ActionName, L"rod_attack_heavy_05_copy")) {//长棍龙王接右键
					Special = true;
					return true;
				}
			}
			if (Distance <= 7.0f) {
				if (LogicName(ActionName, L"rod_attack_heavy_soul_05")) {//长棍五情七灭阵
					Special = true;
					return true;
				}
			}
			if (Distance <= 6.0f) {
				if (LogicName(ActionName, L"rod_attack_hold_light_03_soul_01")) {//长棍定海针·镇天撑地
					Special = true;
					return true;
				}
			}
			if (Distance <= 8.0f) {
				if (LogicName(ActionName, L"rod_attack_light_soul_05")) {//长棍捣海棍
					Special = true;
					return true;
				}
			}
			if (Distance <= 8.0f) {
				if (LogicName(ActionName, L"rod_origin_attack_hold_light_03")) {//长棍长枪同源技能
					Special = true;
					return true;
				}
			}
			if (Distance <= 8.0f) {
				if (LogicName(ActionName, L"rod_origin_attack_hold_light_02")) {//长棍长枪同源技能
					Special = true;
					return true;
				}
			}
			if (Distance <= 8.0f) {
				if (LogicName(ActionName, L"rod_origin_attack_hold_light_01")) {//长棍长枪同源技能
					Special = true;
					return true;
				}
			}
			if (Distance <= 6.0f) {
				if (LogicName(ActionName, L"rod_attack_heavy_soul_03")) {//长棍少林棍
					Special = false;
					return true;
				}
			}
			return false;
		}
		if (WeaponId == 119 && (Status == 20 || Status == 22) && Direction <= AttackRange / 1.8f) {//斩马刀
			if (Distance <= 6.0f) {
				if (LogicName(ActionName, L"saber_attack_light_03")) {//斩马刀左键柄击
					Special = true;
					return true;
				}
			}
			if (Distance <= 6.0f) {
				if (LogicName(ActionName, L"saber_attack_light_02")) {//斩马刀左键二连
					Sleep(150);
					Special = false;
					return true;
				}
			}
			if (Distance <= 5.0f) {
				if (LogicName(ActionName, L"saber_attack_heavy_02")) {//斩马刀右键二连
					Sleep(150);
					Special = false;
					return true;
				}
			}
			if (Distance <= 6.0f) {
				if (LogicName(ActionName, L"saber_attack_light_06")) {//斩马刀攻击_右接左
					Sleep(150);
					Special = false;
					return true;
				}
			}
			if (Distance <= 6.0f) {
				if (LogicName(ActionName, L"saber_attack_light_05")) {//斩马刀攻击_右接左接左
					Sleep(150);
					Special = false;
					return true;
				}
			}
			if (Distance <= 6.0f) {
				if (LogicName(ActionName, L"saber_attack_light_07")) {//斩马刀攻击_左接右接左
					Sleep(150);
					Special = false;
					return true;
				}
			}
			if (Distance <= 9.0f) {
				if (LogicName(ActionName, L"saber_attack_hold_light_01")) {//斩马刀左蓄力一段
					if (Distance <= 5.0f) {
						Special = true;
						return true;
					}
					Sleep(Distance * 25);
					Special = false;
					return true;
				}
			}
			if (Distance <= 6.0f) {
				if (LogicName(ActionName, L"saber_attack_hold_light_02")) {//斩马刀左蓄力二段
					if (Distance <= 5.0f) {
						Special = true;
						return true;
					}
					Sleep(Distance * 25);
					Special = false;
					return true;
				}
			}
			if (Distance <= 6.0f) {
				if (LogicName(ActionName, L"saber_attack_hold_light_03")) {//斩马刀左蓄力三段
					if (Distance <= 5.0f) {
						Special = true;
						return true;
					}
					Sleep(Distance * 25);
					Special = false;
					return true;
				}
			}
			if (Distance <= 7.0f) {
				if (LogicName(ActionName, L"saber_attack_hold_heavy_01")) {//斩马刀右蓄力一段
					Special = false;
					return true;
				}
			}
			if (Distance <= 7.0f) {
				if (LogicName(ActionName, L"saber_attack_hold_heavy_02")) {//斩马刀右蓄力二段
					Special = false;
					return true;
				}
			}
			if (Distance <= 7.0f) {
				if (LogicName(ActionName, L"saber_attack_hold_heavy_03")) {//斩马刀右蓄力三段
					Special = false;
					return true;
				}
			}
			if (Distance <= 6.0f) {
				if (LogicName(ActionName, L"saber_attack_heavy_09")) {//斩马刀右键蓄力接右
					Special = false;
					return true;
				}
			}
			if (Distance <= 6.0f) {
				if (LogicName(ActionName, L"saber_attack_heavy_03")) {//斩马刀攻击_左接右
					Special = true;
					return true;
				}
			}
			if (Distance <= 6.0f) {
				if (LogicName(ActionName, L"saber_attack_heavy_04")) {//斩马刀攻击_左接右接右
					Sleep(150);
					Special = false;
					return true;
				}
			}
			if (Distance <= 7.0f) {
				if (LogicName(ActionName, L"saber_attack_heavy_08")) {//斩马刀攻击_左接右接右_一段
					Special = false;
					return true;
				}
			}
			if (Distance <= 7.0f) {
				if (LogicName(ActionName, L"saber_attack_heavy_08_2")) {//斩马刀攻击_左接右接右_二段
					Special = false;
					return true;
				}
			}
			if (Distance <= 7.0f) {
				if (LogicName(ActionName, L"saber_attack_heavy_08_3")) {//斩马刀攻击_左接右接右_三段
					Special = false;
					return true;
				}
			}
			if (Distance <= 7.0f) {
				if (LogicName(ActionName, L"saber_attack_hold_light_soul_01")) {//斩马刀左蓄力炽焰斩一段
					Special = true;
					return true;
				}
			}
			if (Distance <= 7.0f) {
				if (LogicName(ActionName, L"saber_attack_hold_light_soul_02")) {//斩马刀左蓄力炽焰斩二段
					Special = true;
					return true;
				}
			}
			if (Distance <= 7.0f) {
				if (LogicName(ActionName, L"saber_attack_hold_light_soul_03")) {//斩马刀左蓄力炽焰斩三段
					Special = true;
					return true;
				}
			}
			if (Distance <= 7.0f) {
				if (LogicName(ActionName, L"saber_attack_hold_heavy_soul_03")) {//斩马刀左键蓄力过关斩将
					Special = true;
					return true;
				}
			}
			if (Distance <= 8.0f) {
				if (LogicName(ActionName, L"saber_origin_attack_light_01")) {//阔刀斩马刀同源蓄力
					Special = true;
					return true;
				}
			}
			if (Distance <= 8.0f) {
				if (LogicName(ActionName, L"saber_origin_attack_hold_light_01")) {//阔刀斩马刀同源蓄力
					Special = true;
					return true;
				}
			}
			return false;
		}
		if (WeaponId == 121 && (Status == 20 || Status == 22) && Direction <= AttackRange / 1.8f) {//双戟
			if (Distance <= 6.0f) {
				if (LogicName(ActionName, L"dualhalberd_attack_light_03")) {//双戟左三连
					Special = false;
					return true;
				}
			}
			if (Distance <= 6.0f) {
				if (LogicName(ActionName, L"dualhalberd_attack_heavy_03")) {//双戟右三连
					Special = false;
					return true;
				}
			}
			if (Distance <= 6.0f) {
				if (LogicName(ActionName, L"dualhalberd_attack_hold_light_01")) {//双戟左蓄力
					Special = true;
					return true;
				}
			}
			if (Distance <= 6.0f) {
				if (LogicName(ActionName, L"dualhalberd_attack_hold_heavy_01")) {//双戟右蓄力一段
					Special = true;
					return true;
				}
			}
			if (Distance <= 6.0f) {
				if (LogicName(ActionName, L"dualhalberd_attack_hold_heavy_02")) {//双戟右蓄力二段
					Special = true;
					return true;
				}
			}
			if (Distance <= 6.0f) {
				if (LogicName(ActionName, L"dualhalberd_attack_heavy_09")) {//双戟左蓄力接右
					Special = true;
					return true;
				}
			}
			if (Distance <= 6.0f) {
				if (LogicName(ActionName, L"dualhalberd_attack_heavy_soul_09")) {//双戟左蓄力接右蛟龙入海
					Special = true;
					return true;
				}
			}
			if (Distance <= 6.0f) {
				if (LogicName(ActionName, L"dualhalberd_attack_light_09")) {//双戟左蓄力接右接左
					Special = true;
					return true;
				}
			}
			if (Distance <= 6.0f) {
				if (LogicName(ActionName, L"dualhalberd_attack_light_08")) {//双戟右三连勾旋斩
					Sleep(130);
					Special = false;
					return true;
				}
			}
			if (Distance <= 6.0f) {
				if (LogicName(ActionName, L"dualhalberd_attack_hold_light_01_soul_copy")) {//双戟左蓄力战龙在天
					Special = true;
					return true;
				}
			}
			if (Distance <= 12.0f) {
				if (LogicName(ActionName, L"dualhalberd_attack_hold_light_soul_02")) {//武道·形意钩
					Special = true;
					return true;
				}
			}
			return false;
		}
		if (WeaponId == 122 && (Status == 20 || Status == 22) && Direction <= AttackRange / 1.8f) {//扇
			if (Distance <= 6.0f) {
				if (LogicName(ActionName, L"fan_attack_light_03")) {//扇-左三连
					Special = false;
					return true;
				}
			}
			if (Distance <= 6.0f) {
				if (LogicName(ActionName, L"fan_attack_heavy_03")) {//扇-右三连
					Special = false;
					return true;
				}
			}
			if (Distance <= 8.0f) {
				if (LogicName(ActionName, L"fan_attack_hold_light_01")) {//扇-左蓄力
					Special = true;
					return true;
				}
			}
			if (Distance <= 7.0f) {
				if (LogicName(ActionName, L"fan_attack_hold_heavy_01")) {//扇-右蓄力
					Special = true;
					return true;
				}
			}
			if (Distance <= 6.0f) {
				if (LogicName(ActionName, L"fan_attack_light_04")) {//扇-左蓄力接左
					Special = false;
					return true;
				}
			}
			if (Distance <= 6.0f) {
				if (LogicName(ActionName, L"fan_attack_hold_heavy_soul_01")) {//扇-右蓄力-缠龙奔野
					if (Distance <= 5.0f) {
						Special = true;
						return true;
					}
					Sleep(Distance * 50);
					Special = false;
					return true;
				}
			}
			if (Distance <= 5.0f) {
				if (LogicName(ActionName, L"fan_attack_light_soul_03")) {//扇-左三连-双开圆
					Special = false;
					return true;
				}
			}

			if (Distance <= 6.0f) {
				if (LogicName(ActionName, L"fan_attack_hold_light_soul_01")) {//扇-左蓄力-三风摆
					Special = true;
					return true;
				}
			}
			return false;
		}
		if (WeaponId == 123 && (Status == 20 || Status == 22) && Direction <= AttackRange / 1.8f) {//横刀
			if (Distance <= 5.0f) {
				if (LogicName(ActionName, L"hengdao_attack_light_03")) {//横刀-左三连
					Special = false;
					return true;
				}
			}
			if (Distance <= 5.0f) {
				if (LogicName(ActionName, L"hengdao_attack_heavy_03")) {//横刀-右三连
					Special = false;
					return true;
				}
			}
			if (Distance <= 10.0f) {
				if (LogicName(ActionName, L"hengdao_attack_hold_light_01")) {//横刀-左蓄力
					for (uint64_t i = 0; i < 152; i++) {
						if (Data::GetActionId(PlayerAddress) == 106 || Data::GetActionId(PlayerAddress) == 209) {
							Special = false;
							return false;
						}
						Sleep(1);
					}
					Special = true;
					return true;
				}
			}
			if (Distance <= 10.0f) {
				if (LogicName(ActionName, L"hengdao_attack_light_05")) {//横刀-右蓄力接左
					for (uint64_t i = 0; i < 152; i++) {
						if (Data::GetActionId(PlayerAddress) == 106 || Data::GetActionId(PlayerAddress) == 209) {
							Special = false;
							return false;
						}
						Sleep(1);
					}
					Special = true;
					return true;
				}
			}
			if (Distance <= 5.0f) {
				if (LogicName(ActionName, L"hengdao_attack_heavy_03")) {//横刀-左蓄力接右
					Special = true;
					return true;
				}
			}
			if (Distance <= 6.0f) {
				if (LogicName(ActionName, L"hengdao_attack_hold_heavy_01")) {//横刀-右蓄力一段
					Special = true;
					return true;
				}
			}
			if (Distance <= 6.0f) {
				if (LogicName(ActionName, L"hengdao_attack_hold_heavy_02")) {//横刀-右蓄力二段
					Special = true;
					return true;
				}
			}
			if (Distance <= 7.0f) {
				if (LogicName(ActionName, L"hengdao_attack_hold_heavy_soul_02")) {//横刀-右蓄力极光碎云闪
					Special = true;
					return true;
				}
			}
			if (Distance <= 10.0f) {
				if (LogicName(ActionName, L"hengdao_attack_hold_light_soul_01")) {//横刀-破千军
					Special = true;
					return true;
				}
			}
			if (Distance <= 10.0f) {
				if (LogicName(ActionName, L"hengdao_attack_light_soul_05")) {//横刀-破千军
					Special = true;
					return true;
				}
			}
			return false;
		}
		if (WeaponId == 124 && (Status == 20 || Status == 22) && Direction <= AttackRange / 1.8f) {
			if (Distance <= 5.0f) {
				if (LogicName(ActionName, L"punch_attack_light_03")) {//爪子-左三连
					Special = false;
					return true;
				}
			}
			if (Distance <= 5.0f) {
				if (LogicName(ActionName, L"punch_attack_heavy_03")) {//爪子-右三连
					Special = false;
					return true;
				}
			}
			if (Distance <= 6.0f) {
				if (LogicName(ActionName, L"punch_attack_hold_light_01")) {//爪子-左一段
					Special = false;
					return true;
				}
			}
			if (Distance <= 6.0f) {
				if (LogicName(ActionName, L"punch_attack_hold_light_02")) {//爪子-左2段
					Special = false;
					return true;
				}
			}
			if (Distance <= 6.0f) {
				if (LogicName(ActionName, L"punch_attack_hold_light_03")) {//爪子-左3段
					Special = false;
					return true;
				}
			}
			if (Distance <= 6.0f) {
				if (LogicName(ActionName, L"punch_attack_hold_heavy_01")) {//爪子-右1段
					Special = false;
					return true;
				}
			}
			if (Distance <= 6.0f) {
				if (LogicName(ActionName, L"punch_attack_hold_heavy_02")) {//爪子-右2段
					Special = false;
					return true;
				}
			}
			if (Distance <= 6.0f) {
				if (LogicName(ActionName, L"punch_attack_hold_heavy_03")) {//爪子-右3段
					Special = false;
					return true;
				}
			}
			if (Distance <= 8.0f) {
				if (LogicName(ActionName, L"punch_attack_light_soul_03")) {//爪子苍牙
					Special = false;
					return true;
				}
			}
			if (Distance <= 7.0f) {
				if (LogicName(ActionName, L"male_punch_attack_hold_light_soul_03")) {//爪子太极
					Special = false;
					return true;
				}
			}
			return true;

		}
		return false;
	}

	BOOL LogicYpZhenDao(uint32_t WeaponId, uint32_t Status, wstring ActionName, float Distance, float Direction, float ChargeTime) {
		if (Status != 20 && Status != 22) {//都不在蓄力判断鸡毛
			return false;
		}
		if (!Judg::JudgeYpAction(ActionName)) {//判断是不是预判的招式
			return false;
		}
		if (WeaponId == 116) {//双截棍
			if (Distance <= 9.0f && Direction <= 60.0f) {
				if (ChargeTime > 1.14f) {
					return true;
				}
			}
		}
		if (WeaponId == 102) {//太刀
			if (Distance <= 9.0f && Direction <= 60.0f) {
				if (ChargeTime > 0.8f) {
					return true;
				}
			}
		}
		if (WeaponId == 122) {//扇
			if (Distance <= 7.0f && Direction <= 60.0f) {
				return true;
			}
		}
		return false;
	}
}

namespace Bone {
	//取骨骼坐标
	Vector3 CalcBonePosition(uint64_t Transform) {

		Vector3 ret_value;
		uint64_t matrix_list_base = 0;
		uint64_t dependency_index_table_base = 0;
		uint64_t transform_internal = Transform;
		uint64_t matrices = 0;

		matrices = Memory::ReadMemory<uint64_t>(transform_internal + 0x38);
		if (matrices == 0)
			return ret_value;
		int index = 0;
		index = Memory::ReadMemory<uint32_t>(transform_internal + 0x40);
		matrix_list_base = Memory::ReadMemory<uint64_t>(matrices + 0x18);
		if (matrix_list_base == 0)
			return ret_value;
		dependency_index_table_base = Memory::ReadMemory<uint64_t>(matrices + 0x20);
		if (dependency_index_table_base == 0)
			return ret_value;
		int index_relation = 0;
		index_relation = Memory::ReadMemory<uint32_t>(dependency_index_table_base + static_cast<uint64_t>(index) * 4);
		float base_matrix3x4[64],
			* matrix3x4_buffer0 = (float*)((uint64_t)base_matrix3x4 + 16),
			* matrix3x4_buffer1 = (float*)((uint64_t)base_matrix3x4 + 32),
			* matrix3x4_buffer2 = (float*)((uint64_t)base_matrix3x4 + 48);
		Memory::ReadByte(matrix_list_base + static_cast<uint64_t>(index) * 0x30, 0x10, base_matrix3x4);
		__m128 xmmword_1410D1340 = { -2.f, 2.f, -2.f, 0.f };
		__m128 xmmword_1410D1350 = { 2.f, -2.f, -2.f, 0.f };
		__m128 xmmword_1410D1360 = { -2.f, -2.f, 2.f, 0.f };
		while (index_relation >= 0)
		{
			ULONG matrix_relation_index = 6 * index_relation;
			Memory::ReadByte(matrix_list_base + 8 * static_cast<uint64_t>(matrix_relation_index), 0x10, matrix3x4_buffer2);
			__m128 v_0 = *(__m128*)matrix3x4_buffer2;
			Memory::ReadByte(matrix_list_base + 8 * static_cast<uint64_t>(matrix_relation_index) + 32, 0x10, matrix3x4_buffer0);
			__m128 v_1 = *(__m128*)matrix3x4_buffer0;
			Memory::ReadByte(matrix_list_base + 8 * static_cast<uint64_t>(matrix_relation_index) + 16, 0x10, matrix3x4_buffer1);
			__m128i v9 = *(__m128i*)matrix3x4_buffer1;
			__m128* v3 = (__m128*)base_matrix3x4;
			__m128 v10, v11, v12, v13, v14, v15, v16, v17;
			v10 = _mm_mul_ps(v_1, *v3);
			v11 = _mm_castsi128_ps(_mm_shuffle_epi32(v9, 0));
			v12 = _mm_castsi128_ps(_mm_shuffle_epi32(v9, 85));
			v13 = _mm_castsi128_ps(_mm_shuffle_epi32(v9, 142));
			v14 = _mm_castsi128_ps(_mm_shuffle_epi32(v9, 219));
			v15 = _mm_castsi128_ps(_mm_shuffle_epi32(v9, 170));
			v16 = _mm_castsi128_ps(_mm_shuffle_epi32(v9, 113));
			v17 = _mm_add_ps(_mm_add_ps(_mm_add_ps(_mm_mul_ps(_mm_sub_ps(_mm_mul_ps(_mm_mul_ps(v11, xmmword_1410D1350), v13), _mm_mul_ps(_mm_mul_ps(v12, xmmword_1410D1360), v14)),
				_mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(v10), 170))), _mm_mul_ps(_mm_sub_ps(_mm_mul_ps(_mm_mul_ps(v15, xmmword_1410D1360), v14),
					_mm_mul_ps(_mm_mul_ps(v11, xmmword_1410D1340), v16)), _mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(v10), 85)))),
				_mm_add_ps(_mm_mul_ps(_mm_sub_ps(_mm_mul_ps(_mm_mul_ps(v12, xmmword_1410D1340), v16), _mm_mul_ps(_mm_mul_ps(v15, xmmword_1410D1350), v13)),
					_mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(v10), 0))), v10)), v_0);
			*v3 = v17;
			Memory::ReadByte(dependency_index_table_base + static_cast<uint64_t>(index_relation) * 4, sizeof(index_relation), &index_relation);
		}
		ret_value = *(Vector3*)base_matrix3x4;
		return ret_value;
	}

	//获取骨骼位置
	Vector3 GetBonePosition(uint64_t Entity, uint64_t index) {
		uint64_t player_model = Memory::ReadMemory<uint64_t>(Entity + 0xE0);
		if (player_model != 0) {
			uint64_t boneTransformsPtr = Memory::ReadMemory<uint64_t>(player_model + 0x50);
			uint64_t BoneTransforms = Memory::ReadMemory<uint64_t>(boneTransformsPtr + 0x18);
			if (BoneTransforms != 0) {
				uint64_t entity_bone = Memory::ReadMemory<uint64_t>(BoneTransforms + 0x30 + index * 24);
				if (entity_bone != 0) {
					uint64_t bonePositionPtr = Memory::ReadMemory<uint64_t>(entity_bone + 0x10);
					return CalcBonePosition(bonePositionPtr);
				}
			}
		}
		return Vector3{ 0.f, 0.f, 0.f };
	}
}

namespace Bone_Draw {

	enum Bone : INT {
		头部 = 67,
		脖子 = 65,
		胸部 = 29,
		盆骨 = 4,
		左肩 = 31,
		左肘 = 32,
		左手 = 37,
		右肩 = 70,
		右肘 = 71,
		右手 = 76,
		左大腿 = 14,
		左膝盖 = 11,
		左脚跟 = 7,
		左脚尖 = 9,
		右大腿 = 25,
		右膝盖 = 22,
		右脚跟 = 18,
		右脚尖 = 20,
	};
	list<INT> _上部 = { 脖子, 胸部 };
	list<INT> _右臂 = { 脖子, 右肩, 右肘, 右肘, 右手 };
	list<INT> _左臂 = { 脖子, 左肩, 左肘, 左肘, 左手 };
	list<INT> _脊柱 = { 胸部, 盆骨 };
	list<INT> _右腿 = { 盆骨, 右大腿 , 右膝盖, 右膝盖, 右脚跟, 右脚尖 };
	list<INT> _左腿 = { 盆骨, 左大腿 , 左膝盖, 左膝盖, 左脚跟, 左脚尖 };
	list<list<INT>> BoneList = { _上部, _右臂, _左臂, _脊柱, _右腿, _左腿 };
}

VOID GameUpdata(PVOID lpParameter, BOOLEAN TimerOrWaitFired) {

	vector<uint64_t> m_PlayerArray;

	while (IsWindow(g_Window.Hwnd)) {
		uint64_t m_WorldAddress = Memory::ReadMemory<uint64_t>(g_Module.UWorld);
		uint64_t m_WorldLevel1 = Memory::ReadMemory<uint64_t>(m_WorldAddress + SDK::世界一级);
		uint64_t m_WorldLevel2 = Memory::ReadMemory<uint64_t>(m_WorldLevel1 + SDK::世界二级);
		g_LocalPlayerAddress = Memory::ReadMemory<uint64_t>(m_WorldLevel2 + SDK::本人地址);
		g_MatrixAddress = Data::GetMatrixAddress(m_WorldLevel2);
		uint64_t m_CountArray = Data::GetCountArray(m_WorldAddress);
		uint32_t m_IterationCount = Memory::ReadMemory<uint32_t>(m_CountArray + SDK::遍历数量);
		uint64_t m_IterateArray = Memory::ReadMemory<uint64_t>(m_CountArray + SDK::遍历数组);
		for (uint32_t i = 0; i < m_IterationCount; i++) {
			uint64_t m_PlayerAddress = Memory::ReadMemory<uint64_t>(m_IterateArray + static_cast<uint64_t>(i) * static_cast<uint64_t>(0x8) + SDK::遍历偏移);
			if (m_PlayerAddress <= 0) {//过滤无效指针
				continue;
			}
			if (Memory::ReadMemory<uint32_t>(m_PlayerAddress + SDK::死亡偏移) != 0x101) {//过滤死亡玩家
				continue;
			}
			if (m_PlayerAddress == g_LocalPlayerAddress) {//过滤本人地址
				continue;
			}
			if (Data::GetPlayerTeam(m_PlayerAddress) == Data::GetPlayerTeam(g_LocalPlayerAddress)) {//过滤队友地址
				continue;
			}
			m_PlayerArray.push_back(m_PlayerAddress);
		}
		g_PlayerRefresh = false;
		//Sleep(1);
		g_PlayerArray.clear();
		g_PlayerArray = m_PlayerArray;
		m_PlayerArray.clear();
		//Sleep(1);
		g_PlayerRefresh = true;
		Sleep(10);
	}
}

VOID AimPredictive(Vector3 TarPos) {

	Vector2 m_AimPos;

	if (Comp::WorldToScreen2D(TarPos, m_AimPos)) {
		int AimBotsx = static_cast<int>((m_AimPos.x - Gui.Window.Size.x / 2) * 0.3f);
		int AimBotsy = static_cast<int>((m_AimPos.y - Gui.Window.Size.y / 2) * 0.3f);
		Drv.B_MouseMove(AimBotsx, AimBotsy, MoveType::Relative);
		//Drv::MoveR(AimBotsx, AimBotsy);
		Sleep(10);
	}
	return;
}

VOID ZhuiZong(Vector3 BonePos) {
	uint64_t m_ItemPointer = Memory::ReadMemory<uint64_t>(g_Module.GameAssembly + SDK::物品地址);
	m_ItemPointer = Memory::ReadMemory<uint64_t>(m_ItemPointer + 0xB8);
	m_ItemPointer = Memory::ReadMemory<uint64_t>(m_ItemPointer + 0x8);
	m_ItemPointer = Memory::ReadMemory<uint64_t>(m_ItemPointer + 0x28);
	uint32_t m_ItemCount = Memory::ReadMemory<uint32_t>(m_ItemPointer + 0x24);
	uint64_t m_InitialAddress = Memory::ReadMemory<uint64_t>(m_ItemPointer + 0x18) + 0x28;
	for (uint32_t i = 0; i < m_ItemCount; i++) {
		uint64_t m_ItemAddress = Memory::ReadMemory<uint64_t>(static_cast<uint64_t>(m_InitialAddress) + static_cast<uint64_t>(i) * 0x10);
		if (m_ItemAddress <= 0) {
			continue;
		}
		uint64_t m_ItemPointer = Memory::ReadMemory<uint64_t>(m_ItemAddress);
		if (m_ItemPointer <= 0) {
			continue;
		}
		uint64_t m_ItemType = Memory::ReadMemory<uint64_t>(m_ItemPointer + 0x10);
		char m_ItemTypeTxt[32];
		Memory::ReadByte(m_ItemType, sizeof(m_ItemTypeTxt), &m_ItemTypeTxt);
		m_ItemTypeTxt[sizeof(m_ItemTypeTxt) - 1] = '\0';
		if (!strcmp((const char*)m_ItemTypeTxt, "RangeAgentEntity")) {
			uint32_t m_ItemID = Memory::ReadMemory<uint32_t>(Memory::ReadMemory<uint64_t>(m_ItemAddress + 0x58) + 0x30);
			uint64_t m_CoordinatePointer = Memory::ReadMemory<uint64_t>(Memory::ReadMemory<uint64_t>(Memory::ReadMemory<uint64_t>(Memory::ReadMemory<uint64_t>(Memory::ReadMemory<uint64_t>(Memory::ReadMemory<uint64_t>(m_ItemAddress + 0x30) + 0x18) + 0x28) + 0x10) + 0x30) + 0x8);
			uint32_t m_CoordinateIndex = Memory::ReadMemory<uint32_t>(m_CoordinatePointer + 0x40);
			uint64_t m_CoordinatePosition = Memory::ReadMemory<uint64_t>(Memory::ReadMemory<uint64_t>(m_CoordinatePointer + 0x38) + 0x18);
			m_CoordinatePointer = m_CoordinatePosition + static_cast<uint64_t>(m_CoordinateIndex) * 0x30;
			Vector3 m_ItemPos = Data::GetItemPos(m_CoordinatePointer);

			float m_Distance = Comp::GetPlayerDistance(m_ItemPos, BonePos);
			if (m_Distance <= g_MenuInfo.ZhuiZongRadius) {
				Memory::WriteMemory<float>(m_CoordinatePointer, BonePos.x);
				Memory::WriteMemory<float>(m_CoordinatePointer + 4, BonePos.y);
				Memory::WriteMemory<float>(m_CoordinatePointer + 8, BonePos.z);
			}
		}
	}
}

/*
VOID AimBot(PVOID lpParameter, BOOLEAN TimerOrWaitFired) {

	static vector<uint64_t> m_PlayerArray;

	float m_NearestDistance = 0.0f;
	uint64_t m_AimPlayer = NULL;

	while (IsWindow(g_Window.Hwnd)) {

		if (g_PlayerRefresh) {
			uint32_t m_PlayerArraySize = static_cast<uint32_t>(g_PlayerArray.size());
			if (m_PlayerArraySize > 0) {
				m_PlayerArray.clear();
				m_PlayerArray = g_PlayerArray;
			}
			if (m_PlayerArraySize == 0) {
				m_PlayerArray.clear();
			}
		}

		uint32_t m_PlayerArraySize = static_cast<uint32_t>(m_PlayerArray.size());

		for (uint32_t i = 0; i < m_PlayerArraySize; i++) {

			uint64_t m_PlayerAddress = m_PlayerArray[i];
			uint32_t m_PlayerTeam = Data::GetPlayerTeam(m_PlayerAddress);

			// 过滤不可见玩家
			//if (Memory::ReadMemory<uint32_t>(m_PlayerAddress + SDK::盲区偏移) != 0x1) {
			//	continue;
			//}

			uint64_t m_PlayerEntity = Data::GetPlayerEntity(m_PlayerAddress);
			Vector3 m_PlayerPos = Data::GetPlayerPosition(m_PlayerEntity);
			uint64_t m_LocalPlayerEntity = Data::GetPlayerEntity(g_LocalPlayerAddress);
			Vector3 m_LocalPlayerPos = Data::GetPlayerPosition(m_LocalPlayerEntity);
			float m_Distance = Comp::GetPlayerDistance(m_LocalPlayerPos, m_PlayerPos);

			Vector4 m_ScreenPos;
			if (!Comp::WorldToScreen3D(Data::GetPosition(m_PlayerEntity), m_ScreenPos)) {
				continue;
			}

		

			uint32_t m_LocalHeroId = Data::GetHeroId(g_LocalPlayerAddress);

			if (m_LocalHeroId == 1000008 && m_Distance <= 5) {//大佛不自瞄手上与近点的敌人
				continue;
			}
			float m_CurrentDistance = Comp::GetDistance(Gui.Window.Size.x / 2, Gui.Window.Size.y / 2, m_ScreenPos.x, m_ScreenPos.y);
			if (m_CurrentDistance <= g_MenuInfo.AimRadius) {
				if (m_AimPlayer == NULL) {
					m_NearestDistance = m_CurrentDistance;
					m_AimPlayer = m_PlayerAddress;

					
				}
				else if (m_CurrentDistance <= m_NearestDistance) {
					m_NearestDistance = m_CurrentDistance;
					m_AimPlayer = m_PlayerAddress;
				}
			}



		}
		if (m_AimPlayer != NULL) {
			Vector3 m_BonePos = Bone::GetBonePosition(m_AimPlayer, 0x43);

			if (g_MenuInfo.ZhuiZong) {
				ZhuiZong(m_BonePos);
			}
			if (g_MenuInfo.Aim) {// 这里CTRL键自瞄
				if (GetAsyncKeyState(VK_CONTROL) & 0x8000) {
					AimPredictive(m_BonePos);
				}
				else {
					m_AimPlayer = NULL;
				}
			}
		}
		Sleep(1);
	}
}
*/
VOID ZhenDao(PVOID lpParameter, BOOLEAN TimerOrWaitFired) {

	static vector<uint64_t> m_PlayerArray;

	while (IsWindow(g_Window.Hwnd)) {

		if (g_PlayerRefresh) {
			uint32_t m_PlayerArraySize = static_cast<uint32_t>(g_PlayerArray.size());
			if (m_PlayerArraySize > 0) {
				m_PlayerArray.clear();
				m_PlayerArray = g_PlayerArray;
			}
			if (m_PlayerArraySize == 0) {
				m_PlayerArray.clear();
			}
		}
		uint32_t m_PlayerArraySize = static_cast<uint32_t>(m_PlayerArray.size());

		for (uint32_t i = 0; i < m_PlayerArraySize; i++) {

			uint64_t m_PlayerAddress = m_PlayerArray[i];
			uint32_t m_PlayerTeam = Data::GetPlayerTeam(m_PlayerAddress);

			// 过滤不可见玩家
			if (Memory::ReadMemory<uint32_t>(m_PlayerAddress + SDK::盲区偏移) != 0x1) {
				continue;
			}

			uint64_t m_PlayerEntity = Data::GetPlayerEntity(m_PlayerAddress);
			Vector3 m_PlayerPos = Data::GetPlayerPosition(m_PlayerEntity);
			uint64_t m_LocalPlayerEntity = Data::GetPlayerEntity(g_LocalPlayerAddress);
			Vector3 m_LocalPlayerPos = Data::GetPlayerPosition(m_LocalPlayerEntity);
			float m_Distance = Comp::GetPlayerDistance(m_LocalPlayerPos, m_PlayerPos);

			//过滤距离+高度过滤
			if (m_Distance > 16.0f || abs(m_PlayerPos.z - m_LocalPlayerPos.z) >= 3.0f) {
				continue;
			}

			//敌人朝向
			float m_PlayerA = Face::GetDirection(Data::GetRootComponent(m_PlayerEntity));
			float m_PlayerB = Face::VectorToRotationYaw(Face::FindLookAtVector(m_PlayerPos, m_LocalPlayerPos));
			float m_PlayerDirection = abs(Face::AngularDifference(m_PlayerA, m_PlayerB));

			//本人朝向
			float m_LocalPlayerA = Face::GetDirection(Data::GetRootComponent(m_LocalPlayerEntity));
			float m_LocalPlayerB = Face::VectorToRotationYaw(Face::FindLookAtVector(m_LocalPlayerPos, m_PlayerPos));
			float m_LocalPlayerDirection = abs(Face::AngularDifference(m_LocalPlayerA, m_LocalPlayerB));

			//本人数据
			uint32_t m_LocalPlayerWeaponId = Data::GetWeaponId(g_LocalPlayerAddress);
			uint32_t m_LocalPlayerStatus = Data::GetStatus(g_LocalPlayerAddress);
			wstring m_LocalPlayerActionName = Data::GetActionName(g_LocalPlayerAddress);

			//敌人数据
			uint32_t m_PlayerWeaponId = Data::GetWeaponId(m_PlayerAddress);
			uint32_t m_PlayerStatus = Data::GetStatus(m_PlayerAddress);
			float m_PlayerAttackRange = Data::GetAttackRange(m_PlayerAddress);
			float m_PlayerChargeTime = Data::GetChargeTime(m_PlayerAddress);
			wstring m_PlayerActionName = Data::GetActionName(m_PlayerAddress);

			if (!Judg::JudgeMeleeWeapon(m_LocalPlayerWeaponId) || !Judg::JudgeStatus(m_LocalPlayerStatus) || !Judg::JudgeZhenDao(m_LocalPlayerActionName)) {
				continue;
			}
			BOOL m_ZhenDaoSpecial = false;
			if (g_MenuInfo.ZhenDao && Logc::LogicNcZhenDao(m_PlayerWeaponId, m_PlayerStatus, m_PlayerActionName, m_PlayerAttackRange, m_Distance, m_PlayerDirection, m_ZhenDaoSpecial, m_PlayerAddress)) {
				//std::cout << "震刀" << "\n";
				Hook::HookZhenDao(g_MenuInfo.FangShi && !Judg::JudgeSky(m_LocalPlayerActionName), m_ZhenDaoSpecial);
				continue;
			}
			if (g_MenuInfo.ZhenDao && Logc::LogicYpZhenDao(m_PlayerWeaponId, m_PlayerStatus, m_PlayerActionName, m_Distance, m_PlayerDirection, m_PlayerChargeTime)) {
				//std::cout << "震刀" << "\n";
				Hook::HookZhenDao(g_MenuInfo.FangShi && !Judg::JudgeSky(m_LocalPlayerActionName), false);
				continue;
			}
		}
		Sleep(1);
	}
}

VOID Activation(PVOID lpParameter, BOOLEAN TimerOrWaitFired) {

	static vector<uint64_t> m_PlayerArray;

	while (IsWindow(g_Window.Hwnd)) {
		if (g_MenuInfo.FangXu) {
			if (g_PlayerRefresh) {
				uint32_t m_PlayerArraySize = static_cast<uint32_t>(g_PlayerArray.size());
				if (m_PlayerArraySize > 0) {
					m_PlayerArray.clear();
					m_PlayerArray = g_PlayerArray;
				}
				if (m_PlayerArraySize == 0) {
					m_PlayerArray.clear();
				}
			}

			uint32_t m_PlayerArraySize = static_cast<uint32_t>(m_PlayerArray.size());

			for (uint32_t i = 0; i < m_PlayerArraySize; i++) {

				uint64_t m_PlayerAddress = m_PlayerArray[i];
				uint32_t m_PlayerTeam = Data::GetPlayerTeam(m_PlayerAddress);

				// 过滤不可见玩家
				if (Memory::ReadMemory<uint32_t>(m_PlayerAddress + SDK::盲区偏移) != 0x1) {
					continue;
				}

				uint64_t m_PlayerEntity = Data::GetPlayerEntity(m_PlayerAddress);
				Vector3 m_PlayerPos = Data::GetPlayerPosition(m_PlayerEntity);
				uint64_t m_LocalPlayerEntity = Data::GetPlayerEntity(g_LocalPlayerAddress);
				Vector3 m_LocalPlayerPos = Data::GetPlayerPosition(m_LocalPlayerEntity);
				float m_Distance = Comp::GetPlayerDistance(m_LocalPlayerPos, m_PlayerPos);

				if (m_Distance > 6.0f || abs(m_PlayerPos.z - m_LocalPlayerPos.z) >= 3.0f) {
					continue;
				}

				uint32_t m_PlayerWeaponId = Data::GetWeaponId(m_PlayerAddress);
				uint32_t m_PlayerZhenDaoAction = Data::GetZhenDaoAction(m_PlayerAddress);
				uint32_t m_PlayerAttackAction = Data::GetAttackAction(m_PlayerAddress);
				uint32_t m_PlayerStatus = Data::GetStatus(m_PlayerAddress);
				BOOL m_PlayerStiffStatus = Data::GetStiffStatus(m_PlayerAddress);
				wstring m_PlayerActionName = Data::GetActionName(m_PlayerAddress);

				if (Data::GetXuliAfter(g_LocalPlayerAddress)) {
					if (m_PlayerZhenDaoAction == 1) {
						if (m_PlayerWeaponId == 116) {
							Sleep(185);
						}
						else {
							Sleep(160);
						}

						if (GetKeyState(VK_LBUTTON) < 0) {
							keyMsg::LeftUp();
							continue;
						}
						if (GetKeyState(VK_RBUTTON) < 0) {
							keyMsg::RightUp();
							continue;
						}
					}
					if (m_PlayerStiffStatus) {
						if (GetKeyState(VK_LBUTTON) < 0) {
							keyMsg::LeftUp();
							continue;
						}
						if (GetKeyState(VK_RBUTTON) < 0) {
							keyMsg::RightUp();
							continue;
						}
					}
					if (m_PlayerAttackAction == 12 && m_PlayerStatus != 20) {
						if (GetKeyState(VK_LBUTTON) < 0) {
							keyMsg::LeftUp();
							continue;
						}
						if (GetKeyState(VK_RBUTTON) < 0) {
							keyMsg::RightUp();
							continue;
						}
					}

					if (Judg::JudgeSkillRecovery(m_PlayerActionName)) {
						if (GetKeyState(VK_LBUTTON) < 0) {
							keyMsg::LeftUp();
							continue;
						}
						if (GetKeyState(VK_RBUTTON) < 0) {
							keyMsg::RightUp();
							continue;
						}
					}
					if (Judg::JudgeLongFlash(m_PlayerActionName)) {
						if (GetKeyState(VK_LBUTTON) < 0) {
							keyMsg::LeftUp();
							continue;
						}
						if (GetKeyState(VK_RBUTTON) < 0) {
							keyMsg::RightUp();
							continue;
						}
					}
					if (Judg::JudgeShortFlash(m_PlayerActionName)) {
						if (GetKeyState(VK_LBUTTON) < 0) {
							keyMsg::LeftUp();
							continue;
						}
						if (GetKeyState(VK_RBUTTON) < 0) {
							keyMsg::RightUp();
							continue;
						}
					}
				}
			}
		}
		Sleep(1);
	}
}

//VOID ZhuiZong(PVOID lpParameter, BOOLEAN TimerOrWaitFired) {
//
//	while (IsWindow(g_Window.Hwnd)) {
//		uint64_t m_ItemPointer = Memory::ReadMemory<uint64_t>(g_Module.GameAssembly + SDK::物品地址);
//		m_ItemPointer = Memory::ReadMemory<uint64_t>(m_ItemPointer + 0xB8);
//		m_ItemPointer = Memory::ReadMemory<uint64_t>(m_ItemPointer + 0x8);
//		m_ItemPointer = Memory::ReadMemory<uint64_t>(m_ItemPointer + 0x28);
//		uint32_t m_ItemCount = Memory::ReadMemory<uint32_t>(m_ItemPointer + 0x24);
//		uint64_t m_InitialAddress = Memory::ReadMemory<uint64_t>(m_ItemPointer + 0x18) + 0x28;
//		for (uint32_t i = 0; i < m_ItemCount; i++) {
//			uint64_t m_ItemAddress = Memory::ReadMemory<uint64_t>(static_cast<uint64_t>(m_InitialAddress) + static_cast<uint64_t>(i) * 0x10);
//			if (m_ItemAddress <= 0) {
//				continue;
//			}
//			uint64_t m_ItemPointer = Memory::ReadMemory<uint64_t>(m_ItemAddress);
//			if (m_ItemPointer <= 0) {
//				continue;
//			}
//			uint64_t m_ItemType = Memory::ReadMemory<uint64_t>(m_ItemPointer + 0x10);
//			char m_ItemTypeTxt[32];
//			Memory::ReadByte(m_ItemType, sizeof(m_ItemTypeTxt), &m_ItemTypeTxt);
//			m_ItemTypeTxt[sizeof(m_ItemTypeTxt) - 1] = '\0';
//			if (!strcmp((const char*)m_ItemTypeTxt, "RangeAgentEntity")) {
//				uint32_t m_ItemID = Memory::ReadMemory<uint32_t>(Memory::ReadMemory<uint64_t>(m_ItemAddress + 0x58) + 0x30);
//				uint64_t m_CoordinatePointer = Memory::ReadMemory<uint64_t>(Memory::ReadMemory<uint64_t>(Memory::ReadMemory<uint64_t>(Memory::ReadMemory<uint64_t>(Memory::ReadMemory<uint64_t>(Memory::ReadMemory<uint64_t>(m_ItemAddress + 0x30) + 0x18) + 0x28) + 0x10) + 0x30) + 0x8);
//				uint32_t m_CoordinateIndex = Memory::ReadMemory<uint32_t>(m_CoordinatePointer + 0x40);
//				uint64_t m_CoordinatePosition = Memory::ReadMemory<uint64_t>(Memory::ReadMemory<uint64_t>(m_CoordinatePointer + 0x38) + 0x18);
//				m_CoordinatePointer = m_CoordinatePosition + static_cast<uint64_t>(m_CoordinateIndex) * 0x30;
//				Vector3 m_ItemPos = Data::GetItemPos(m_CoordinatePointer);
//
//				float m_Distance = Comp::GetPlayerDistance(m_ItemPos, g_BonePos);
//				if (m_Distance <= g_MenuInfo.ZhuiZongRadius) {
//					Memory::WriteMemory<float>(m_CoordinatePointer, g_BonePos.x);
//					Memory::WriteMemory<float>(m_CoordinatePointer + 4, g_BonePos.y);
//					Memory::WriteMemory<float>(m_CoordinatePointer + 8, g_BonePos.z);
//				}
//			}
//		}
//		Sleep(10);
//	}
//}

VOID DrawBones(uint64_t Player) {

	Vector3 NeckPosition = Bone::GetBonePosition(Player, Bone_Draw::脖子);
	Vector3 WaistPosition = Bone::GetBonePosition(Player, Bone_Draw::盆骨);

	Vector3 previous, current;
	Vector2 p1, c1;

	for (auto a : Bone_Draw::BoneList) {
		previous = Vector3{ 0.f, 0.f, 0.f };
		for (int bone : a) {
			current = bone == Bone_Draw::脖子 ? NeckPosition : (bone == Bone_Draw::盆骨 ? WaistPosition : Bone::GetBonePosition(Player, bone));
			if (previous.x == 0.f) {
				previous = current;
				continue;
			}
			Comp::WorldToScreen2D(previous, p1);
			Comp::WorldToScreen2D(current, c1);
			Gui.Line(Vec2(p1.x, p1.y), Vec2(c1.x, c1.y), g_MenuInfo.BoneColor, 2);
			previous = current;
		}
	}
}

VOID DrawHp(float x, float y, float w, uint32_t Cur, uint32_t Max) {

	if (Max != 0 && Cur != 0) {
		float m_x = x - 1;
		float m_y = y;
		float m_ratio = static_cast<float>(Cur) * 100.0f / static_cast<float>(Max);
		ImColor hpColor;
		if (m_ratio > 70) {
			hpColor = ImColor(0, 255, 0, 255);
		}
		else if (m_ratio > 30) {
			hpColor = ImColor(238, 201, 0, 255);
		}
		else {
			hpColor = ImColor(255, 0, 0, 255);
		}
		float m_Hp = Cur * w / Max;
		Gui.Rectangle(Vec2(m_x - 9, m_y), Vec2(9, w), ImColor(0, 0, 0, 255), 2);
		Gui.RectangleFilled(Vec2(m_x - 8, m_y + 1), Vec2(6, w - 1), ImColor(128, 128, 128, 180));
		Gui.RectangleFilled(Vec2(m_x - 8, m_y + 1), Vec2(6, m_Hp - 1), hpColor);
	}
}

VOID DrawShield(float x, float y, float w, uint32_t Cur, uint32_t Max) {

	if (Max != 0 && Cur != 0) {
		float m_x = x - 1;
		float m_y = y;
		ImColor ShieldColor;
		if (Max == 600) {
			ShieldColor = ImColor(190, 190, 190, 255);
		}
		if (Max == 900) {
			ShieldColor = ImColor(30, 144, 255, 255);
		}
		if (Max == 1200) {
			ShieldColor = ImColor(186, 85, 211, 255);
		}
		if (Max == 1500) {
			ShieldColor = ImColor(238, 201, 0, 255);
		}
		if (Max == 1800) {
			ShieldColor = ImColor(255, 0, 0, 255);
		}
		float m_Shield = Cur * w / Max;
		Gui.Rectangle(Vec2(m_x - 18, m_y), Vec2(9, w), ImColor(0, 0, 0, 255), 2);
		Gui.RectangleFilled(Vec2(m_x - 17, m_y + 1), Vec2(6, w - 1), ImColor(128, 128, 128, 180));
		Gui.RectangleFilled(Vec2(m_x - 17, m_y + 1), Vec2(6, m_Shield - 1), ShieldColor);
	}
}

VOID DrawPlayer() {

	static vector<uint64_t> m_PlayerArray;
	uint64_t m_PlayerAddress = NULL;

	if (g_PlayerRefresh) {
		uint32_t m_PlayerArraySize = static_cast<uint32_t>(g_PlayerArray.size());
		if (m_PlayerArraySize > 0) {
			m_PlayerArray.clear();
			m_PlayerArray = g_PlayerArray;
		}
		if (m_PlayerArraySize == 0) {
			m_PlayerArray.clear();
		}
	}

	uint32_t m_PlayerArraySize = static_cast<uint32_t>(m_PlayerArray.size());

	int 附近傻逼 = 0;

	for (uint32_t i = 0; i < m_PlayerArraySize; i++) {

		m_PlayerAddress = m_PlayerArray[i];
		uint32_t m_PlayerTeam = Data::GetPlayerTeam(m_PlayerAddress);
		附近傻逼++;
		// 过滤不可见玩家
		if (Memory::ReadMemory<uint32_t>(m_PlayerAddress + SDK::盲区偏移) != 0x1) {
			continue;
		}
		if (m_PlayerAddress == NULL)
			continue;
		uint64_t m_PlayerEntity = Data::GetPlayerEntity(m_PlayerAddress);
		Vector3 m_PlayerPos = Data::GetPlayerPosition(m_PlayerEntity);
		uint64_t m_LocalPlayerEntity = Data::GetPlayerEntity(g_LocalPlayerAddress);
		Vector3 m_LocalPlayerPos = Data::GetPlayerPosition(m_LocalPlayerEntity);
		PropertyData m_PlayerPropertyData = Data::GetPropertyData(m_PlayerAddress);
		float m_PlayerDistance = Comp::GetPlayerDistance(m_LocalPlayerPos, m_PlayerPos);
		wstring m_PlayerHeroName = Data::GetPlayerHeroName(m_PlayerAddress);
		wstring m_PlayerName = Data::GetPlayerName(m_PlayerAddress);
		wstring m_DrawName_1 = Data::GetRoBot(m_PlayerAddress) ? L"人机 - " + m_PlayerHeroName : m_PlayerName + L" - " + m_PlayerHeroName;
		wstring m_DrawName_2 = L"队伍：" + to_wstring(m_PlayerTeam) + L" - [" + to_wstring((uint32_t)m_PlayerDistance) + L"m]";


		Vector4 m_ScreenPos;
		if (!Comp::WorldToScreen3D(Data::GetPosition(m_PlayerEntity), m_ScreenPos)) {
			continue;
		}
		if (g_MenuInfo.Box) {//方框
			Gui.Rectangle(Vec2(m_ScreenPos.x, m_ScreenPos.y), Vec2(m_ScreenPos.z, m_ScreenPos.w), g_MenuInfo.BoxColor, 2);
		}
		if (g_MenuInfo.Bone) {//骨骼
			DrawBones(m_PlayerAddress);
		}
		if (g_MenuInfo.Shield) {//护甲
			DrawShield(m_ScreenPos.x, m_ScreenPos.y, m_ScreenPos.w, m_PlayerPropertyData.CurShield, m_PlayerPropertyData.ShieldMax);
		}
		if (g_MenuInfo.Hp) {//血条
			DrawHp(m_ScreenPos.x, m_ScreenPos.y, m_ScreenPos.w, m_PlayerPropertyData.CurHp, m_PlayerPropertyData.HpMax);
		}
		if (g_MenuInfo.Info) {//信息
			std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
			std::string narrowStr = converter.to_bytes(m_DrawName_1);
			Gui.Text(narrowStr.c_str(), Vec2(m_ScreenPos.x, m_ScreenPos.y - 20), g_MenuInfo.InfoColor, 20);//这个15数字是大小 懂了。
			std::wstring_convert<std::codecvt_utf8<wchar_t>> converter_;
			std::string narrowStr_ = converter_.to_bytes(m_DrawName_2);
			Gui.Text(narrowStr_.c_str(), Vec2(m_ScreenPos.x, m_ScreenPos.y + m_ScreenPos.w), g_MenuInfo.InfoColor, 20);
		}
	}

	//char 傻逼[100];
	//sprintf_s(傻逼, sizeof(傻逼), u8"附近有：%d 个傻逼", 附近傻逼);
	//Gui.Text(傻逼, Vec2(ImGui::GetWindowWidth(), ImGui::GetWindowHeight() / 1), ImColor(255, 0, 0), 60);

	static RECT g_WinwowSize = { 0 };
	GetClientRect(g_Window.Hwnd, &g_WinwowSize);
	//printf("left:%d		top:%d		right:%d		bottom:%d \n", g_WinwowSize.left, g_WinwowSize.top, g_WinwowSize.right, g_WinwowSize.bottom);

	if (附近傻逼)
	{
		std::string sb = u8"警告:附近有 " + std::to_string(附近傻逼) + u8" 个傻逼";
		//char 人数[100];
		//sprintf_s(人数, sizeof(人数), u8"附近有：%d 个傻逼", 附近傻逼);
		Gui.Text(sb.c_str(), Vec2(g_WinwowSize.right / 2 - 80, g_WinwowSize.bottom / 10), ImColor(255, 0, 0), 30);
	}

}

VOID DrawItem() {

	uint64_t m_ItemPointer = Memory::ReadMemory<uint64_t>(g_Module.GameAssembly + SDK::物品地址);
	m_ItemPointer = Memory::ReadMemory<uint64_t>(m_ItemPointer + 0xB8);
	m_ItemPointer = Memory::ReadMemory<uint64_t>(m_ItemPointer + 0x8);
	m_ItemPointer = Memory::ReadMemory<uint64_t>(m_ItemPointer + 0x28);
	uint32_t m_ItemCount = Memory::ReadMemory<uint32_t>(m_ItemPointer + 0x24);
	uint64_t m_InitialAddress = Memory::ReadMemory<uint64_t>(m_ItemPointer + 0x18) + 0x28;
	for (uint32_t i = 0; i < m_ItemCount; i++) {
		uint64_t m_ItemAddress = Memory::ReadMemory<uint64_t>(static_cast<uint64_t>(m_InitialAddress) + static_cast<uint64_t>(i) * 0x10);
		if (m_ItemAddress <= 0) {
			continue;
		}
		uint64_t m_ItemPointer = Memory::ReadMemory<uint64_t>(m_ItemAddress);
		if (m_ItemPointer <= 0) {
			continue;
		}
		uint64_t m_ItemType = Memory::ReadMemory<uint64_t>(m_ItemPointer + 0x10);
		char m_ItemTypeTxt[32];
		Memory::ReadByte(m_ItemType, sizeof(m_ItemTypeTxt), &m_ItemTypeTxt);
		m_ItemTypeTxt[sizeof(m_ItemTypeTxt) - 1] = '\0';
		if (!strcmp((const char*)m_ItemTypeTxt, "DropItemEntity")) {
			uint32_t m_ItemID = Memory::ReadMemory<uint32_t>(Memory::ReadMemory<uint64_t>(m_ItemAddress + 0x58) + 0x30);
			DataInfo m_ItemInfo = Data::GetItemName(m_ItemID);
			if (m_ItemInfo.Name == "Item") {
				continue;
			}
			uint64_t m_CoordinatePointer = Memory::ReadMemory<uint64_t>(Memory::ReadMemory<uint64_t>(Memory::ReadMemory<uint64_t>(Memory::ReadMemory<uint64_t>(Memory::ReadMemory<uint64_t>(Memory::ReadMemory<uint64_t>(m_ItemAddress + 0x30) + 0x18) + 0x28) + 0x10) + 0x30) + 0x8);
			uint32_t m_CoordinateIndex = Memory::ReadMemory<uint32_t>(m_CoordinatePointer + 0x40);
			uint64_t m_CoordinatePosition = Memory::ReadMemory<uint64_t>(Memory::ReadMemory<uint64_t>(m_CoordinatePointer + 0x38) + 0x18);
			m_CoordinatePointer = m_CoordinatePosition + static_cast<uint64_t>(m_CoordinateIndex) * 0x30;
			Vector3 m_ItemPos = Data::GetItemPos(m_CoordinatePointer);
			Vector2 m_ScreenPos;
			if (Comp::WorldToScreen2D(m_ItemPos, m_ScreenPos)) {
				if (m_ItemInfo.Name != "Item") {
					Gui.StrokeText(m_ItemInfo.Name, Vec2(m_ScreenPos.x, m_ScreenPos.y), m_ItemInfo.Color, 20.0f, true);//15.0f是字体大小
				}
				else {
					ImColor silverColor(255.f, 255.f, 255.f);
					Gui.StrokeText(to_string(m_ItemID), Vec2(m_ScreenPos.x, m_ScreenPos.y), silverColor, 20.0f, true);
				}
			}

		}
	}
}

VOID DrawBuilding() {

	uint64_t m_BuildingPointer = Memory::ReadMemory<uint64_t>(g_Module.GameAssembly + SDK::界面地址);
	m_BuildingPointer = Memory::ReadMemory<uint64_t>(m_BuildingPointer + 0xB8);
	m_BuildingPointer = Memory::ReadMemory<uint64_t>(m_BuildingPointer);
	m_BuildingPointer = Memory::ReadMemory<uint64_t>(m_BuildingPointer + 0x30);
	m_BuildingPointer = Memory::ReadMemory<uint64_t>(m_BuildingPointer + 0x1F0);
	m_BuildingPointer = Memory::ReadMemory<uint64_t>(m_BuildingPointer + 0x10);
	m_BuildingPointer = Memory::ReadMemory<uint64_t>(m_BuildingPointer + 0x18);
	uint32_t m_BuildingCount = Memory::ReadMemory<uint32_t>(m_BuildingPointer + 0x18);
	for (uint32_t i = 0; i < m_BuildingCount; i++) {
		uint64_t m_BuildingAddress = Memory::ReadMemory<uint64_t>(static_cast<uint64_t>(m_BuildingPointer) + 0x18 + static_cast<uint64_t>(i) * 0x18);
		uint32_t m_BuildingID = Memory::ReadMemory<uint32_t>(m_BuildingAddress + 0x14);
		if (m_BuildingID == 0) {
			continue;
		}
		uint32_t m_BuildingOpen = Memory::ReadMemory<uint32_t>(m_BuildingAddress + 0x34);
		if (m_BuildingOpen != 0) {
			continue;
		}
		DataInfo m_ItemInfo = Data::GetBuildingName(m_BuildingID);
		if (m_ItemInfo.Name == "Item") {
			continue;
		}
		Vector3 m_BuildingPos;
		m_BuildingPos.x = Memory::ReadMemory<FLOAT>(m_BuildingAddress + 0x18);
		m_BuildingPos.y = Memory::ReadMemory<FLOAT>(m_BuildingAddress + 0x1C);
		m_BuildingPos.z = Memory::ReadMemory<FLOAT>(m_BuildingAddress + 0X20);
		Vector2 m_ScreenPos;
		if (Comp::WorldToScreen2D(m_BuildingPos, m_ScreenPos)) {
			if (m_ItemInfo.Name != "Item") {
				Gui.StrokeText(m_ItemInfo.Name, Vec2(m_ScreenPos.x, m_ScreenPos.y), m_ItemInfo.Color, 15.0f, true);
			}
			else {
				ImColor silverColor(255.f, 255.f, 255.f);
				Gui.StrokeText(to_string(m_BuildingID), Vec2(m_ScreenPos.x, m_ScreenPos.y), silverColor, 15.0f, true);
			}
		}
	}
}

void 艳青风格()
{
	ImGuiStyle& style = ImGui::GetStyle();
	style.Alpha = 1.0;
	// style.WindowFillAlphaDefault = 0.83;
	// style.ChildWindowRounding = 3;
	style.WindowRounding = 3;
	style.GrabRounding = 1;
	style.GrabMinSize = 20;
	style.FrameRounding = 3;


	style.Colors[ImGuiCol_Text] = ImVec4(0.00f, 1.00f, 1.00f, 1.00f);
	style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.00f, 0.40f, 0.41f, 1.00f);
	style.Colors[ImGuiCol_WindowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.80f);
	style.Colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	style.Colors[ImGuiCol_Border] = ImVec4(0.00f, 1.00f, 1.00f, 0.65f);
	style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	style.Colors[ImGuiCol_FrameBg] = ImVec4(0.44f, 0.80f, 0.80f, 0.18f);
	style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.44f, 0.80f, 0.80f, 0.27f);
	style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.44f, 0.81f, 0.86f, 0.66f);
	style.Colors[ImGuiCol_TitleBg] = ImVec4(0.14f, 0.18f, 0.21f, 0.73f);
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.54f);
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.00f, 1.00f, 1.00f, 0.27f);
	style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.20f);
	style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.22f, 0.29f, 0.30f, 0.71f);
	style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.00f, 1.00f, 1.00f, 0.44f);
	style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.00f, 1.00f, 1.00f, 0.74f);
	style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.00f, 1.00f, 1.00f, 1.00f);
	// style.Colors[ImGuiCol_ComboBg] = ImVec4(0.16f, 0.24f, 0.22f, 0.60f);
	style.Colors[ImGuiCol_CheckMark] = ImVec4(0.00f, 1.00f, 1.00f, 0.68f);
	style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.00f, 1.00f, 1.00f, 0.36f);
	style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.00f, 1.00f, 1.00f, 0.76f);
	style.Colors[ImGuiCol_Button] = ImVec4(0.00f, 0.65f, 0.65f, 0.46f);
	style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.01f, 1.00f, 1.00f, 0.43f);
	style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.00f, 1.00f, 1.00f, 0.62f);
	style.Colors[ImGuiCol_Header] = ImVec4(0.00f, 1.00f, 1.00f, 0.33f);
	style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.00f, 1.00f, 1.00f, 0.42f);
	style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.00f, 1.00f, 1.00f, 0.54f);
	//  style.Colors[ImGuiCol_Column] = ImVec4(0.00f, 0.50f, 0.50f, 0.33f);
	 // style.Colors[ImGuiCol_ColumnHovered] = ImVec4(0.00f, 0.50f, 0.50f, 0.47f);
	  //style.Colors[ImGuiCol_ColumnActive] = ImVec4(0.00f, 0.70f, 0.70f, 1.00f);
	style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 1.00f, 1.00f, 0.54f);
	style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.00f, 1.00f, 1.00f, 0.74f);
	style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.00f, 1.00f, 1.00f, 1.00f);
	//  style.Colors[ImGuiCol_CloseButton] = ImVec4(0.00f, 0.78f, 0.78f, 0.35f);
   //   style.Colors[ImGuiCol_CloseButtonHovered] = ImVec4(0.00f, 0.78f, 0.78f, 0.47f);
	 /// style.Colors[ImGuiCol_CloseButtonActive] = ImVec4(0.00f, 0.78f, 0.78f, 1.00f);
	style.Colors[ImGuiCol_PlotLines] = ImVec4(0.00f, 1.00f, 1.00f, 1.00f);
	style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.00f, 1.00f, 1.00f, 1.00f);
	style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.00f, 1.00f, 1.00f, 1.00f);
	style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.00f, 1.00f, 1.00f, 1.00f);
	style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.00f, 1.00f, 1.00f, 0.22f);
	//  style.Colors[ImGuiCol_TooltipBg] = ImVec4(0.00f, 0.13f, 0.13f, 0.90f);
}

VOID SetCarbonFiberStyle() {

	ImGuiStyle& style = ImGui::GetStyle();

	// 设置颜色
	ImVec4* colors = style.Colors;
	colors[ImGuiCol_Text] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f); // 文本颜色
	colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f); // 禁用文本颜色
	colors[ImGuiCol_WindowBg] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f); // 窗口背景颜色
	colors[ImGuiCol_Border] = ImVec4(0.90f, 0.90f, 0.90f, 0.75f); // 边框颜色
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
	艳青风格();
	ImGui::SetNextWindowSize(ImVec2(450, 405));
	ImGui::Begin(u8"ETC"); {
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

	//if (GetAsyncKeyState(VK_F10)& 0x8000) {//无视振刀快捷键

	//	g_MenuInfo.FangXu = !g_MenuInfo.FangXu;

	//	if (g_MenuInfo.FangXu)
	//	{
	//		Drv.B_WriteMemory(g_Module.GameAssembly + SDK::无视振刀地址, newBytes, 1, RWMode::Mdl);
	//		Beep(1000, 120);
	//		g_KeyTime = GetTickCount64();
	//		return;
	//	}
	//	else
	//	{
	//		Drv.B_WriteMemory(g_Module.GameAssembly + SDK::无视振刀地址, OBytes, 1, RWMode::Mdl);
	//		Beep(1000, 120);
	//		Beep(1000, 120);
	//		g_KeyTime = GetTickCount64();
	//		return;
	//	}
	//}

	//Beep(1000, 120); 滴滴开启声

	//带着所有绘制  取消屏蔽 上面屏蔽=恢复透视
	static bool ShowMenu = false;
	static chrono::time_point LastTimePoint = chrono::steady_clock::now();
	auto CurTimePoint = chrono::steady_clock::now();
	if (GetAsyncKeyState(VK_F11) & 0x8000 && CurTimePoint - LastTimePoint >= std::chrono::milliseconds(150)) {
		ShowMenu = !ShowMenu;
		LastTimePoint = CurTimePoint;
	}
	if (ShowMenu) {
		DrawMenu();
	}

	if (g_MatrixAddress) {
		Data::GetMatrixData(g_MatrixAddress);

		DrawPlayer();
		if (g_MenuInfo.Item) {
			DrawItem();
		}
		if (g_MenuInfo.Building) {
			DrawBuilding();
		}
	}
}

//无视振刀初始化
void wushiInit()
{
	Memory::ReadByte(g_Module.GameAssembly + SDK::无视振刀地址, sizeof(OBytes), OBytes);
}




int main() {

	Api::CloseProgram_DEP();

	if (!Api::EnableDebugPrivilege()) {
		TCHAR tzTemp[MAX_PATH];
		wsprintf(tzTemp, TEXT("进程提权失败！请管理员运行！"));
		MessageBox(NULL, tzTemp, TEXT("提示："), MB_ICONSTOP);
		ExitProcess(-1);
	}

	if (!Drv.B_InitDrv("", B_InstallMode::NtLoadDriver)) {//输入卡密
		TCHAR tzTemp[MAX_PATH];
		wsprintf(tzTemp, TEXT("驱动安装失败！请管理员运行！"));
		printf("Failed: %s\n", Drv.B_GetInitResult());
		MessageBox(NULL, tzTemp, TEXT("提示："), MB_ICONSTOP);
		ExitProcess(-1);
	}
	auto timeOver = Drv.B_GetExpiration();
	std::cout << "截至时间" << timeOver << "\n";
	MessageBox(NULL, TEXT("点击确定运行游戏！"), TEXT("提示："), MB_ICONASTERISK);

	ShowWindow(GetConsoleWindow(), SW_HIDE);
	FreeConsole();

	uint64_t  NeacInterface{};

	do {
		g_Window.Hwnd = FindWindow(L"UnityWndClass", L"Naraka");
		if (g_Window.Hwnd) {
			GetWindowThreadProcessId(g_Window.Hwnd, reinterpret_cast<LPDWORD>(&g_Window.Pid));
			if (g_Window.Pid) {
				Drv.B_AttachProcess(g_Window.Pid);
				g_Module.GameAssembly = Memory::GetModuleAddress("GameAssembly.dll");
				NeacInterface = Memory::GetModuleAddress("NeacInterface.dll");
				
				std::cout << "g_Module.GameAssembly:" << g_Module.GameAssembly << "\n";

				if (g_Module.GameAssembly) {
					g_Module.UWorld = g_Module.GameAssembly + SDK::世界地址;
				}
			}
		}
		Sleep(100);
	} while (g_Module.UWorld == NULL);

	//if (!Hook::HookLoad()) {
	//	TCHAR tzTemp[MAX_PATH];
	//	wsprintf(tzTemp, TEXT("初始内存失败！"));
	//	MessageBox(NULL, tzTemp, TEXT("提示："), MB_ICONSTOP);
	//	ExitProcess(-1);
	//}

	//遍历游戏数据的不能屏蔽
	Kernel::_SafeCreateThread(GameUpdata);
	//无视振刀
	//wushiInit();
	//自瞄线程
	//Kernel::_SafeCreateThread(AimBot);
	//振刀线程
	Kernel::_SafeCreateThread(ZhenDao);
	//放蓄线程
	//Kernel::_SafeCreateThread(Activation);
	//追踪线程
	//Kernel::_SafeCreateThread((WAITORTIMERCALLBACK)ZhuiZong);


	while (true)
	{
		//快捷键开关
		if (GetAsyncKeyState(VK_END) & 0x8000) {//END退出
			ExitProcess(-1);
		}
		Sleep(200);
	}
	//try
	//{
	//	//绘制线程
	//	Gui.AttachAnotherWindow("Naraka", "", DrawRun);
	//}
	//catch (OSImGui::OSException& e) {
	//	std::cout << e.what() << std::endl;
	//}

	system("pause");
}