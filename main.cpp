#include "Windows.h"
#include <iostream>
#include <stdio.h>
#include <string>
#include <sstream>
#include <vector>
#include <fstream>
#include "mips.hpp"

#include "hdr.h"
#include "test.hpp"
#include "AudioSamples.h"
#include "tools/magic_enum/magic_enum.hpp"

void SetColor(WORD wAttributes)
{
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hConsole, wAttributes);
}
void copyToClipboard(const char* text)
{
	if (OpenClipboard(NULL))
	{
		EmptyClipboard();
		size_t len = strlen(text) + 1;
		HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, len);
		if (hMem) { memcpy(GlobalLock(hMem), text, len); GlobalUnlock(hMem); SetClipboardData(CF_TEXT, hMem); }
		CloseClipboard();
	}
}
void MboxSTD(std::string msg, std::string title = "") { MessageBoxA(HWND_DESKTOP, msg.c_str(), title.c_str(), MB_SYSTEMMODAL | MB_ICONWARNING); }

#define CW_R() SetColor(FOREGROUND_RED)                               // Красный
#define CW_G() SetColor(FOREGROUND_GREEN)                             // Зеленый
#define CW_B() SetColor(FOREGROUND_BLUE)                              // Синий
#define CW_Y() SetColor(FOREGROUND_RED | FOREGROUND_GREEN)            // Желтый
#define CW_C() SetColor(FOREGROUND_GREEN | FOREGROUND_BLUE)           // Голубой (Cyan)
#define CW_M() SetColor(FOREGROUND_RED | FOREGROUND_BLUE)             // Магента (Magenta)
#define CW_W() SetColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE) // Белый
#define CW_K() SetColor(0)                                            // Черный (выключить все цвета)

//static inline uintptr_t EEMainMemoryStart = 0/*0x20000000*/;
//static inline uintptr_t EEMainMemoryEnd = 0/*0x21ffffff*/;

struct MemoryRegion { void* baseAddress; SIZE_T size; };
std::vector<MemoryRegion> inline FindRegions(SIZE_T targetSize, DWORD targetType = 0, DWORD targetProtect = 0, DWORD targetState = MEM_COMMIT) {
	std::vector<MemoryRegion> regions;
	MEMORY_BASIC_INFORMATION mbi;
	uintptr_t address = 0;
	while (VirtualQuery((LPCVOID)address, &mbi, sizeof(mbi)) != 0) {
		if (mbi.RegionSize == targetSize) {
			if (targetType != 0 && mbi.Type != targetType) { address += mbi.RegionSize; continue; }
			if (targetProtect != 0 && mbi.Protect != targetProtect) { address += mbi.RegionSize; continue; }
			if (targetState != 0 && mbi.State != targetState) { address += mbi.RegionSize; continue; }
			regions.push_back({ mbi.BaseAddress, mbi.RegionSize });
		}
		address += mbi.RegionSize;
	}
	return regions;
}

inline static uintptr_t CalcPointerFromOffset(uintptr_t op_addr, uintptr_t offset) // call offset => pointer func (restore)
{
	return (op_addr + 1 + sizeof(uintptr_t) + offset);
}
// op_addr , dest pointer, Relative
inline static uintptr_t CalcOffset(uintptr_t op_addr, void* dest_ptr) { return ((uintptr_t)dest_ptr - (op_addr + 1 + sizeof(uintptr_t))); }
inline static void* Relative(void* to, void* address) { return (void*)((int)to - (unsigned int)address - sizeof address); } // same

// same
inline static uintptr_t CalcOffset(uintptr_t op_addr, uintptr_t dst) { return (dst - (op_addr + 1 + sizeof(uintptr_t))); } // W
//inline static uintptr_t CalcPointerFromOffset(uintptr_t op_addr, uintptr_t offset) { return (op_addr + 1 + sizeof(uintptr_t) + offset); } // R

// 0x20100000 1st bytes *.elf binary
// 0x20000000 + poiner -> physical address
#define P_PCSX2_BASE 0x20000000
#define P_PCSX2_END  0x21FFFFFF
#define P_ELF_BASE  0x00100000 // P_PCSX2_BASE + P_ELF_BASE = 1st byte elf program
//#define IDATRANSLATE(p) (((uintptr_t)p) + P_PCSX2_BASE) // IDA -> PCSX2
#define PCSX2POINTER(p) (((uintptr_t)p) + P_PCSX2_BASE) // VIRTUAL -> PHYSICAL
#define PCSXTRANSLATE(p) (((uintptr_t)p) - P_PCSX2_BASE) // PCSX2 -> IDA
#define IDATRANSLATE(p) PCSX2POINTER(p)
void* gpPCSX2base = NULL;
uint32_t inline TranslatePCSX2IDAPTR(uint32_t idap) {
	if (!gpPCSX2base) { gpPCSX2base = FindRegions(0x6C4000, MEM_IMAGE, 0, MEM_COMMIT)[0].baseAddress; }
	return (idap - 0x401000) + (uint32_t)gpPCSX2base;
}
#define IDA2PCSX2160(p) TranslatePCSX2IDAPTR((uint32_t)p) // pcsx2 asm (patch pcsx2)
#define RESET_RECOMP_EE() { auto recResetEE = (void(__stdcall*)())IDA2PCSX2160(0x665570); recResetEE(); } // reset recompiler EE (prevent cached exec)(4patch)
// TODO ADD SUPPORT PCSX2 pnach (need?)
bool& eeCpuExecuting = *(bool*)IDA2PCSX2160(0x02FD4EB8);
bool& eeRecNeedsReset = *(bool*)IDA2PCSX2160(0x02FD4E13);
//auto fdbg = (void(__stdcall*)())IDA2PCSX2160(0x665490);
#define null NULL
#define nil NULL
// GET_REG(eax, _eax); printf("eax: 0x%p\n", _eax);
#define GET_REG(reg, varname) \
    uint32_t varname; \
    __asm { mov varname, reg }
// SET_REG(ecx, 0xFF)
#define SET_REG(reg, value) \
    __asm { mov reg, value }
#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))
#define ALIGN4BYTES(s) ((((uint32_t)s) + 3) & 0xFFFFFFFC) // re3 pcsave
template <typename T, typename R = uint32_t> R inline _stackcast(T v) { return *((R*)&v); } // const reinterpret_cast (stack)
#define BYTESF2U32(f) _stackcast<float, uint32_t>(f)
#define BIT(num) (1<<(num)) // mask BIT[31...0] (x86 Little Endian)
#define GET_BIT(num, n) (((num) >> (n)) & 1) // 11111111(3) 11111111(2) 11111101(1) 11111111(0) mem(FF FD FF FF), 32t_byte1, bit[76543210] <- 1<<(N1)
#define SET_BIT(num, n, val) ((num) = ((num) & ~BIT(n)) | ((val) << (n)))
#define GET_BYTE(num, n) ((num >> (8 * n)) & 0xFF) // 0-lob, 1-midlob, 2-midhib, 3-hib BYTE[0123] (x86 Little Endian)
#define SET_BYTE(num, n, val) ((num) = ((num) & ~(0xFF << (8 * (n)))) | ((val) << (8 * (n))))
#define SWAP_BIT(num, n) SET_BIT(num, n, !GET_BIT(num, n))
#define DUMP_BITS(num) (printf("%d%d%d%d%d%d%d%d", GET_BIT(num, 7), GET_BIT(num, 6), \
	GET_BIT(num, 5), GET_BIT(num, 4), GET_BIT(num, 3), GET_BIT(num, 2), GET_BIT(num, 1), GET_BIT(num, 0)))
#define SWAP_ENDIAN(x) ( \
    (((uint32_t) (x) & 0x000000ff) << 24) | \
    (((uint32_t) (x) & 0x0000ff00) <<  8) | \
    (((uint32_t) (x) & 0x00ff0000) >>  8) | \
    (((uint32_t) (x) & 0xff000000) >> 24) \
) // PS2 SDK
// PDP-10 like byte functions
#define MASK(p, s) (((1 << (s)) - 1) << (p))
inline uint32_t dpb(uint32_t b, uint32_t p, uint32_t s, uint32_t w) { uint32_t m = MASK(p, s); return (w & ~m) | ((b << p) & m); } // Deposit Bit Field
inline uint32_t ldb(uint32_t p, uint32_t s, uint32_t w) { return w >> p & (1 << s) - 1; } // Load Bit Field
#define INRANGE(x,a,b) (x >= a && x <= b) // xarex1337
#define getBits( x ) (INRANGE((x&(~0x20)),'A','F') ? ((x&(~0x20)) - 'A' + 0xa) : (INRANGE(x,'0','9') ? x - '0' : 0))
#define getByte( x ) (getBits(x[0]) << 4 | getBits(x[1]))
template<typename T> T inline EMUPOINTER(void* p) { return (T)(p ? PCSX2POINTER(p) : null); }
template<typename T> T inline EMUPOINTER(uintptr_t p) { return EMUPOINTER<T>((void*)p); }

// rw so funny :/
#define LLLinkGetData(linkvar,type,entry) \
    ((type*)(((uint8_t*)(linkvar))-offsetof(type,entry)))

// Have to be careful since the link might be deleted.
#define FORLIST(_link, _list) \
	for(RwLLLink *_next = nil, *_link = (_list).link.next; \
	_next = (_link)->next, (_link) != (_list).end(); \
	(_link) = _next)

#define PLUGINOFFSET(type, base, offset) \
	((type*)((char*)(base) + (offset)))

#define RWRGBAINT(r, g, b, a) ((uint32)((((a)&0xff)<<24)|(((b)&0xff)<<16)|(((g)&0xff)<<8)|((r)&0xff)))


#define MYDLL_EXPORTS
#define MAZAHAKA // PCSX2 ver 1.6.0 x86  SLUS-215.90
#ifdef MYDLL_EXPORTS
#define MYDLL_API __declspec(dllexport)
#else
#define MYDLL_API __declspec(dllimport)
#endif

extern "C" void MYDLL_API Loader_Dummy() {} // creating export table. cff explorer-> import adder


template<typename T>
void inline patch(uintptr_t address, T value/*, bool idaptr = false*/)
{
	DWORD vp[2];
	//if (idaptr) { address = IDATRANSLATE(address); }
	T* ptr = reinterpret_cast<T*>(address);
	VirtualProtect(ptr, sizeof(T), PAGE_EXECUTE_READWRITE, &vp[0]);
	*ptr = value;
	VirtualProtect(ptr, sizeof(T), vp[0], &vp[1]);
}
template<typename T>
void inline patch(void* address, T value/*, bool idaptr = false*/) { patch<T>((uintptr_t)address, value/*, idaptr*/); }
//#define IDApatch(p, t, v) patch<t>(IDATRANSLATE(p), v); // 	IDApatch(0x123, char, 77);
void inline patchblock(uintptr_t pto, uintptr_t pfrom, uint32_t size/*, bool idaptr = false*/)
{
	DWORD vp[2];
	if (!pto || !pfrom || !size) { printf("err patch\n"); return; }
	//if (idaptr) { pfrom = IDATRANSLATE(address); }
	VirtualProtect((void*)pto, size, PAGE_EXECUTE_READWRITE, &vp[0]);
	memcpy((void*)pto, (void*)pfrom, size);
	VirtualProtect((void*)pto, size, vp[0], &vp[1]);
}
void inline patchblock(uintptr_t pto, void* pfrom, uint32_t size/*, bool idaptr = false*/) { patchblock((uintptr_t)pto, (uintptr_t)pfrom, size); }
void inline patchblock(void* pto, uintptr_t pfrom, uint32_t size/*, bool idaptr = false*/) { patchblock((uintptr_t)pto, (uintptr_t)pfrom, size); }
void inline patchblock(void* pto, void* pfrom, uint32_t size/*, bool idaptr = false*/) { patchblock((uintptr_t)pto, (uintptr_t)pfrom, size); }
void inline patchstring(uintptr_t pto, char* str) { if (str) { patchblock(pto, str, strlen(str) + 1); } }
void inline patchstring(void* pto, char* str) { patchstring((uintptr_t)pto, str); }

uintptr_t inline UNJAL(uintptr_t pos, uintptr_t val) {
	// Извлекаем 26-битный адрес из инструкции JAL
	uintptr_t target = (val & 0x03FFFFFF) << 2;
	// Восстанавливаем старшие 4 бита на основе адреса следующей инструкции
	uintptr_t upper = (pos + 4) & 0xF0000000;
	return upper | target;
}


bool is_valid_pointer(void* ptr)
{
	if (!ptr) { return false; }
	MEMORY_BASIC_INFORMATION mbi;
	if (VirtualQuery(ptr, &mbi, sizeof(mbi)) == 0) { return false; }
	return (mbi.State == MEM_COMMIT) && !(mbi.Protect & PAGE_NOACCESS);
}

bool
IsCurrentProcessWindowIsFocused()
{
	//HWND window = PSGLOBAL(window); 
	HWND activeWindow = GetForegroundWindow();

	DWORD foregroundPID = 0;
	GetWindowThreadProcessId(activeWindow, &foregroundPID);

	return (foregroundPID == GetCurrentProcessId());

	////bool IsMinimized = (IsIconic(window) != 0); // no used here
	//bool IsActive = (window == activeWindow);
	//return IsActive;
}


class MemoryPatcher
{
public:
	uintptr_t address;
	std::vector<uint8_t> originalBytes;
	std::vector<uint8_t> patchBytes;
	int size;
	bool isPatched;
	MemoryPatcher() {}
	MemoryPatcher(uintptr_t addr, uint8_t val, uint32_t cnt) : isPatched(false)//, address(addr), size(cnt)
	{
		if (!addr || !cnt || !is_valid_pointer((void*)addr)) { return; }
		address = addr; // 0 if !valid
		size = cnt;
		for (size_t i = 0; i < size; i++) { patchBytes.push_back(val); }
	}
	MemoryPatcher(uintptr_t addr, std::vector<uint8_t> patchData) : isPatched(false)//, address(addr)
	{
		if (!addr || !patchData.size() || !is_valid_pointer((void*)addr)) { return; }
		address = addr; // 0 if !valid
		patchBytes = patchData;
		size = patchBytes.size();
	}
	void inline ApplyPatch()
	{
		if (!originalBytes.size()) { for (int i = 0; i < size; i++) { originalBytes.push_back(((uint8_t*)address)[i]); } }
		if (size && !isPatched) { patchblock(address, patchBytes.data(), patchBytes.size()); isPatched = true; printf("[0x%p/%d]: Ena\n", address, size); }
	}
	void inline RemovePatch() { if (size && isPatched) { patchblock(address, originalBytes.data(), originalBytes.size()); isPatched = false; printf("[0x%p/%d]: Disa\n", address, size); } }
};


HANDLE InitConsole() // with proto
{
	AllocConsole();

	//SetConsoleOutputCP(866);
	setlocale(LC_ALL, "Russian");
	SetConsoleOutputCP(1251);
	SetConsoleCP(1251);


	freopen("CONIN$", "r", stdin);
	freopen("CONOUT$", "w", stdout);

	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN);

	return hConsole;
}

std::vector<std::string> FileReadAllLines(std::string filePath)
{

	std::vector<std::string> lines;
	std::ifstream file(filePath);

	if (!file.is_open()) { return lines; }

	std::string line;
	while (std::getline(file, line)) {
		lines.push_back(line);
	}

	file.close();
	return lines;
}

void inline reverseString(char* str) {
	int length = strlen(str);
	for (int i = 0; i < length / 2; i++) {
		char temp = str[i];
		str[i] = str[length - i - 1];
		str[length - i - 1] = temp;
	}
}


void transformCheat(char* input, bool encode = true) {
	if (!input) { return; }
	int ps2lvcs_shifts[] = { 2, 5, 10, 1, 7, 6, 10, 11, 7, 9, 3, 8 };
	int shifts_len = sizeof(ps2lvcs_shifts) / sizeof(ps2lvcs_shifts[0]);
	int length = strlen(input);
	for (int i = 0; i < length; i++) {
		input[i] = encode ? (input[i] + ps2lvcs_shifts[i % shifts_len]) : (input[i] - ps2lvcs_shifts[i % shifts_len]);
	}
	input[length] = '\0';
}

void U_SetCurrentDirectory()
{
	// mod by diktor SET CURRENT PATH
	// char currentDir[MAX_PATH]; // STATIC PATH
	// GetCurrentDirectory(MAX_PATH, currentDir);
	// printf("DIR: %s\n", currentDir);
	// memset(currentDir, 0, MAX_PATH);
	// strncpy(currentDir, "C:\\_GTA_RE\\revc\\reVC_GAME", MAX_PATH);
	// currentDir[MAX_PATH - 1] = '\0'; // Ensure null-termination
	// SetCurrentDirectory(currentDir);

	char currentDir[MAX_PATH]; // dynamic set curr dir to exe
	GetModuleFileNameA(NULL, currentDir, MAX_PATH);
	std::string::size_type pos = std::string(currentDir).find_last_of("\\/");
	SetCurrentDirectoryA(std::string(currentDir).substr(0, pos).c_str());
}

#define NUMSECTORS_X 50
#define NUMSECTORS_Y 50
#define SECTOR_SIZE_X 80.0f
#define SECTOR_SIZE_Y 80.0f
#define WORLD_MIN_X -2400.0f
#define WORLD_MIN_Y -2000.0f


#define SLUS_21590 // ntsc
//#define SLES_54622 // pal
//#define SLES_54623 // pal unk
//#define SLPM_66917 // ntsc jap

#ifdef SLUS_21590 // define для того чтобы когда нужно читать с памяти данные, в ините плагина не было краша, читать только когда нужно
#define CWorld_Players ((void*)IDATRANSLATE(0x4E4910))
#define CWorld_PlayerInFocus ((*(uint8_t*)IDATRANSLATE(0x4CD128)))
#define CWorld_ms_aSectors ((CSector*)PCSX2POINTER(*(uintptr_t**)IDATRANSLATE(0x486464))) // 50*50 2500

#define CPad_Pads ((CPad*)IDATRANSLATE(0x5147A8))

#define CModelInfo_ms_modelInfoPtrs ((CBaseModelInfo**)PCSX2POINTER(*(uintptr_t**)IDATRANSLATE(0x486518)))
#define CModelInfo_msNumModelInfos (*(uint32_t*)IDATRANSLATE(0x4CD10C)) // ITS NUM, NOT POINTER!!!!

#define CTheCarGenerators_CarGeneratorArray ((CCarGenerator*)IDATRANSLATE(0x749168))
#define CTheCarGenerators_NumOfCarGenerators (*(uint32_t*)IDATRANSLATE(0x4CD620))

#define gpStreaming ((char*)PCSX2POINTER(*(uintptr_t**)IDATRANSLATE(0x489FF8)))
#define gpModelIndices ((int16_t*)PCSX2POINTER(*(uintptr_t**)IDATRANSLATE(0x48A040))) // CModelIndices

#define pEmpireHud ((CEmpireHud*)PCSX2POINTER(*(uintptr_t**)IDATRANSLATE(0x48F370)))
#define EmpireMgr ((CEmpireMgr*)PCSX2POINTER(*(uintptr_t**)IDATRANSLATE(0x48F050)))
#define EntryExitManager ((char*)PCSX2POINTER(*(uintptr_t**)IDATRANSLATE(0x48FF48)))

#define CFont_Details ((void*)IDATRANSLATE(0x711F30)) // ?

#define gpSkidTex ((void*)PCSX2POINTER(*(uintptr_t**)IDATRANSLATE(0x48A5B0)))
#define currentTexDict ((void*)PCSX2POINTER(*(uintptr_t**)IDATRANSLATE(0x4887B8)))

#define SampleManager ((cSampleManager*)IDATRANSLATE(0x4CDA08))
#define gAm_sfxgxt ((sMissionAudioManager*)IDATRANSLATE(0x4CDA40))
#define TOTAL_AUDIO_SAMPLES 7721

#define aEngineSounds ((tEngineSounds*)IDATRANSLATE(0x4A9538))

#define TheCamera ((uint32_t*)IDATRANSLATE(0x6F44D0))
//#define CMBlur_Drunkness ((float*)IDATRANSLATE(0x6F50A8)) // wrong. todo int32
//#define CTimer_ms_fTimeScale ((float*)IDATRANSLATE(0x4CD168))

//=================================================================================================== POOLS
#define CPools_ms_pPtrNodePool ((CPool*)PCSX2POINTER(*(uintptr_t**)IDATRANSLATE(0x487A8C))) // 12
#define CPools_ms_pEntryInfoNodePool ((CPool*)PCSX2POINTER(*(uintptr_t**)IDATRANSLATE(0x487A90))) // 20
#define CPools_ms_pPedPool ((CPool*)PCSX2POINTER(*(uintptr_t**)IDATRANSLATE(0x487A94))) // 3360
#define CPools_ms_pVehiclePool ((CPool*)PCSX2POINTER(*(uintptr_t**)IDATRANSLATE(0x487A98))) // 2240
#define CPools_ms_pBuildingPool ((CPool*)PCSX2POINTER(*(uintptr_t**)IDATRANSLATE(0x487A9C))) // 96
#define CPools_ms_pTreadablePool ((CPool*)PCSX2POINTER(*(uintptr_t**)IDATRANSLATE(0x487AA0))) // 96
#define CPools_ms_pObjectPool ((CPool*)PCSX2POINTER(*(uintptr_t**)IDATRANSLATE(0x487AA4))) // 544
#define CPools_ms_pEmpirePool ((CPool*)PCSX2POINTER(*(uintptr_t**)IDATRANSLATE(0x487AA8))) // 352
#define CPools_ms_pDummyPool ((CPool*)PCSX2POINTER(*(uintptr_t**)IDATRANSLATE(0x487AAC))) // 96
#define CPools_ms_pAudioScriptObjectPool ((CPool*)PCSX2POINTER(*(uintptr_t**)IDATRANSLATE(0x487AB0))) // 48
#define CTexListStore_ms_pTexListPool ((CPool*)PCSX2POINTER(*(uintptr_t**)IDATRANSLATE(0x489ECC))) // 28

#define POOLFLAG_ID 0x7F
#define POOLFLAG_ISFREE 0x80
// get entity by pool handle
inline void* CPools_GetAt(CPool* p, int32_t h, int32_t maxe) { return (h == -1) ? null : ((uint8_t*)PCSX2POINTER(p->m_ByteMap))[h >> 8] == (h & 0xFF) ? &((uint8_t*)PCSX2POINTER(p->m_Objects))[(h >> 8) * maxe] : null; }
// get entity by array index (slot)
inline void* CPools_GetSlot(CPool* p, int32_t i, int32_t maxentsize) { return p ? (void*)(PCSX2POINTER(p->m_Objects) + (i * maxentsize)) : null; }
// index (number object in pool (array index))
inline int CPools_GetJustIndex(CPool* p, void* pE, int32_t maxe) { return pE ? (((uintptr_t)pE) - ((uintptr_t)PCSX2POINTER(p->m_Objects))) / maxe : 0; }
// index (pool handle)
inline int CPools_GetIndex(CPool* p, void* pE, int32_t maxe) { int i = CPools_GetJustIndex(p, pE, maxe); return ((uint8_t*)PCSX2POINTER(p->m_ByteMap))[i] + (i << 8); }
// is slot free
inline bool CPools_GetSlotIsFree(CPool* p, int32_t i) { return !!(((uint8_t*)PCSX2POINTER(p->m_ByteMap))[i] & POOLFLAG_ISFREE); }

// pool handle system 2
#define POOL_INDEX_MASK     0x7FFFFF
#define POOLFLAG_BUILDING  0x1000000
#define POOLFLAG_EMPIRE    0x0800000

#define CTheScripts_aCommandsHandlers ((int64_t*)IDATRANSLATE(0x4BEAF0))
#define CTheScripts_ScriptSpace ((char*)PCSX2POINTER(*(uintptr_t**)IDATRANSLATE(0x48741C))) // CScriptThread :)
#define CTheScripts_pActiveScripts ((CRunningScript*)PCSX2POINTER(*(uintptr_t**)IDATRANSLATE(0x4CD3C4)))
#define CTheScripts_pIdleScripts ((CRunningScript*)PCSX2POINTER(*(uintptr_t**)IDATRANSLATE(0x4CD3C8)))
#define CTheScripts_MainScriptSize ((*(uint32_t*)IDATRANSLATE(0x4CD3D8))) // COMMAND_LOAD_AND_LAUNCH_MISSION_INTERNAL
#define CTheScripts_LargestMissionScriptSize ((*(uint32_t*)IDATRANSLATE(0x4CD3E8))) // COMMAND_LOAD_AND_LAUNCH_MISSION_INTERNAL
#define CTheScripts_MultiScriptArray ((int32_t*)IDATRANSLATE(0x7404D0)) // missions offsets
#define ScriptParams ((uint32_t*)IDATRANSLATE(0x50DEF8))
bool& gbGlassCheat = *(bool*)IDATRANSLATE(0x489EB4);
bool& CSpecialFX_bLiftCam = *(bool*)IDATRANSLATE(0x481CD8);
bool& CPad_bHasPlayerCheated = *(bool*)IDATRANSLATE(0x487A54);
float& CTimer_ms_fTimeScale = *(float*)IDATRANSLATE(0x4CD168);
int& CGame_currArea = *(int*)IDATRANSLATE(0x489F7C);
int& CRenderer_ms_nNoOfVisibleEmpires = *(int*)IDATRANSLATE(0x4CD5A0);
//int& CWorld_ms_aSectors[50][50] = *(int*)IDATRANSLATE();
RwObjectNameIdAssocation** CVehicleModelInfo_ms_vehicleDescs = (RwObjectNameIdAssocation**)IDATRANSLATE(0x489E38); // size 10
#elif defined(SLES_54622) 
#elif defined(SLES_54623) 
#elif defined(SLPM_66917) 
#endif


#define CTheScripts_pMissionScript (&CTheScripts_ScriptSpace[CTheScripts_MainScriptSize]) // mission scm
//#define SCRVAR(i) ((uint32_t*)(((uintptr_t)CTheScripts_ScriptSpace) + (sizeof(uint32_t) * i)))
#define SCRVAR(i) (&((uint32_t*)CTheScripts_ScriptSpace)[i])
#define SCRBYTEVAR(i) (&((uint8_t*)CTheScripts_ScriptSpace)[i])
#define DUMPSCRVAR(i) printf("SCR_%d: %d\n", i, *SCRVAR(i))
#define SCRIPRANGE(ip) ((ip >= 0) && (ip < CTheScripts_MainScriptSize + CTheScripts_LargestMissionScriptSize))
#define SCRSTRIP(ip) (SCRIPRANGE(ip) ? ip : 0)
#define DUMPSCRSTRVAR(i) printf("SCRSTR_%d: %.7s\n", i, SCRBYTEVAR(SCRSTRIP(*SCRVAR(i)))) // strvar is ip in scm where str
#define DUMPSCRARRAY(i, sz) printf("SCR_%d [", i); for (int j = 0; j < sz; j++) { printf("%d ", *SCRVAR(i+j)); } printf("]\n");
#define DUMPSCRSTRARRAY(i, sz) printf("SCRSTR_%d [", i); for (int j = 0; j < sz; j++) { printf("%.7s ", SCRBYTEVAR(SCRSTRIP(*SCRVAR(i+j)))); } printf("]\n");
#define DUMPVEC(s, vec) printf("%s %f %f %f\n", s, vec.x, vec.y, vec.z)
#define DUMPPOS(vec) printf("x:%f  y:%f  z:%f\n",vec.x, vec.y, vec.z)
void inline SETCHEAT(uintptr_t p, const char* c) { // T S X C  L R U D  (L1)1 (R1)2
	if (p && c)	{ char ch[100] = { 0 }; strcpy(ch, c); reverseString(ch); transformCheat(ch); patchstring(IDATRANSLATE(p), ch); }
}
void inline DumpController(CControllerState* p) {
	if (p) printf("LX:%d LY:%d RX:%d RY:%d L1:%d L2:%d R1:%d R2:%d DU:%d DD:%d DL:%d DR:%d S:%d SE:%d SQ:%d TR:%d CR:%d CI:%d LSH:%d RSH:%d\n",
		p->LeftStickX, p->LeftStickY, p->RightStickX, p->RightStickY,
		p->LeftShoulder1, p->LeftShoulder2, p->RightShoulder1, p->RightShoulder2,
		p->DPadUp, p->DPadDown, p->DPadLeft, p->DPadRight,
		p->Start, p->Select, p->Square, p->Triangle,
		p->Cross, p->Circle, p->LeftShock, p->RightShock);
}
CSector* GetSector(int x, int y) { return &CWorld_ms_aSectors[(NUMSECTORS_X*y)+x]; }
CSector* GetSectorByPos(float wx, float wy) {
	int sectorX = (int)((wx - WORLD_MIN_X) / SECTOR_SIZE_X);
	int sectorY = (int)((wy - WORLD_MIN_Y) / SECTOR_SIZE_Y);
	// Ограничиваем диапазон, чтобы не выйти за пределы массива
	if (sectorX < 0) sectorX = 0;
	if (sectorX >= NUMSECTORS_X) sectorX = NUMSECTORS_X - 1;
	if (sectorY < 0) sectorY = 0;
	if (sectorY >= NUMSECTORS_Y) sectorY = NUMSECTORS_Y - 1;
	return GetSector(sectorX, sectorY);
}
int GetEntityType(CEntity* pEntity) { int m_type = ((pEntity->_CE_flags_E >> 1) & 0x07); return m_type; }
void SetEntityType(CEntity* pEntity, int type) { pEntity->_CE_flags_E &= ~(0x07 << 1); pEntity->_CE_flags_E |= (type & 0x07) << 1; }
int GetEntityStatus(CEntity* pEntity) { int m_status = ((pEntity->_CE_flags_E >> 4) & 0x0F) | ((pEntity->CE_flags_F & 0x01) << 4); return m_status; }
void SetEntityStatus(CEntity* pEntity, int st)
{ 
	pEntity->_CE_flags_E &= ~(0x0F << 4); pEntity->_CE_flags_E |= (st & 0x0F) << 4; pEntity->CE_flags_F &= ~0x01; pEntity->CE_flags_F |= (st >> 4) & 0x01;
}

void TeleportEntity(CEntity* pE, CVector pos, bool updrw = true)
{
	//if (pE) { pE->CPlaceable.m_pMat.pos.x = pos.x; pE->CPlaceable.m_pMat.pos.y = pos.y; pE->CPlaceable.m_pMat.pos.z = pos.z; }
	if (pE) { SetCVector4VU(&pE->CPlaceable.m_pMat.pos, &pos); }
	if (pE && updrw && pE->CPlaceable.m_pMat.m_pRwMat) { SetRWV3D(&pE->CPlaceable.m_pMat.m_pRwMat->pos, &pos); } // todo update rw stuff
}
inline CBaseModelInfo* GetModelInfo(int index) { return EMUPOINTER<CBaseModelInfo*>(CModelInfo_ms_modelInfoPtrs[index]); }
CPlayerPed* FindPlayerPed() { return EMUPOINTER<CPlayerPed*>(((CPlayerInfo*)CWorld_Players)[CWorld_PlayerInFocus].m_pPed); }
CVehicle* FindPlayerVehicle() {
	CPed* pPed = EMUPOINTER<CPed*>(((CPlayerInfo*)CWorld_Players)[CWorld_PlayerInFocus].m_pPed);
	return EMUPOINTER<CVehicle*>(pPed ? pPed->m_pMyVehicle : null);
}
CVector FindPlayerPos() { return FindPlayerPed() ? (*(CVector*)&FindPlayerPed()->CPed.CPhysical.CEntity.CPlaceable.m_pMat.pos) : CVector{0, 0, 0}; }

const char* GetRwObjectDescByType(int objtype)
{
	switch (objtype) {
	case rpFRAME: return "FRAME";
	case rpATOMIC: return "ATOMIC";
	case rpCLUMP: return "CLUMP";
	case rpLIGHT: return "LIGHT";
	case rpCAMERA: return "CAMERA";
	case rp5: return "reserved";
	case rpTEXDICTIONARY: return "TEXDICTIONARY";
	case rpWORLD: return "WORLD";
	case rpGEOMETRY: return "GEOMETRY";
	default: return "Unknown case";
	}
}

//void SwitchScriptMode(CRunningScript* pScript, bool turnoff = true) {
//	if (pScript) {
//		if (turnoff) {
//			//RemoveScriptFromList(&CTheScripts::pActiveScripts); // todo
//			//AddScriptToList(&CTheScripts::pIdleScripts);
//			pScript->m_bIsActive = false;
//		}
//		else {
//
//		}
//	}
//}



std::vector<MemoryPatcher> patches;
void InitPatches()
{
	patches = {
		//MemoryPatcher(IDATRANSLATE(0x489EB4), { 0x01 }), // test glass cheat
		//MemoryPatcher(IDATRANSLATE(0x004905FC), { 0x00, 0x00, 0x00, 0x00 }), // стопит
		//MemoryPatcher(IDATRANSLATE(0x00490614), { 0x00, 0x00, 0x00, 0x00 }),

		//MemoryPatcher(IDATRANSLATE(0x231588), 0, 0x23159C - 0x231588), // nop CPlayerPed::ProcessPlayerWeapon
		//MemoryPatcher(IDATRANSLATE(0x003A7AD0), 0, 0x003A7AE0 - 0x003A7AD0), // nop getpadstuff
		//MemoryPatcher(IDATRANSLATE(0x003BAAC0), 0, 4), // nop CTheScripts::Process
		//MemoryPatcher(IDATRANSLATE(0x003A7C98), 0, 0x003A80EC - 0x003A7C98), // nop 887 COMMAND_GET_PAD_BUTTON_STATE !!
		//MemoryPatcher(IDATRANSLATE(0x21F768), 0, 8),

		//MemoryPatcher(IDATRANSLATE(0x003A7CC0), 0, 0x003A7CD8 - 0x003A7CC0), // r3

		//MemoryPatcher(IDATRANSLATE(0x00411EC8), 0, 0x00411ED4 - 0x00411EC8), // fe
		//MemoryPatcher(IDATRANSLATE(0x0021CFEC), 0, 0x0021D014 - 0x0021CFEC), // bg

		//MemoryPatcher(IDATRANSLATE(0x0021EBCC), 0, 4), // RenderScene
		//MemoryPatcher(IDATRANSLATE(0x0021EBF4), 0, 4), // RenderEffects
		//MemoryPatcher(IDATRANSLATE(0x0021EC24), 0, 0x0021EC34 - 0x0021EC24), // Render2DStuff
		//MemoryPatcher(IDATRANSLATE(0x0021F74C), 0, 0x0021F754 - 0x0021F74C), // empire stuff draw
		//MemoryPatcher(IDATRANSLATE(0x00104B48), 0, 0x00104B64 - 0x00104B48), // test
		//MemoryPatcher(IDATRANSLATE(0x00104B70), 0, 4), // test empire_sub_104BD0 sprite+text
		//MemoryPatcher(IDATRANSLATE(0x00104D1C), 0, 0x00104D30 - 0x00104D1C), // test empire не заметил разницы
		//MemoryPatcher(IDATRANSLATE(0x00104D34), 0, 0x00104D40 - 0x00104D34), // test empire
		//MemoryPatcher(IDATRANSLATE(0x00104FB4), 0, 0x00104FC4 - 0x00104FB4), // test empire (draw sprites)
		//MemoryPatcher(IDATRANSLATE(0x0017A818+8), {/*0,0,0,0, 0x2d,0x10,0x80,0,*/ 0,0,0,0, 0,0,0,0, 0x8,0,0xe,0x3, /*0,0,0,0*/}), // tst

		//MemoryPatcher(IDATRANSLATE(0x0017A818), 0, 4), // 0
		//MemoryPatcher(IDATRANSLATE(0x0017A820), 0, 4), // 12
		//MemoryPatcher(IDATRANSLATE(0x0017A824), 0, 4), // 8
		//MemoryPatcher(IDATRANSLATE(0x0017A82C), 0, 4), // 4
		//MemoryPatcher(IDATRANSLATE(0x0017A82C), {0x00, 0, 0x8f, 0xe4}), // 4
		//MemoryPatcher(IDATRANSLATE(0x00272F50+4), BYTESF2U32(440.0f), 4), // 440.0f
		//MemoryPatcher(IDATRANSLATE(0x00272F58+4), BYTESF2U32(440.0f), 4), // 272.0f
		//MemoryPatcher(IDATRANSLATE(0x00272F60+4), BYTESF2U32(40.0f), 4), // 40.0f

		//MemoryPatcher(IDATRANSLATE(0x00104B4C), 0, 4), //  assert
		//MemoryPatcher(IDATRANSLATE(0x0013C420), {0,0,0,0, (uint8_t)199,0,2,0x24 }), // idaho hydraulic
		//MemoryPatcher(IDATRANSLATE(0x0013C3E4), { (uint8_t)199,}), // idaho 
		//MemoryPatcher(IDATRANSLATE(0x004CD2DC), { (uint8_t)1,}), // bullet world stuff

		//MemoryPatcher(IDATRANSLATE(0x0021EF80), 0, 4), // cvis не заметил
		//MemoryPatcher(IDATRANSLATE(0x0021EF0C), 0, 4), // matt1 мир
		//MemoryPatcher(IDATRANSLATE(0x0021EF54), 0, 4), // matt2 вода
		//MemoryPatcher(IDATRANSLATE(0x0021EF5C), 0, 4), // unk sub like ps2 alpha test
		//MemoryPatcher(IDATRANSLATE(0x0021F158), 0, 4), // some render empire build 3d
		//MemoryPatcher(IDATRANSLATE(0x0021F040), 0, 4), // some render 

		//MemoryPatcher(IDATRANSLATE(0x00109420), 0, 4), // matren cws render1  roads builds
		//MemoryPatcher(IDATRANSLATE(0x00109460), 0, 4), // matren cws render2 pr2 builds
		//MemoryPatcher(IDATRANSLATE(0x00109468), 0, 4), //roads
		//MemoryPatcher(IDATRANSLATE(0x0023E458), 0, 4), //push

		//MemoryPatcher(IDATRANSLATE(0x003BAC38), 0, 4), // TheCamera Process
		//MemoryPatcher(IDATRANSLATE(0x00177CB0), 0, 0x00178024  - 0x00177CB0), // cboat proc unk nop
		//MemoryPatcher(IDATRANSLATE(0x0031C8E8), 0, 0x0031C928 - 0x0031C8E8), // nop

		//MemoryPatcher(IDATRANSLATE(0x00230664), 0, 0x00230670 - 0x00230664), // nop
		//MemoryPatcher(IDATRANSLATE(0x00104C4C), 0, 8), // nop cfont
		//MemoryPatcher(IDATRANSLATE(0x00104D1C), 0, 0x00104D30 - 0x00104D1C), // nop
		//MemoryPatcher(IDATRANSLATE(0x00104CA0), 0, 0x00104D50 - 0x00104CA0), // nop
		//MemoryPatcher(IDATRANSLATE(0x00104D3C), 0, 4), // nop
		//MemoryPatcher(IDATRANSLATE(0x003F41F8), 0, 0x003F4224 - 0x003F41F8), // nop

		//MemoryPatcher(IDATRANSLATE(0x003BAB44), 0, 4), // nop peds gen
		//MemoryPatcher(IDATRANSLATE(0x003BAC88), 0, 4), // nop cars gen

		//MemoryPatcher(IDATRANSLATE(0x26BD34), 0, 4), // nop test menu draw empires
		//MemoryPatcher(IDATRANSLATE(0x35D208), 0, 8), // empire bool
		//MemoryPatcher(IDATRANSLATE(0x0035D16C), 0, 4), // empire ren1 (primary)
		//MemoryPatcher(IDATRANSLATE(0x0035D21C), 0, 4), // empire ren2  (bool1) // окна
		//MemoryPatcher(IDATRANSLATE(0x0035D208), 0, 4*2), // empire ren bool not
		//MemoryPatcher(IDATRANSLATE(0x0035D0BC), 0, 4*3), // empire geo2mat cb zero nop  // не заметил

		//MemoryPatcher(IDATRANSLATE(0x003F3C3C), 0, 0x003F3D08 - 0x003F3C3C), // fonts reset
		//MemoryPatcher(IDATRANSLATE(0x003F4478), 0, 0x003F448C - 0x003F4478), // fonts setcol3
	};
}
void inline SetPatchesState(bool state) { for (size_t i = 0; i < patches.size(); i++) { if (state) { patches[i].ApplyPatch(); } else { patches[i].RemovePatch(); } } }



void PatchTest() // simple hack 2 call func from pad
{   // T S X C  L R U D  (L1)1 (R1)2
	uint32_t ptrashmaster_jal = 0x284108;
	uint32_t instr = *(uint32_t*)IDATRANSLATE(ptrashmaster_jal);
	//printf("old trashmaster cheat pointer [PS2] 0x%p  [PHYS] 0x%p\n", UNJAL(ptrashmaster_jal, instr), PCSX2POINTER(UNJAL(ptrashmaster_jal, instr)));
	SETCHEAT(0x481C10, "1122"); // L1 L1 R1 R1   [trashmaster cheat]
	//patch<uint32_t>(IDATRANSLATE(ptrashmaster_jal), mips::jal(0x2826F8)); // [trashmaster cheat 2 money cheat]
	// simple jal from pad
	RESET_RECOMP_EE(); // update pcsx2 cached mips
}

void SetCarSpawnerID(uint8_t mi)
{
	SETCHEAT(0x481C10, "1122"); // L1 L1 R1 R1   [trashmaster cheat] // simplify
	patch<uint8_t>(IDATRANSLATE(0x28217C), mi);
	RESET_RECOMP_EE(); // update pcsx2 cached mips
}

void TeleportPlayer(CVector pos)
{
	CPed* pPed = EMUPOINTER<CPed*>(((CPlayerInfo*)CWorld_Players)[CWorld_PlayerInFocus].m_pPed);
	CVehicle* pVehicle = EMUPOINTER<CVehicle*>(pPed ? pPed->m_pMyVehicle : null);
	if (pVehicle) { SetCVector4VU(&pVehicle->CPhysical.CEntity.CPlaceable.m_pMat.pos, &pos); } // todo TeleportEntity
	else if (pPed) { SetCVector4VU(&pPed->CPhysical.CEntity.CPlaceable.m_pMat.pos, &pos); } // todo TeleportEntity
}

bool gbTeleportHold = false;
int giTeleportIndex = 0;
void GeneratorTeleporterTick()
{
	bool prev = (GetAsyncKeyState(VK_OEM_COMMA) & 0x8000);
	bool next = (GetAsyncKeyState(VK_OEM_PERIOD) & 0x8000);
	bool tp = false;
	if (prev && !gbTeleportHold)
	{
		--giTeleportIndex;
		gbTeleportHold = tp = true;
	}
	else if (next && !gbTeleportHold)
	{
		++giTeleportIndex;
		gbTeleportHold = tp = true;
	}
	else if (!prev && !next) { gbTeleportHold = false; }
	if (giTeleportIndex < 0) { giTeleportIndex = CTheCarGenerators_NumOfCarGenerators - 1; }
	if (giTeleportIndex >= CTheCarGenerators_NumOfCarGenerators) { giTeleportIndex = 0; }
	if (tp) {
		CVector pos = CTheCarGenerators_CarGeneratorArray[giTeleportIndex].m_vecPos;
		printf("[TP id_%d, mi_%d]: %f %f %f\n", giTeleportIndex, CTheCarGenerators_CarGeneratorArray[giTeleportIndex].m_nModelIndex, pos.z, pos.y, pos.z);
		pos.x -= 2.0f;
		pos.y -= 2.0f;
		pos.z -= 0.5f;
		TeleportPlayer(pos);
	}
}

// todo hack call ps2 malloc, tmp using script space or any array
#define MAX_MIPS_PATCH_SIZE (4 * 30) // (msp) [MIPS] [SCM]
void* SetBuff(void* buff, uint32_t size, uint32_t offset)
{
	if (buff && size)
	{
		size = size > MAX_MIPS_PATCH_SIZE ? MAX_MIPS_PATCH_SIZE : size;
		DWORD vp[2];
		void* p = (void*)ALIGN4BYTES(CTheScripts_pMissionScript + offset); // :)
		VirtualProtect(p, size, PAGE_EXECUTE_READWRITE, &vp[0]); // !!
		memcpy(p, buff, size);
		VirtualProtect(p, size, vp[0], &vp[1]);
		return p;
	}
	return null;
}

void* gpmips = null;
void PatchMIPS(int32_t ip)
{
	//ip = 250; // mi boat
	uint32_t buff[] =
	{
		mips::addiu<int16_t>(mips::sp, mips::sp, -0x10), // malloc stack
		//0x27BDFFF0,
		0xFFBF0000, // sd      $ra, var_s0($sp)  // save stack
		//-----------------------------------------------------------------

		//0x240400F1, // li      $a0, 0xF1 
		mips::lui(mips::a0, (ip >> 16) & 0xFFFF), // Загружаем старшие 16 бит // Загрузка `ip` в `$a0` (li эквивалент)
		mips::ori(mips::a0, mips::a0, ip & 0xFFFF), // Добавляем младшие 16 бит

		//mips::jal(0x281D90), // VehicleCheat
		mips::jal(0x255950), // CTheScripts::StartNewScript
		mips::nop(), // NOP для delay slot (jal после себя скипнет это, забей её аля иструкцией над jmp или nop)
		//0x240400F1, // li      $a0, 0xF1 

		//mips::li(mips::a0, ip), // Загружаем ip в $a0
		////0x0000202D, // move    $a0, $zero       # ip

		//mips::lui(mips::a0, (ip >> 16) & 0xFFFF), // Загружаем старшие 16 бит // Загрузка `ip` в `$a0` (li эквивалент)
		//mips::ori(mips::a0, mips::a0, ip & 0xFFFF), // Добавляем младшие 16 бит
		//mips::nop(), // NOP для delay slot

		//mips::jal(0x00324388), // SpawnInModel
		//mips::nop(), // NOP для delay slot (jal после себя скипнет это, забей её аля иструкцией над jmp или nop)

		//-----------------------------------------------------------------
		0xDFBF0000, // ld      $ra, var_s0($sp)
		mips::jr(mips::ra),
		//0x03E00008, // jr      $ra
		mips::addiu<uint16_t>(mips::sp, mips::sp, 0x10),
		//0x27BD0010, // addiu   $sp, 0x10
	};

	SETCHEAT(0x481C10, "1122"); // L1 L1 R1 R1   [trashmaster cheat] // simplify
	gpmips = SetBuff(buff, sizeof(buff), 0);
	patch<uint32_t>(IDATRANSLATE(0x284108), mips::jal(PCSXTRANSLATE(gpmips))); // [trashmaster cheat]
	printf("[MIPS] patched 0x%p\n", gpmips);
	RESET_RECOMP_EE(); // update pcsx2 cached mips
}

uint8_t tmppedspawntype = 6;
uint8_t tmppedspawnmi = 8;
uint8_t tmpcarspawnmi = 0 ? 219 : 171; // 219 171
CVector pedpos = { -860.789978f, -259.415802f, 11.0f };
int32_t PatchSCM()
{ // buff scrvar 5547  (forklift float)  0xE2, 0xAB
#define IB(v) GET_BYTE(v, 0), GET_BYTE(v, 1), GET_BYTE(v, 2), GET_BYTE(v, 3)
#define B(v) IB(BYTESF2U32(v))
	uint8_t mi = tmppedspawnmi;
	uint8_t cmi = tmpcarspawnmi;
	pedpos = FindPlayerPos();
	DUMPVEC("pos %f %f %f\n", pedpos);
	//printf("pos %d %d %d %d\n", B(pedpos.x));
	uint8_t buff[] =
	{
		// {84394} 0160: request_model 228
		0x60, 0x01, 0x07, mi,
		0x60, 0x01, 0x07, cmi,//219

		// {76286} 0228: load_all_models_now
		0x28, 0x02,

		//0001: wait 0
		0x01, 0x00, 0x01,
		//0x01, 0x00, 0x08, 0xE8, 0x03,

		0x5B, 0x00, 0x01, 0x01, // 005B: set_time_of_day 0 0
		// 011D: print_with_number_big 'M_PASS' number 99 time 5000 style 6 // ~Y~MISSION PASSED! ~n~$~1~
		0x1D, 0x01, 0x0A,  /**/0x4D, 0x5F, 0x50, 0x41, 0x53, 0x53, 0x00,/**/  0x07, 0x63, 0x08, 0x88, 0x13, 0x07, 0x06,
		//0x3E, 0x00, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x13, // {72242} 003E: create_char 0@ model 1@ at 2@ 3@ 4@ store_to 6@

		// 003E: create_char 6 model 8 at 1000.0 1000.0 1000.0 store_to $5547
		0x3E, 0x00,  0x07, tmppedspawntype, 0x07, mi,  0x9, B(pedpos.x),  0x9, B(pedpos.y), 0x9, B(pedpos.z),  0xE2, 0xAB,

		// 0048: create_car 219 at 5 5 5 store_to $fdg
		//0x48, 0x00,  0x07, cmi, 0x9, B(x),  0x9, B(y), 0x9, B(z),  0xE2, 0xAB, //todo
		//0x48, 0x00, 0x08, 0xDB, 0x00, 0x07, 0x05, 0x07, 0x05, 0x07, 0x05, 0xE2, 0xAC,

		//0x9C, 0x02, 0x01, // 029C: start_credits group 0
		0x23, 0x00, //0023: terminate_this_script (mv2idle)
		//0x26, 0x00, // 0026: return (gosub)
	};

	void* p = SetBuff(buff, sizeof(buff), MAX_MIPS_PATCH_SIZE);
	int32_t ip = ((int32_t)p - (int32_t)CTheScripts_ScriptSpace);
	//SETCHEAT(0x481C10, "1122"); // L1 L1 R1 R1   [trashmaster cheat] // simplify
	//patch<uint32_t>(IDATRANSLATE(0x284108), mips::jal(PCSXTRANSLATE(p))); // [trashmaster cheat]
	printf("[SCM] patched [IP:%d]\n", ip);
	RESET_RECOMP_EE(); // update pcsx2 cached mips
	return ip;
}

void inline RunTestSCM() { int32_t ip = PatchSCM(); PatchMIPS(ip); }

void ProcessPrekol(bool prekol)
{
	static int phase = 0;
	static int colorValue = 0;
	static int step = 5; // Шаг изменения цвета
	static int tickCounter = 0;
	static int tickThreshold = 5; // Количество тиков для смены фазы

	if (!prekol) { CSpecialFX_bLiftCam = false; return; }

	CVehicle* pPlayerVeh = FindPlayerVehicle();
	if (!pPlayerVeh) { return; }

	CSpecialFX_bLiftCam = true;
	CTimer_ms_fTimeScale = 1.0f;
	CRGBA& c1 = pPlayerVeh->m_currentColour1;
	CRGBA& c2 = pPlayerVeh->m_currentColour2;

	if (++tickCounter >= tickThreshold) {
		tickCounter = 0;
		switch (phase) {
		case 0: // Увеличиваем синий
			c1.blue += step;
			if (c1.blue >= 255) phase = 1;
			break;
		case 1: // Уменьшаем красный
			c1.red -= step;
			if (c1.red <= 0) phase = 2;
			break;
		case 2: // Увеличиваем зелёный
			c1.green += step;
			if (c1.green >= 255) phase = 3;
			break;
		case 3: // Уменьшаем синий
			c1.blue -= step;
			if (c1.blue <= 0) phase = 4;
			break;
		case 4: // Увеличиваем красный
			c1.red += step;
			if (c1.red >= 255) phase = 5;
			break;
		case 5: // Уменьшаем зелёный
			c1.green -= step;
			if (c1.green <= 0) phase = 0;
			break;
		}
	}
	c2 = c1;
}

void dump_debug_string_array(uintptr_t idaptr, int string_num)
{
	char* ptr = (char*)IDATRANSLATE(idaptr);
	for (int i = 0; i < string_num; ++i)
	{
		int len = strlen(ptr);
		//printf("String %2d: %.*s\n", i + 1, (int)len, ptr);
		printf("%s\n", ptr);
		ptr += len;
		while (*ptr == '\0') { ++ptr; }
	}
}
// Notes:
// flags_E &= 0xFFFFFFFFFFFFFDFFui64;  turn off by1[bi1]  bUsesCollision = 0
// flags_E |= ~0xFFFFF7FFFFFFFFFFui64; turn on  by5[bi3]  bIsStaticWaitingForCollision = 1
// flags_E = flags_E & 0xFFFFFFFFFFFFBFFFui64 | 0x4000; &by1[bi6], |by1[bi6] bIsStuck = 1 (cls and set)
// if (((flags_E >> 15) & 1) == 0) by1[bi7] if(!bIsInSafePosition)

/* 575 */
enum CE_flagsnum_F // bitpos for SET_BIT
{
	CE_flags_F_0 = 0,
	bUsesCollision = 1,
	CE_flags_F_2 = 2,
	CE_flags_F_3 = 3,
	CE_flags_F_4 = 4,
	CE_flags_F_5 = 5,
	bIsStuck = 6,
	bIsInSafePosition = 7,
};

/* 576 */
enum CP_flagsnum_J // bitpos for SET_BIT
{
	CP_flags_J_0 = 0x0,
	CP_flags_J_1 = 0x1,
	CP_flags_J_2 = 0x2,
	_bIsStaticWaitingForCollision = 0x3,
	CP_flags_J_4 = 0x4,
	CP_flags_J_5 = 0x5,
	CP_flags_J_6 = 0x6,
	CP_flags_J_7 = 0x7,
};


void EmpireTest(int mode)
{
	if (mode == 1) { return; } // dll loop
	//system("cls");
	CEmpireHud* pEmpireHudInstance = pEmpireHud;
	printf("EmpireHud: 0x%p\n", pEmpireHudInstance);
	if (!pEmpireHudInstance) { return; }
	//printf("start: 0x%p  end: 0x%p\n", pEmpireHudInstance->pHudNodeArrayList, pEmpireHudInstance->pHudNodeArrayListEnd);
	tHudElement** current = EMUPOINTER<tHudElement**>(pEmpireHudInstance->hudElementList); // fixed array pointer
	tHudElement** end = EMUPOINTER<tHudElement**>(pEmpireHudInstance->hudElementListEnd);
	if (!current || !*current || !end || !*end) { return; }
	int i = 0;
	while (current != end)
	{
		//printf("curr[%d]: 0x%p\n", i, current);
		if (!current) { continue; }
		tHudElement* node = EMUPOINTER<tHudElement*>(*current); // fixing array element pointer
		if (node)
		{
			printf("-----------------------------\n");
			printf("val[%d]: 0x%p  val %d\n", i, node, node->isVisible);
			//printf("[%d]: %d\n", i, node->isVisible);
			//node->isVisible = false;

			tHudElementInfo* infoCurrent = EMUPOINTER<tHudElementInfo*>(node->pHudElementInfoList);
			tHudElementInfo* infoEnd = EMUPOINTER<tHudElementInfo*>(node->pHudElementInfoListEnd);
			printf("info: start 0x%p  end 0x%p\n", infoCurrent, infoEnd);
			int j = 0;
			while (infoCurrent != infoEnd) {
				// Print details for each tHudElementInfo
				printf("  pHudElementInfo[%d]: 0x%p\n", j, infoCurrent);
				printf("    type: %d\n",infoCurrent->type);
				printf("    pointer_textkey_field_4: 0x%p\n", EMUPOINTER<void*>(infoCurrent->pointer_textkey_field_4));
				printf("    field_8: %d\n", infoCurrent->field_8);
				printf("    some_x_field_C: %d\n", infoCurrent->some_x_field_C);
				printf("    some_y_field_10: %d\n", infoCurrent->some_y_field_10);
				printf("    field_14: %d\n", infoCurrent->field_14);
				printf("    font_style_field_18: %d\n", infoCurrent->font_style_field_18);
				printf("    float_font_field_1C: %.3f\n", infoCurrent->font_size_field_1C);
				printf("    font_crgba_col_field_20: (R: %d, G: %d, B: %d, A: %d)\n\n",
					infoCurrent->font_crgba_col_field_20.red,
					infoCurrent->font_crgba_col_field_20.green,
					infoCurrent->font_crgba_col_field_20.blue,
					infoCurrent->font_crgba_col_field_20.alpha);

				// Move to the next element in pHudElementInfoList
				++infoCurrent;
				++j;
			}
		}
		//else printf("!node\n");
		++current;
		++i;
	}
}

bool HW()
{
	//int& CRenderer_ms_nNoOfVisibleEmpires = *(int*)IDATRANSLATE(0x4CD5A0);

	//*(int*)IDATRANSLATE(0x48A1DC) = 2; // bank
	//*(int*)IDATRANSLATE(0x48A1E0) = 1; // bool mode unk
	//*(float*)IDATRANSLATE(0x48A1E4) = 2.0f; // scale
	//*(int*)IDATRANSLATE(0x48A1E8) = 0; // растояние widemodestuff
	//*(int*)IDATRANSLATE(0x48A1EC) = 1; // centre
	//*(int*)IDATRANSLATE(0x48A200) = 10; // unk

	//return true;
	return false;
}

static std::unordered_map<int, std::string> modelNames;
void LoadModelNames(const char* filename) {
	std::ifstream f(filename);
	if (!f.is_open()) {
		std::cerr << "Failed to open " << filename << "\n";
		return;
	}
	std::string line;
	while (std::getline(f, line)) {
		std::istringstream iss(line);
		int id;
		std::string name;
		if (iss >> id >> name) {
			modelNames[id] = name;
		}
	}
}

bool tmp = false;
bool tmp2 = false;
bool tmp3 = false;
bool tmp4 = false;
bool can_update = false;
int itmp = 0;
bool OnKey(int mode) // ret bool isallowhold
{
	if (!IsCurrentProcessWindowIsFocused()) { return false; }
	switch (mode)
	{
	case 0:
	{
		if (HW()) { return true; }
		//CRenderer_ms_nNoOfVisibleEmpires = 0;
		//return true; // allow hold

		//for (int32_t i = CTexListStore_ms_pTexListPool->m_nSize - 1; i >= 0; i--)
		//{
		//	TxdDef* def = (TxdDef*)CPools_GetSlot(CTexListStore_ms_pTexListPool, i, 28);
		//	RwTexDictionary* rwtex = EMUPOINTER<RwTexDictionary*>(def ? def->texDict : null);
		//	//if (def && def->texDict)
		//	if (rwtex)
		//	{
		//		printf("tex %d: RwTexDictionary 0x%p\n", i, rwtex);
		//	}
		//}
		//return false;

		//for (int32_t i = CPools_ms_pEmpirePool->m_nSize - 1; i >= 0; i--)
		//{
		//	CEntity* e = (CEntity*)CPools_GetSlot(CPools_ms_pEmpirePool, i, 352);
		//	if (e)
		//	{
		//		//SetEntityType(e, 6);
		//		printf("i %d: pe 0x%p mi %d  type %d\n", i, e, e->m_modelIndex, GetEntityType(e)); // 5
		//	}
		//}
		//return false;

		for (int32_t i = 0; i < CModelInfo_msNumModelInfos; i++)
		{
			CBaseModelInfo* mi = GetModelInfo(i); 

			//if (mi) { printf("%d %s\n", i, mi->m_name); continue; }
			//else { continue; }

			if (mi)
			{
				//printf("%d  0x%p type: %d\n", i, mi, mi->m_type);
				//printf("%d  0x%p e_enex: %d\n", i, mi, mi->e_enex); // enex
				//mi->e_enex = -1;
				//mi->e_enex = 9; // 0

			}
		}


		tmp3 = !tmp3;
		////////////////////////////////////////////////////////can_update = !can_update;
		//printf("[SPAWNER]: Enter type mi: ");
		//scanf("%hhu %hhu", &tmppedspawntype, &tmppedspawnmi);

		//gbGlassCheat = !gbGlassCheat;
		//printf("gbGlassCheat: %d\n", gbGlassCheat);
		//PatchMIPS();
		//PatchSCM();
		//PatchMIPS();
		RunTestSCM();

		if ((GetAsyncKeyState('U') & 0x8000)) { TeleportPlayer({ -932.3f, -1082.5f, 14.4f }); }

		CBaseModelInfo* mi = GetModelInfo(7508); // (empires) 7508, 7436, 7520, 7443, 7436
		if (mi) { printf("%d 0x%p\n", 7508, &mi->m_name); }

		printf("gpModelIndices: 0x%p\n", gpModelIndices);

		//LoadModelNames("C:\\ide_output.txt");
		//printf("parsed: %d\n", modelNames.size());
		//for (int i = 0; i < 300; i++)
		//{
		//	//if (gpModelIndices[i] > 0 && gpModelIndices[i] >= CModelInfo_msNumModelInfos) { break; }

		//	if (gpModelIndices[i] > 0 && gpModelIndices[i] < CModelInfo_msNumModelInfos)
		//	{
		//		auto it = modelNames.find(gpModelIndices[i]);
		//		if (it != modelNames.end())
		//		{
		//			const char* name = it->second.c_str();
		//			printf("__int16 MI_%d_%s;\n", gpModelIndices[i], name);
		//		}
		//		else
		//		{
		//			printf("__int16 MI_%d;\n", gpModelIndices[i]);
		//		}
		//	}
		//	else if (gpModelIndices[i] == -1) { printf("__int16 MI_MINUS_N%d;\n", i); }
		//	else if (gpModelIndices[i] == 0) { printf("__int16 MI_%d;\n", gpModelIndices[i]); }
		//	else
		//	{
		//		break;
		//	}

		//	//if (gpModelIndices[i] <= 0) { printf("-------------------------------------------------\n"); }
		//	//printf("gpModelIndices[%d]: %d\n", i, gpModelIndices[i]);
		//	//if(gpModelIndices[i] == -1) { printf("__int16 MI_MINUS_N%d;\n", i); }
		//	//else { printf("__int16 MI_%d;\n", gpModelIndices[i]); }
		//}
		//printf("\n");



		//printf("# sounds [ID, AccelerationSampleIndex, Bank, HornSample, HornFrequency, SirenOrAlarmSample, SirenOrAlarmFrequency, DoorType]\n\n");
		//FILE* file = fopen("C:\\Users\\Zver\\Desktop\\vehaudio.txt", "w");
		//std::vector<std::string> names = FileReadAllLines("C:\\NAMES.txt");
		//printf("names %d\n", names.size());
		//int fmi = 170;
		//for (int32_t i = 0; i < CModelInfo_msNumModelInfos; i++)
		//{
		//	CBaseModelInfo* mi = GetModelInfo(i); 

		//	//if (mi) { printf("%d %s\n", i, mi->m_name); continue; }
		//	//else { continue; }

		//	if (mi)
		//	{
		//		//printf("%d  0x%p type: %d\n", i, mi, mi->m_type);
		//		switch (mi->m_type)
		//		{
		//		case MITYPE_VEHICLE:
		//		{
		//			CVehicleModelInfo* vmi = (CVehicleModelInfo*)mi;
		//			//printf("%d  0x%p type: %d\n", i, mi, mi->m_type);
		//			//printf("mi: %d\n", i);
		//			//printf("%d, %d, %d, %d, %d, %d, %d, %d\n",
		//			//printf("%d,\t%d,\t%d,\t%d,\t%d,\t%d,\t%d,\t%d\n",
		//			//	i,
		//			//	vmi->m_nAccelerationSampleIndex,
		//			//	vmi->m_nBank,
		//			//	vmi->m_nHornSample,
		//			//	vmi->m_nHornFrequency,
		//			//	vmi->m_nSirenOrAlarmSample,
		//			//	vmi->m_nSirenOrAlarmFrequency,
		//			//	vmi->m_bDoorType);
		//			printf("%d %s\n", i, names[i - fmi].c_str());
		//			//fprintf(file, "%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n", i,
		//			fprintf(file, "%s\t\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n", names[i - fmi].c_str(),
		//				vmi->m_nAccelerationSampleIndex,
		//				vmi->m_nBank,
		//				vmi->m_nHornSample,
		//				vmi->m_nHornFrequency,
		//				vmi->m_nSirenOrAlarmSample,
		//				vmi->m_nSirenOrAlarmFrequency,
		//				vmi->m_bDoorType);

		//			//vmi->m_nHornFrequency = 24000;
		//			break;
		//		}
		//		case MITYPE_PED:
		//		{

		//			break;
		//		}
		//		}
		//	}
		//}
		//fclose(file);

		//int cnt_enginesets = 25;
		//for (size_t i = 0; i < cnt_enginesets; i++)
		//{
		//	printf("%s, %s\n", ((std::string)magic_enum::enum_name((eSfxSample)(aEngineSounds[i].val1))).c_str(),
		//		((std::string)magic_enum::enum_name((eSfxSample)(aEngineSounds[i].val2))).c_str());
		//}


		//dump_debug_string_array(0x4AB328, 71); // ped states
		//dump_debug_string_array(0x4ACAF8, 40); // personality
		//dump_debug_string_array(0x4B6588, 68); // sfx banks
		
		// dbg
		CPlayerPed* pPlayer = FindPlayerPed();
		int playerhandle = CPools_GetIndex(CPools_ms_pPedPool, pPlayer, 3360); // pool handle
		printf("player : 0x%p handle %d\n\n", pPlayer, playerhandle);

		//for (int32_t i = CPools_ms_pPedPool->m_nSize - 1; i >= 0; i--)
		//{
		//	CPed* pPed = (CPed*)CPools_GetSlot(CPools_ms_pPedPool, i, 3360);
		//	int index = CPools_GetJustIndex(CPools_ms_pPedPool, pPed, 3360); // array index (slot)
		//	int handle = CPools_GetIndex(CPools_ms_pPedPool, pPed, 3360); // pool handle
		//	CPed* pTesteddd = (CPed*)CPools_GetAt(CPools_ms_pPedPool, handle, 3360);
		//	bool isFree = CPools_GetSlotIsFree(CPools_ms_pPedPool, index);
		//	//printf("i: %d  handle %d isfree %d\n", i, handle, isFree);
		//	printf("ped[%d]: handle %d isfree %d 0x%p health %f\n\n", i, handle, isFree, pTesteddd, pTesteddd->m_fHealth);
		//	//if(!isFree) pTested->m_fHealth = 0.0f;
		//}
		//DUMPSCRVAR(1568); // on off
		//DUMPSCRVAR(1571); // num
		//DUMPSCRVAR(1572+0); // handles recruit
		//DUMPSCRVAR(1572+1);
		//DUMPSCRVAR(1572+2);
		//DUMPSCRVAR(1575);
		//DUMPSCRVAR(1576);
		//printf("\n");



		// tester ped handles
		int pedh = 0;
		printf("[~]: Enter ped handle: ");
		scanf("%d", &pedh);
		printf("[~]: h %d \n", pedh);
		CPed* pTested = (CPed*)CPools_GetAt(CPools_ms_pPedPool, pedh, 3360);
		if (pTested)
		{
			int index = CPools_GetJustIndex(CPools_ms_pPedPool, pTested, 3360); // array index (slot)
			bool isFree = CPools_GetSlotIsFree(CPools_ms_pPedPool, index);
			printf("tested : 0x%p health %f  isfree %d\n\n", pTested, pTested->m_fHealth, isFree);
			//if(!isFree) pTested->m_fHealth = 0.0f;
		}
		else { printf("!!!!!!not found\n"); }

		tSample* pSamples = EMUPOINTER<tSample*>(SampleManager->m_aSamples);
		printf("SampleManager 0x%p\n", SampleManager);
		printf("SampleManager m_aSamples 0x%p\n", pSamples);
		printf("sfxgxt 0x%p\n", gAm_sfxgxt);




		//printf("[");
		//for (size_t i = 0; i < 6; i++) { printf("%d ", gAm_sfxgxt->field_0[i]); }
		//printf("]");

		//int rndsfx = rand() % TOTAL_AUDIO_SAMPLES; // bred
		//tSample tmp = EMUPOINTER<tSample*>(SampleManager->n_pSamplesDesc)[rndsfx];
		//for (int i = 0; i < TOTAL_AUDIO_SAMPLES; i++)
		//{
		//	//printf("%d %d\n", i, EMUPOINTER<tSample*>(SampleManager->n_pSamplesDesc)[i].nOffset);
		//	//EMUPOINTER<tSample*>(SampleManager->n_pSamplesDesc)[i].nFrequency = 5000;
		//	//EMUPOINTER<tSample*>(SampleManager->n_pSamplesDesc)[i].nSize = 500000;
		//	//EMUPOINTER<tSample*>(SampleManager->n_pSamplesDesc)[i].nOffset = 500000;
		//	//EMUPOINTER<tSample*>(SampleManager->n_pSamplesDesc)[i] = tmp;
		//}

		return false;
		//---------------------------------------------------------------------------------------------------------------------------------------
		printf("\n");
		const char* desc[] = { "carIds", "boatIds", "jetskiIds", "trainIds", "heliIds", "planeIds", "bikeIds", "ferryIds", "bmxIds", "quadIds" };
		for (int i = 0; i < ARRAY_SIZE(desc); i++)
		{
			//printf("%s------\n", desc[i]);
			printf("RwObjectNameIdAssocation %s[] = {\n", desc[i]);
			RwObjectNameIdAssocation* pIdAssoc = (EMUPOINTER<RwObjectNameIdAssocation*>(CVehicleModelInfo_ms_vehicleDescs[i]));
			while(pIdAssoc && (*(uint32_t*)pIdAssoc))
			{
				//printf("\t%s\n", EMUPOINTER<char*>(pIdAssoc->name));
				printf("    { \"%s\", %d, 0x%x },\n", EMUPOINTER<char*>((char*)pIdAssoc->name), pIdAssoc->hierId, pIdAssoc->flags);
				++pIdAssoc;
			}
			++pIdAssoc;
			//printf("\n");
			printf("    { nil, 0, 0 }\n");
			printf("};\n\n");
		}

		if (FindPlayerVehicle())
		{
			CVehicle* pVeh = FindPlayerVehicle(); //						   76543210 <--   1<<(N1)
			//flags_E &= 0xFFFFFFFFFFFFFDFFui64; // 11111111 11111111 11111111 11111111 11111111 11111111 11111101 11111111

			//bool bUsesCollision = GET_BIT(pVeh->CPhysical.CEntity.CE_flags_F, CE_flags_F::bUsesCollision);
			//SET_BIT(pVeh->CPhysical.CEntity.CE_flags_F, CE_flags_F::bUsesCollision, !bUsesCollision);

			//SWAP_BIT(pVeh->CPhysical.CEntity.CE_flags_F, CE_flags_F::bUsesCollision);

			//SET_BIT(pVeh->CPhysical.CEntity.CE_flags_G, 2, 1);
			//SWAP_BIT(pVeh->CPhysical.CEntity.CE_flags_G, 2);
			//SWAP_BIT(pVeh->flags_field_265, 4);
			//SWAP_BIT(pVeh->flags_field_265, 3);
		}
		if (FindPlayerPed())
		{
			CPlayerPed* pPlayer = FindPlayerPed();
			//SWAP_BIT(pPlayer->CPed.CPhysical.CEntity.CE_flags_K, 3); // :/ not rendernotcontrol
		}

		EmpireTest(0);

		//int i = (CPools_ms_pPedPool->m_nSize - 1);
		//while (i--) {
		//	CPed* pPed = CPools::GetPedPool()->GetSlot(i);
		//}

		//printf("%p %p \n", GET_BYTE(BYTESF2U32(1000.0f), 0), GET_BYTE(BYTESF2U32(1000.0f), 1));

		printf("vehp: 0x%p\n", CPools_ms_pVehiclePool);
		printf("vehpsz: %d\n", CPools_ms_pVehiclePool->m_nSize);
		printf("vehpobj: 0x%p\n", PCSX2POINTER(CPools_ms_pVehiclePool->m_Objects));
		//for (int32_t i = CPools_ms_pVehiclePool->m_nSize - 1; i >= 0; i--)
		//{
		//	CVehicle* vehicle = (CVehicle*)CPools_GetSlot(CPools_ms_pVehiclePool, i, 2240);
		//	if (vehicle) {
		//		printf("mi %d  vehtype %d\n", vehicle->CPhysical.CEntity.m_modelIndex, vehicle->m_vehType);
		//		vehicle->m_fHealth = 0.0f;
		//	}
		//}

		for (int32_t i = CPools_ms_pPedPool->m_nSize - 1; i >= 0; i--)
		{
			CPed* pPed = (CPed*)CPools_GetSlot(CPools_ms_pPedPool, i, 3360);
			if (pPed && pPed->CPhysical.CEntity.m_modelIndex == 89) {
				printf("0x%p leader(&0x%p): 0x%p\n", pPed, &pPed->m_leader, EMUPOINTER<CPed*>(pPed->m_leader));
				printf("0x%p m_threatEntity(&0x%p): 0x%p\n\n", pPed, &pPed->m_threatEntity, EMUPOINTER<CPed*>(pPed->m_threatEntity));
				printf("\n fF1 ");
				DUMP_BITS(pPed->m_fearFlags1);
				printf("\n fF2 ");
				DUMP_BITS(pPed->m_fearFlags2);
				printf("\n fF3 ");
				DUMP_BITS(pPed->m_fearFlags3);
				printf("\n fF4 ");
				DUMP_BITS(pPed->m_fearFlags4);
				printf("\n");
			}
		}

		//for (int32_t i = 0; i < CModelInfo_msNumModelInfos; i++)
		//{
		//	if (CModelInfo_ms_modelInfoPtrs[i] && EMUPOINTER<CBaseModelInfo*>(CModelInfo_ms_modelInfoPtrs[i])->pointer_possiblename_field_C)
		//	{ printf("%d name: %s\n", i, EMUPOINTER<char*>(EMUPOINTER<CBaseModelInfo*>(CModelInfo_ms_modelInfoPtrs[i])->pointer_possiblename_field_C)); }
		//}

		//printf("171 %p\n", EMUPOINTER<void*>(EMUPOINTER<CBaseModelInfo*>(CModelInfo_ms_modelInfoPtrs[171])->m_colModel)); // MI_ADMIRAL
		//printf("246 %p\n", EMUPOINTER<void*>(EMUPOINTER<CBaseModelInfo*>(CModelInfo_ms_modelInfoPtrs[246])->m_colModel)); // MI_RHINO
		//printf("246 nb %d\n", EMUPOINTER<CColModel*>(EMUPOINTER<CBaseModelInfo*>(CModelInfo_ms_modelInfoPtrs[246])->m_colModel)->numBoxes); // MI_RHINO (3)

		//*SCRVAR(1571) = 2;

		//int index = 0;
		//CRunningScript* script = CTheScripts_pActiveScripts;
		//while (script != nil)
		//{
		//	CRunningScript* next = EMUPOINTER<CRunningScript*>(script->m_pNext);
		//	//if(script->m_abScriptName) { printf("%s\n", script->m_abScriptName); }
		//	if (!script->m_bMissionFlag) // ch_loui
		//	{
		//		printf("!!! %d %s\n", index, script->m_abScriptName);
		//		script->m_nWakeTime = INT32_MAX; // terminate
		//		++index;
		//		if (index >= 54) { break; } // n non mission
		//	}

		//	script = next;
		//	if (script && !script->m_bIsActive)
		//		script = nil;
		//}

		//for (uint32_t i = 0; i < CTheCarGenerators_NumOfCarGenerators; i++)
		//{
		//	printf("[%d (%d)]: %f %f %f\n", i,
		//		CTheCarGenerators_CarGeneratorArray[i].m_nModelIndex,
		//		CTheCarGenerators_CarGeneratorArray[i].m_vecPos.x,
		//		CTheCarGenerators_CarGeneratorArray[i].m_vecPos.y,
		//		CTheCarGenerators_CarGeneratorArray[i].m_vecPos.z);
		//}

		break;
	}
	case 1:
	{
		tmp2 = !tmp2;
		printf("now tmp2: %d\n\n", tmp2);
		//for (size_t i = 0; i < 30; i++)
		//{
		//	printf("%p ", CTheScripts_pMissionScript[i]);
		//}
		//printf("\n");
		//for (size_t i = 0; i < MAX_NUM_MISSION_SCRIPTS; i++)
		//{
		//	printf("[%d]: %d\n", i, CTheScripts_MultiScriptArray[i]);
		//}

		// ped 0x21D41320   armour 0x21D41808 off 0x4E8       health 0x21D41804  off 0x4E4
		CPed* pPed = EMUPOINTER<CPed*>(((CPlayerInfo*)CWorld_Players)[CWorld_PlayerInFocus].m_pPed);
		CVehicle* pVehicle = EMUPOINTER<CVehicle*>(pPed ? pPed->m_pMyVehicle : null);

		if (pPed)
		{
			printf("====================VFTABLE====================\n");
			printf("VFTABLE pPed: 0x%p\n", pPed->CPhysical.CEntity.vftable);
			if (pVehicle) { printf("VFTABLE pVehicle: 0x%p\n", pVehicle->CPhysical.CEntity.vftable); }
			printf("===============================================\n\n");
			//DUMPVEC("ped pos:", pPed->CPhysical.CEntity.CPlaceable.m_matrix.p);
		}

		printf("pinfpPED: 0x%p\n", pPed);
		printf("pinfpPEDVEHICLE: 0x%p\n", pVehicle);
		//printf("empireinst: 0x%p\n", (PCSX2POINTER(*(uintptr_t**)IDATRANSLATE(0x48FF48)))); // ?
		printf("EntryExitManager: 0x%p\n", EntryExitManager);
		printf("gpSkidTex: 0x%p\n", gpSkidTex);
		printf("currentTexDict: 0x%p\n", currentTexDict);
		printf("CFont::Details: 0x%p\n", CFont_Details);
		printf("EmpireMgr: 0x%p\n", EmpireMgr);
		printf("EmpireMgr->m_pEmpires: 0x%p\n", EMUPOINTER<void*>(EmpireMgr->m_pEmpiresInfos));
		//printf("EmpireMgr->pointer_field_14: 0x%p\n", EMUPOINTER<void*>(EmpireMgr->pointer_field_14));
		//printf("EmpireMgr->pointer_field_1C: 0x%p\n", EMUPOINTER<void*>(EmpireMgr->pointer_field_1C));
		//printf("EmpireMgr+0x10: 0x%p\n", EmpireMgr+0x10);
		//printf("EmpireMgr+0x144: 0x%p\n", EmpireMgr+0x144);

		printf("vehp: 0x%p\n", pVehicle?((uintptr_t)pVehicle+0x160):null);
		printf("gpStreaming: 0x%p\n", gpStreaming);
		printf("pTheCamera: 0x%p\n", TheCamera);
		//printf("pDrunkness: 0x%p\n", CMBlur_Drunkness);
		printf("pCTimer_ms_fTimeScale: 0x%p\n", CTimer_ms_fTimeScale);
		
		printf("pcpadnewstate: 0x%p\n", (&CPad_Pads[0].NewState));
		printf("pcpadnewstatershock: 0x%p\n", (&CPad_Pads[0].NewState.RightShock));
		printf("pcpadoldstatershock: 0x%p\n", (&CPad_Pads[0].OldState.RightShock));
		printf("pcpadmode: 0x%p\n", (&CPad_Pads[0].Mode));
		printf("pinfocus: %d\n", CWorld_PlayerInFocus);
		printf("pinfarr: 0x%p\n", ((CPlayerInfo*)CWorld_Players));
		printf("pinfpedpos: %f %f %f\n", pPed->CPhysical.CEntity.CPlaceable.m_pMat.pos.x,
			pPed->CPhysical.CEntity.CPlaceable.m_pMat.pos.y,
			pPed->CPhysical.CEntity.CPlaceable.m_pMat.pos.z);

		printf("pinfpedvehhea: %f\n", pVehicle ? pVehicle->m_fHealth : 0.0f);
		printf("pinfhea: 0x%p\n", &pPed->m_fHealth);
		printf("pinfarm: 0x%p\n", &pPed->m_fArmour);
		printf("pinfhea: %f\n", pPed->m_fHealth);
		printf("pinfarm: %f\n", pPed->m_fArmour);
		printf("m_pPointGunAt: 0x%p\n", EMUPOINTER<CPed*>(pPed->m_pPointGunAt));
		printf("m_nPedState: %d\n", pPed->m_nPedState);

		printf("m_animGroup: 0x%p\n", &pPed->m_animGroup);
		printf("pvalid: 0x%p\n", &pPed->m_collPoly.valid);
		printf("wanted: %d\n", ((CPlayerPed*)pPed)->m_pWanted.m_nWantedLevel);

		CRunningScript* scr = CTheScripts_pActiveScripts;
		printf("CTheScripts_ScriptSpace: 0x%p\n", CTheScripts_ScriptSpace);
		printf("CTheScripts_pMissionScript: 0x%p\n", CTheScripts_pMissionScript);
		printf("s: %s IP: %d TIMERA %d\n", scr->m_abScriptName, scr->m_nIp, scr->m_anLocalVariables[TIMERA]);
		printf("CTheScripts_MainScriptSize: %d\n", CTheScripts_MainScriptSize);
		printf("largest: %d\n", CTheScripts_LargestMissionScriptSize);
		//if (command < coms.size())
		//sprintf(tmp, "n: %s, MIP %d OP %s:0x%04X Cmp %d Not %d", m_abScriptName, oldip - mzhkstartip, coms[command].c_str(), command, m_bCondResult, m_bNotFlag);
		break;
	}
	case 2: // dbg
	{
		tmp = !tmp;
		SetPatchesState(tmp);
		//if (tmp) { patches[ePATCH1].ApplyPatch(); }
		//else { patches[ePATCH1].RemovePatch(); }
		//printf("d: %d\n", 0);
		RESET_RECOMP_EE(); // update pcsx2 cached mips
		break;
	}
	case 3:
	{
		testhpp();
		break;
	}
	}
}

void UpdNonSyncStuff()
{
	if (!can_update) { return; }
	if (!FindPlayerPed()) { return; }
	CVector pos = FindPlayerPos();
	//return; //-------------------------------------
	bool neednewline = false;
	//printf("desc 0x%p\n", EMUPOINTER<tSample*>(SampleManager->n_pSampleDesc_stuff));
	//printf("sfxgxt 0x%p\n", gAm_sfxgxt);
	if(!(GetAsyncKeyState(VK_CONTROL) & 0x8000)) system("cls");
	printf("player: 0x%p\n", FindPlayerPed());
	DUMPPOS(pos);
	printf("CGame::currArea: %d\n", CGame_currArea);
	printf("CWorld::ms_aSectors: 0x%p\n", CWorld_ms_aSectors);
	CSector* s = GetSectorByPos(pos.x, pos.y);
	if (s)
	{
		printf("sector: 0x%p\n", s);
		//CPtrList list = s->m_buildingList; // type 1
		//CPtrList list = s->m_buildingOverlapList; // type 1
		//CPtrList list = s->m_vehicleList; // type 2
		//CPtrList list = s->m_vehicleOverlapList; // type 2
		//CPtrList list = s->m_pedList; // type 3
		//CPtrList list = s->m_pedOverlapList; // type 3
		//CPtrList list = s->m_objectList; // type 4
		//CPtrList list = s->m_objectOverlapList; // type 4
		CPtrList list = s->empire; // type 5 empire
		//CPtrList list = s->empireover; // type 5? empire over
		//CPtrList list = s->m_dummyList; // type 6  (mi 578)
		//CPtrList list = s->m_dummyOverlapList; // type 6 (mi 574)

		//CPtrList list = s->m_multiplayerList; // type 7?
		//CPtrList list = s->unk3; // type 1 (mi 318) pos 0 0 0


		CPtrNode* first = EMUPOINTER<CPtrNode*>(list.first);
		if (first) { printf("first pointer: 0x%p\n", first); }
		if (first && first->item)
		{
			while (first)
			{
				CEntity* pEntity = EMUPOINTER<CEntity*>(first->item);
				CBaseModelInfo* mi = GetModelInfo(pEntity->m_modelIndex);
				printf("modelindex: %d\n", pEntity->m_modelIndex); // empire 0  mi 7436 type 5  status 4
				printf("entity enex id: %d\n", mi ? mi->e_enex : -1);
				printf("entity mi: 0x%p\n", mi);
				printf("entity type: %d\n", GetEntityType(pEntity)); // (empires) 7508, 7436, 7520, 7443, 7436
				printf("entity status: %d\n", GetEntityStatus(pEntity));
				DUMPPOS(pEntity->CPlaceable.m_pMat.pos);

				// RW RSL test
				/*RwObject* rwo = EMUPOINTER<RwObject*>(pEntity->m_rwObject);
				if (rwo)
				{
					//printf("rwobj type id: %d %s\n", rwo->type, rwo->type == rpATOMIC ? "atomic" : "clump");
					printf("rwobj type id: %d (%s)\n", rwo->type, GetRwObjectDescByType(rwo->type));

					switch (rwo->type)
					{
					case rpATOMIC:
					{
						RpAtomic* rwa = (RpAtomic*)rwo;
						RpGeometry* geo = EMUPOINTER<RpGeometry*>(rwa->geometry);
						printf("geo: 0x%p\n", geo);
						for (int i = 0; i < geo->matList.numMaterials; i++)
						{
							RpMaterial* prp = EMUPOINTER<RpMaterial*>(EMUPOINTER<RpMaterial**>(geo->matList.materials)[i]);
							printf("mat[%d] (num %d): 0x%p\n", i, geo->matList.numMaterials, prp);
							//EMUPOINTER<RpMaterial**>(geo->matList.materials)[i] = null;
							//prp->texture = null;
							//prp->color = { 255,0,0,255 };
							if (prp && prp->unk2) { printf("----------------------------\n0x%p\n\n", prp->unk2); }
						}
						//FORLIST(lnk, clump->atomics)
						//{
						//}

						break;
					}
					case rpCLUMP:
					{
						RpClump* rwc = (RpClump*)rwo;

						break;
					}
					default:
					{

						break;
					}
					}

				}*/

				//MboxSTD("find");
				//if ((GetAsyncKeyState('U') & 0x8000)) { TeleportEntity(pEntity, FindPlayerPos()); break; }
				if ((GetAsyncKeyState('U') & 0x8000)) { TeleportPlayer(*(CVector*)&pEntity->CPlaceable.m_pMat.pos); break; }
				first = EMUPOINTER<CPtrNode*>(first->next);
				printf("\n");
			}
		}
		else { printf("!listelement\n"); }
	}
	else { printf("!sector\n"); }

	if (0)
	{
		if (FindPlayerPed()) { printf("%d\n", FindPlayerPed()->CPed.m_nPedState); }
		neednewline = true;
	}

	if(0)
	{
		//system("cls"); for (size_t i = 0; i < 106; i++) { printf("%d ", CTheScripts_pActiveScripts->m_anLocalVariables[i]); } printf("\n");
		//printf("SCR_%d: %d\n", 5375, *SCRVAR(5375));
		//////printf("SCR_%d: %d\n", 5376, *SCRVAR(5376)); // r3 done
		//printf("SCR_%d: %d\n", 5414, *SCRVAR(5414));

		// todo 0457:   player $player_char aiming_at_actor $854 vc
		//DUMPSCRVAR(1568); // on off
		//DUMPSCRVAR(1571); // num
		//DUMPSCRVAR(1572+0); // handles
		//DUMPSCRVAR(1572+1);
		//DUMPSCRVAR(1572+2);
		//DUMPSCRVAR(1575);
		//DUMPSCRVAR(1576);

		// audio slots
		//DUMPSCRSTRVAR(1547);
		DUMPSCRARRAY(1542, 5); // flags 1 1 1 1 1  (1 free, 2loading??, 4 loaded/prepared4play, 8 playing)
		DUMPSCRSTRARRAY(1547, 5); // sfx str
		DUMPSCRSTRARRAY(1552, 5); // gxt str
		DUMPSCRARRAY(1557, 5); // -99 ped handle
		DUMPSCRARRAY(1562, 5); // 0 0 0 1 0 playing/busy flag
		neednewline = true;
	}

	if (neednewline) { printf("\n\n"); }
	if(0)
	{
		//printf("+0x00\t\t["); for (size_t i = 0; i < 6; i++) { printf("%d ", gAm_sfxgxt->field_0[i]); } printf("]\n");
		//printf("+0x18\t\t["); for (size_t i = 0; i < 6; i++) { printf("%d ", gAm_sfxgxt->field_18[i]); } printf("]\n");
		//printf("+0x30 peds\t["); for (size_t i = 0; i < 6; i++) { printf("%d ", gAm_sfxgxt->unk[i]); } printf("]\n");
		////printf("gxt\t["); for (size_t i = 0; i < 6; i++) { printf("%d ", gAm_sfxgxt->gxt[i]); } printf("]\n");
		//printf("+0x48 gxts\t["); for (size_t i = 0; i < 6; i++) { printf("0x%p ", EMUPOINTER<void*>(gAm_sfxgxt->gxt[i])); } printf("]\n");
		//printf("+0x60 \t\t["); for (size_t i = 0; i < 6; i++) { printf("%d ", gAm_sfxgxt->_0_field_60[i]); } printf("]\n");
		////printf("len\t["); for (size_t i = 0; i < 6; i++) { printf("%d ", gAm_sfxgxt->len_field_74[i]); } printf("]\n");
		//printf("+0x78 \t\t["); for (size_t i = 0; i < 6; i++) { printf("%d ", gAm_sfxgxt->_1_field_78[i]); } printf("]\n");
		//printf("+0x90 len\t["); for (size_t i = 0; i < 6; i++) { printf("%d ", gAm_sfxgxt->sound_length[i]); } printf("]\n");
		//printf("+0xA8 \t\t["); for (size_t i = 0; i < 6; i++) { printf("%d ", gAm_sfxgxt->_0_field_A8[i]); } printf("]\n");
		neednewline = true;
	}

	if (neednewline) { printf("\n\n"); }
	//for (size_t i = 0; i < 6; i++) {gAm_sfxgxt->unk[i] = 777; } // paused?
	//for (size_t i = 0; i < 6; i++) {gAm_sfxgxt->_1_field_78[i] = 0; } //
	//for (size_t i = 0; i < 6; i++) {gAm_sfxgxt->_0_field_60[i] = 1; } //
}


//int __fastcall sub_7DC460(void* ecx, void* edx) { int res = ((int(__thiscall*)(void*))IDA2PCSX2160(0x7DC460))(ecx); OnLoadState(); return res; }
std::vector<std::string> coms;
void PluginInit()
{
	InitConsole();
	coms = FileReadAllLines("coms.txt");
	printf("[COMS] %d\n", coms.size());
	InitPatches();
	//PatchTest();
	//patch<uint32_t>(IDA2PCSX2160(0x47A804 + 1), CalcOffset(IDA2PCSX2160(0x47A804), (uintptr_t)sub_7DC460)); // 1.6.0
	printf("MaZaHaKa PCSX2 Plugin Initialized!!\n");
}

bool gbKeyHold = false;
void PluginLoop()
{
	bool key = (GetAsyncKeyState('R') & 0x8000);
	bool key2 = (GetAsyncKeyState('G') & 0x8000);
	bool key3 = (GetAsyncKeyState('B') & 0x8000);
	bool p = (GetAsyncKeyState('P') & 0x8000);
	bool o = (GetAsyncKeyState('O') & 0x8000);
	bool d = (GetAsyncKeyState('D') & 0x8000);
	bool t = (GetAsyncKeyState('T') & 0x8000);
	bool shift = (GetAsyncKeyState(VK_SHIFT) & 0x8000);
	//bool f7 = (GetAsyncKeyState(VK_F7) & 0x8000); // f not work
	//bool f8 = (GetAsyncKeyState(VK_F8) & 0x8000); // screenshot
	if (key && !gbKeyHold)
	{
		bool allowhold = OnKey(0); // glass
		if (!allowhold) { gbKeyHold = true; }
	}
	else if (key2 && !gbKeyHold)
	{
		bool allowhold = OnKey(1);
		if (!allowhold) { gbKeyHold = true; }
	}
	else if (key3 && !gbKeyHold)
	{
		bool allowhold = OnKey(2);
		if (!allowhold) { gbKeyHold = true; }
	}
	else if (d && !gbKeyHold)
	{
		bool allowhold = OnKey(3);
		if (!allowhold) { gbKeyHold = true; }
	}
	else if (o && !gbKeyHold)
	{
		PatchTest();
		gbKeyHold = true;
	}
	else if (!key && !key2 && !key3 && !o && !d) { gbKeyHold = false; }

	//GeneratorTeleporterTick();
	if (tmp3) { EmpireTest(1); }
	UpdNonSyncStuff();
	patch<uint32_t>(SCRVAR(5547), 0); // PHI_A2 float
	//OnKey(1);
	//Patch(); // quick load reload space
	CPad_bHasPlayerCheated = false;

	//system("cls"); for (size_t i = 0; i < 106; i++) { printf("%d ", CTheScripts_pActiveScripts->m_anLocalVariables[i]); } printf("\n");
	//printf("SCR_%d: %d\n", 5375, *SCRVAR(5375));
	//////printf("SCR_%d: %d\n", 5376, *SCRVAR(5376)); // r3 done
	//printf("SCR_%d: %d\n", 5414, *SCRVAR(5414));

	// todo 0457:   player $player_char aiming_at_actor $854 vc
	//DUMPSCRVAR(1568); // on off
	//DUMPSCRVAR(1571); // num
	//DUMPSCRVAR(1572+0); // handles
	//DUMPSCRVAR(1572+1);
	//DUMPSCRVAR(1572+2);
	//DUMPSCRVAR(1575);
	//DUMPSCRVAR(1576);
	//printf("\n");

	//DumpController(&CPad_Pads[0].NewState); // re helper mazahaka
	//DumpController(&CPad_Pads[0].OldState);
	//printf("\n");

	CPlayerPed* pPed = EMUPOINTER<CPlayerPed*>(((CPlayerInfo*)CWorld_Players)[CWorld_PlayerInFocus].m_pPed);
	CVehicle* pVehicle = EMUPOINTER<CVehicle*>(pPed ? pPed->CPed.m_pMyVehicle : null);

	if (pPed)
	{
		//printf("pinfpedveh: 0x%p\n", ((CPed*)PCSX2POINTER(((CPlayerInfo*)CWorld_Players)[CWorld_PlayerInFocus].m_pPed))->m_animGroup);

		pPed->CPed.m_fHealth = 100.0f;
		pPed->CPed.m_fArmour = 100.0f;
		pPed->CPed.m_fBreath = 1.0f;
		pPed->m_fCurrentStamina = 150.0f;

		//CPlayerPed* pPlayerPed = (CPlayerPed*)pPed;
		//pPlayerPed->m_pWanted.m_nChaos = 9999;
		//pPlayerPed->m_pWanted.m_nWantedLevel = 6;
		if (pVehicle) {
			//printf("m_fMaxHealth: f %f  d %d\n", pVehicle->m_fMaxHealth, pVehicle->m_fMaxHealth);
			//printf("m_fHealth: f %f  d %d\n", pVehicle->m_fHealth, pVehicle->m_fHealth);
			//printf("m_fFireBlowUpTimer: f %f  d %d\n", pVehicle->m_fFireBlowUpTimer, pVehicle->m_fFireBlowUpTimer);
			//printf("\n");

			pVehicle->m_fHealth = pVehicle->m_fMaxHealth;
			//pVehicle->m_fFireBlowUpTimer = 0.0f;
		}
		if (p) {
			CPlayerPed* pPlayerPed = (CPlayerPed*)pPed;
			pPlayerPed->m_pWanted.m_nChaos = 0;
			pPlayerPed->m_pWanted.m_nWantedLevel = 0;
			// teleport CRadar::TargetMarkerPos  CRadar::TargetMarkerId
			CVector pos = {0, 0, 0};
			//((CPed*)PCSX2POINTER(((CPlayerInfo*)CWorld_Players)[CWorld_PlayerInFocus].m_pPed))->CPhysical.CEntity.CPlaceable.m_matrix.p = pos;
		}
	}

	if (d && shift) // spawn
	{
		uint8_t mi = 0;
		printf("Enter CarSpawner MI: ");
		fflush(stdin);
		scanf("%hhu", &mi);
		printf("OK! MI: %hhu\n", mi);
		SetCarSpawnerID(mi);
	}

	if (t && shift) // teleport
	{
		CVector pos;
		printf("Enter tp (x y z): ");
		//std::cin.ignore();
		scanf("%f %f %f", &pos.x, &pos.y, &pos.z);
		TeleportPlayer(pos);
	}

	//ProcessPrekol(tmp2);
	if (tmp2)
	{
		for (int32_t i = CPools_ms_pVehiclePool->m_nSize - 1; i >= 0; i--)
		{
			CVehicle* vehicle = (CVehicle*)CPools_GetSlot(CPools_ms_pVehiclePool, i, 2240);
			//if (vehicle && (vehicle->field_170 || vehicle->field_174) && vehicle->CPhysical.CEntity.m_modelIndex == 219) {
			//	CVector pv = *(CVector*)&vehicle->CPhysical.CEntity.CPlaceable.m_pMat.pos;
			//	CVector pp = FindPlayerPos();
			//	float diff = 5.0f;

			//	float dx = pv.x - pp.x;
			//	float dy = pv.y - pp.y;
			//	float dz = pv.z - pp.z;
			//	if (dx * dx + dy * dy + dz * dz <= diff * diff) { // magnitude
			//		printf("mi %d 0x%p vehtype %d 70 %p  74 %p  ", vehicle->CPhysical.CEntity.m_modelIndex, vehicle, vehicle->m_vehType, vehicle->field_170, vehicle->field_174);
			//		DUMPVEC("pos: %f %f %f\n", vehicle->CPhysical.CEntity.CPlaceable.m_pMat.pos);
			//		printf("\n");
			//		//vehicle->m_fHealth = 0.0f;
			//	} // 8   0x21D99840+0x170   0x21EA2900   0x21E8CC40

			//}
		}
		//if (pVehicle)
		//{// todo cauto dest autopiloc cvec16?
		//	system("cls");
		//	//DUMPVEC("TS:", pVehicle->CPhysical.m_vecTurnSpeed);//+++++++++
		//	DUMPVEC("TF:", pVehicle->CPhysical.m_vecTurnFriction);
		//	//DUMPVEC("MS:", pVehicle->CPhysical.m_vecMoveSpeed);//+++++
		//	//ms avg?
		//	DUMPVEC("mf:", pVehicle->CPhysical.field_80);
		//	printf("\n");
		//}
		//if (FindPlayerPed()) { printf("%f\n", FindPlayerPed()->m_fBreath); }
		//if (FindPlayerPed()) { printf("%d\n", ((CPlayerInfo*)CWorld_Players)[CWorld_PlayerInFocus].field_F8); }
	}

	//if (FindPlayerVehicle())
	//{
	//	CVehicle* pVeh = FindPlayerVehicle();
	//	bool bIsStuck = GET_BIT(pVeh->CPhysical.CEntity.CE_flags_F, CE_flags_F::bIsStuck);
	//	if (bIsStuck) { printf("STUCK!!!\n"); }
	//}

}

void inline WaitElf() { while (!(is_valid_pointer((void*)P_PCSX2_BASE) && ((uint32_t*)P_PCSX2_BASE)[0])) { Sleep(1); } }

DWORD CALLBACK ThreadEntry(LPVOID)
{
	WaitElf();
	PluginInit();
	while (1)
	{
		PluginLoop();
		Sleep(1);
	}
	return TRUE;
}

BOOL WINAPI DllMain(_In_ HINSTANCE hinstDLL, _In_ DWORD fdwReason, _In_ LPVOID lpvReserved)
{
	if (fdwReason == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls(hinstDLL);
		CreateThread(NULL, 0, ThreadEntry, NULL, 0, NULL);
	}
	return TRUE;
}