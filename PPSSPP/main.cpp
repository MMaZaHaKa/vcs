#include "Windows.h"
#include "assert.h"
#include <iostream>
#include <stdio.h>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <algorithm>
#include <fstream>
#include <cstdint>
#include <cstring>
#include <iomanip>


#define null NULL
#define nil NULL

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
std::vector<MemoryRegion> inline FindRegions(SIZE_T targetSize = 0, DWORD targetType = 0, DWORD targetProtect = 0, DWORD targetState = MEM_COMMIT) {
	std::vector<MemoryRegion> regions;
	MEMORY_BASIC_INFORMATION mbi;
	uintptr_t address = 0;
	while (VirtualQuery((LPCVOID)address, &mbi, sizeof(mbi)) != 0) {
		if (targetSize != 0 && mbi.RegionSize != targetSize) { address += mbi.RegionSize; continue; }
		if (targetType != 0 && mbi.Type != targetType) { address += mbi.RegionSize; continue; }
		if (targetProtect != 0 && mbi.Protect != targetProtect) { address += mbi.RegionSize; continue; }
		if (targetState != 0 && mbi.State != targetState) { address += mbi.RegionSize; continue; }
		regions.push_back({ mbi.BaseAddress, mbi.RegionSize });
		address += mbi.RegionSize;
	}
	return regions;
}

void* SearchPointerByPattern(void* ptrStart, int block_size, std::string pattern)
{
#define INRANGE(x, a, b) (x >= a && x <= b)
#define getBits(x) (INRANGE((x & (~0x20)), 'A', 'F') ? ((x & (~0x20)) - 'A' + 0xa) : (INRANGE(x, '0', '9') ? x - '0' : 0))
#define getByte(x) (getBits(x[0]) << 4 | getBits(x[1]))
	const char* buffptr_pattern = pattern.c_str();
	uintptr_t pMatch = 0;
	for (uintptr_t MemPtr = (uintptr_t)ptrStart; MemPtr < ((uintptr_t)ptrStart + block_size); MemPtr++)
	{
		if (!*buffptr_pattern) { break; }
		if (*(PBYTE)buffptr_pattern == '\?' || *(BYTE*)MemPtr == getByte(buffptr_pattern))
		{
			if (!pMatch) { pMatch = MemPtr; }
			if (!buffptr_pattern[2]) { break; } // паттерн закончился
			//PWORD первых 2 символа из паттерна, PBYTE первый символ
			if (*(PWORD)buffptr_pattern == '\?\?' || *(PBYTE)buffptr_pattern != '\?') { buffptr_pattern += 3; }
			else { buffptr_pattern += 2; } //one ?
		}
		else
		{ // срыв совпадения
			buffptr_pattern = pattern.c_str();
			if (pMatch) { MemPtr = pMatch; }
			pMatch = 0;
		}
	}
	//free((void*)buffptr_pattern); // GetProcAddressNoExternCExport
	if (!pMatch) { return NULL; }
	//printf("found str: 0x%p\n", (char*)pMatch);
	return (void*)pMatch;
#undef getByte;
#undef getByte;
#undef INRANGE;
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
#define GET_BITS(value, mask, shift) (((value) & (mask)) >> (shift))
#define GET_BIT(num, n) (((num) >> (n)) & 1) // 11111111(3) 11111111(2) 11111101(1) 11111111(0) mem(FF FD FF FF), 32t_byte1, bit[76543210] <- 1<<(N1)
#define SET_BIT(num, n, val) ((num) = ((num) & ~BIT(n)) | ((val) << (n)))
#define GET_BYTE(num, n) ((num >> (8 * n)) & 0xFF) // 0-lob, 1-midlob, 2-midhib, 3-hib BYTE[0123] (x86 Little Endian)
#define SET_BYTE(num, n, val) ((num) = ((num) & ~(0xFF << (8 * (n)))) | ((val) << (8 * (n))))
#define SWAP_BIT(num, n) SET_BIT(num, n, !GET_BIT(num, n))
#define DUMP_BITS(num) (printf("%d%d%d%d%d%d%d%d\n", GET_BIT(num, 7), GET_BIT(num, 6), \
	GET_BIT(num, 5), GET_BIT(num, 4), GET_BIT(num, 3), GET_BIT(num, 2), GET_BIT(num, 1), GET_BIT(num, 0)))
#define DUMP_BITS2(num) (printf("0_[%d], 1_[%d], 2_[%d], 3_[%d], 4_[%d], 5_[%d], 6_[%d], 7_[%d]\n", GET_BIT(num, 7), GET_BIT(num, 6), \
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
//template<typename T> T inline EMUPOINTER(void* p) { return (T)(p ? PCSX2POINTER(p) : null); } // need?


#define RWRGBAINT(r, g, b, a) ((uint32)((((a)&0xff)<<24)|(((b)&0xff)<<16)|(((g)&0xff)<<8)|((r)&0xff)))
#define PI (3.1415f)
#define TWOPI (PI * 2)
#define HALFPI (PI / 2)
#define DEGTORAD(x) ((x) * PI / 180.0f)
#define RADTODEG(x) ((x) * 180.0f / PI)

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
#define SET_VAL_NOVP patch
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


#define is_valid_pointer(a) 1
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

std::vector<uint8_t> FileReadAllBytes(std::string filePath)
{
	std::vector<uint8_t> buffer;
	std::ifstream file(filePath, std::ios::binary | std::ios::ate);
	if (!file.is_open()) { return buffer; }
	std::streamsize size = file.tellg();
	buffer.resize(size);
	file.seekg(0, std::ios::beg);
	if (!file.read((char*)(buffer.data()), size)) { return buffer; }
	return buffer;
}

bool FileWriteAllBytes(std::string filePath, std::vector<uint8_t>& buffer)
{
	std::ofstream file(filePath, std::ios::binary);
	if (!file.is_open()) { return false; }
	file.write((char*)(buffer.data()), buffer.size());
	return file.good();
}

bool FileWriteAllBytes(std::string filePath, const uint8_t* buffer, uint32_t size)
{
	std::ofstream file(filePath, std::ios::binary);
	if (!file.is_open()) { return false; }
	file.write((char*)(buffer), size);
	return file.good();
}

void U_SetCurrentDirectory()
{
	char currentDir[MAX_PATH]; // dynamic set curr dir to exe
	GetModuleFileNameA(NULL, currentDir, MAX_PATH);
	std::string::size_type pos = std::string(currentDir).find_last_of("\\/");
	SetCurrentDirectoryA(std::string(currentDir).substr(0, pos).c_str());
}

void PrintCurrDir()
{
	char buffer[MAX_PATH];  // MAX_PATH is typically 260
	DWORD length = GetCurrentDirectoryA(MAX_PATH, buffer);
	if (!length || length > MAX_PATH) { return; }
	printf("Current directory: %s\n", buffer);
}

float GetRandomFloatInc(float low, float high) {
	if (low == high) return low;
	if (low > high) std::swap(low, high);
	float t = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
	return low + t * (high - low);
}

inline bool BTN(int c) { return (GetAsyncKeyState(c) & 0x8000); }

// MEM_MAPPED -> SPACE - > 1ST BIN BYTE
#define ELF_BASE_OFFSET (0x1C004000 - 0x1B800000) // 1st bytes bin in ida - MEM_MAPPED base = StartOffset
#define PSP_BASE 0x08804000 // starts from
uintptr_t PPSSPP_BASE = 0; // pointer to 1st byte elf
#define PSPPOINTER(p)  ( (p) ? ((((uintptr_t)p) - PSP_BASE) + PPSSPP_BASE) : null ) // pPSP(ida) -> pPSP(win) (null saving)
#define PSPTRANSLATE(p) ( (p) ? ((((uintptr_t)p) - PSP_BASE) - PPSSPP_BASE) : null ) // pPSP(win) -> pPSP(ida) (null saving)
#define IDATRANSLATE(p) ((((uintptr_t)p) - PSP_BASE) + PPSSPP_BASE) /*PSPPOINTER(p)*/ // null non save

//template<typename T> T inline EMUPOINTER(void* p) { return (T)(p ? PSPPOINTER(p) : null); } // need?
template<typename T> T inline EMUPOINTER(void* p) { return (T)(PSPPOINTER(p)); } // need? moved to define
template<typename T> T inline EMUPOINTER(uintptr_t p) { return EMUPOINTER<T>((void*)p); } // need?


/* 507 */
struct CControllerState
{
	__int16 LeftStickX;
	__int16 LeftStickY;
	__int16 RightStickX;
	__int16 RightStickY;
	__int16 LeftShoulder1;
	__int16 LeftShoulder2;
	__int16 RightShoulder1;
	__int16 RightShoulder2;
	__int16 DPadUp;
	__int16 DPadDown;
	__int16 DPadLeft;
	__int16 DPadRight;
	__int16 unk_nipple_up_field_18;
	__int16 unk_nipple_down_field_1A;
	__int16 unk_nipple_left_field_1C;
	__int16 unk_nipple_right_field_1E;
	__int16 Start;
	__int16 Select;
	__int16 Square;
	__int16 Triangle;
	__int16 Cross;
	__int16 Circle;
	__int16 LeftShock;
	__int16 RightShock;
};
struct CPad
{
	__int16 field_0;
	CControllerState NewState;
	CControllerState OldState;
	char field_62[48];
	__int16 Mode;
	__int16 field_94;
	__int16 DisablePlayerControls;
	char field_98[13];
	char KeyBoardCheatString[12];
	char field_B1[31];
	float field_D0;
};
struct CVuVector
{
	float x;
	float y;
	float z;
	float w;
};
struct __declspec(align(4)) RwV3d
{
	float x;
	float y;
	float z;
};
struct RwMatrix
{
	RwV3d right;
	int field_C;
	RwV3d up;
	int field_1C;
	RwV3d forvard_at;
	int field_2C;
	RwV3d pos;
	int field_3C;
};
void inline SetCVector4VU(CVuVector* p1, CVuVector* p2) { if (p1 && p2) { memcpy(p1, p2, 3 * 4); } }
void inline SetRWV3D(RwV3d* p1, CVuVector* p2) { memcpy(p1, p2, 3 * 4); }
struct CMatrix
{
	CVuVector right;
	CVuVector at_forward;
	CVuVector up;
	CVuVector pos;
	RwMatrix* m_pRwMat;
};
struct CPlaceable
{
	CMatrix m_pMat;
};
struct __declspec(align(4)) RwLLLink
{
	RwLLLink* next;
	RwLLLink* prev;
};
struct RwLinkList
{
	RwLLLink link;
};
struct RwObject
{
	char type;
	char subType;
	char flags;
	char privateFlags;
	RwObject* parent;
};
struct RwObjectHasFrame
{
	RwObject object;
	RwLLLink lFrame;
	int sync;
};
struct ClumpExt
{
	int visibilityCallBack;
	int alpha;
};
struct CRGBA
{
	unsigned __int8 red;
	unsigned __int8 green;
	unsigned __int8 blue;
	unsigned __int8 alpha;
};
struct RwTexDictionary;
struct RwRaster
{
	int unk1;
	int unk2;
	char* data;
	int flags;
};
struct RwTexture
{
	RwRaster* raster;
	RwTexDictionary* dict;
	RwLLLink lInDictionary;
	char name[32];
	char mask[32];
	int refCount; // ?
	int field_54; // ?
};
struct RpMaterial
{
	RwTexture* texture;
	CRGBA color;
	__int16 refCount;
	__int16 pad;
	int unk2;
};
struct RpMaterialList
{
	RpMaterial** materials;
	int numMaterials;
	int space;
};
struct sPspGeometry
{
	int size;
	int flags;
	int numStrips;
	int unk1;
	float bound[4];
	float scale[3];
	int numVerts;
	float pos[3];
	int unk2;
	int offset;
	float unk3;
};
struct sPspGeometryMesh
{
	int offset;
	__int16 numTriangles;
	__int16 matID;
	float unk1;
	float uvScale[2];
	float unk2[4];
	float unk3;
	char bonemap[8];
};
struct RpGeometry
{
	RwObject object;
	__int16 refCount;
	__int16 pad1;
	RpMaterialList matList;
	int pSkinPlg;
	int pad2;
	sPspGeometry msPspGeometry;
	sPspGeometryMesh msPspGeometryMesh;
};
struct CAnimBlendClumpData
{
	//CAnimBlendLink link;
	//int numFrames;
	//CVuVector* velocity2d;
	//AnimBlendFrameData* frames;
};
struct RpClump
{
	RwObject object;
	RwLinkList atomicList;
	ClumpExt ClumpExt;
	CAnimBlendClumpData* pClumpAnimDataPlugin;
};
#define RpAtomic_fromClump(ptr) ( (RpAtomic*) (((uint8_t*)ptr) - 0x1C) ) // RslElementGroupForAllElements + inElementGroupLink__inClump: link ptr - 0x1C = pAtomic*
struct AtomicExt
{
	__int16 modelId;
	__int16 flags;
};
struct HAnimNodeInfo
{
	char field_0;
	char field_1;
	char flags;
	char field_3;
	int field_4;
};
struct RpHAnimHierarchy
{
	int flags;
	int numNodes;
	int pCurrentAnim;
	int currentTime;
	int pNextFrame;
	int pAnimCallBack;
	int pAnimCallBackData;
	int animCallBackTime;
	int pAnimLoopCallBack;
	int pAnimLoopCallBackData;
	int pMatrixArray;
	int pMatrixArrayUnaligned;
	HAnimNodeInfo* pNodeInfo;
	int field_34;
};
struct RpAtomic
{
	RwObjectHasFrame object;
	RpGeometry* geometry;
	RpClump* clump;
	RwLLLink inElementGroupLink;
	void* renderCallBack;
	AtomicExt AtomicExt;
	RpHAnimHierarchy* pAtomicHAnimHierarchyPlugin;
	int field_30;
};
enum RwObjectType
{
	rpFRAME = 0,
	rpATOMIC = 1,
	rpCLUMP = 2,
	rpLIGHT = 3,
	rpCAMERA = 4,
	rp5 = 5,
	rpTEXDICTIONARY = 6,
	rpWORLD = 7,
	rpGEOMETRY = 8,
};
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
union uRslObjects
{
	RwObject* m_rwObject;
	RpClump* m_rpClump;
	RpAtomic* m_rpAtomic;
};
struct CEntity
{
	CPlaceable CPlaceable;
	char CE_flags_A;
	char CE_flags_B;
	char CE_flags_C;
	char CE_flags_D;
	char _CE_flags_E;
	char CE_flags_F;
	char CE_flags_G;
	char CE_flags_H;
	char CE_flags_I;
	char CE_flags_J;
	char CE_flags_K;
	char CE_flags_L;
	//RpAtomic* m_rwObject;
	uRslObjects m_urwObject;
	__int16 m_scanCode;
	__int16 m_modelIndex;
	__int16 m_modelIndex2;
	char flags_field_5A;
	char m_lastWepDam;
	void* vftable;
};

/* 486 */
struct CPlayerInfo
{
	CEntity* m_pPed;
	CEntity* m_pRemoteVehicle;
	char field_8[180];
	int m_nMoney;
	char pppaaaddd[28];
	int m_pHooker;
	char field_E0[24];
	int field_F8;
	char field_FC[4];
	int m_nTimeTankShotGun;
	char field_104[140];
}; // size done



#define CWorld_Players ((void*)IDATRANSLATE(0x08BDE4B0))
#define CWorld_PlayerInFocus ((*(uint8_t*)IDATRANSLATE(0x08BB3BD8)))
#define CPad_Pads ((CPad*)IDATRANSLATE(0x08BDE610))



int GetEntityType(CEntity* pEntity) { int m_type = ((pEntity->_CE_flags_E >> 1) & 0x07); return m_type; }
void SetEntityType(CEntity* pEntity, int type) { pEntity->_CE_flags_E &= ~(0x07 << 1); pEntity->_CE_flags_E |= (type & 0x07) << 1; }
int GetEntityStatus(CEntity* pEntity) { int m_status = ((pEntity->_CE_flags_E >> 4) & 0x0F) | ((pEntity->CE_flags_F & 0x01) << 4); return m_status; }
void SetEntityStatus(CEntity* pEntity, int st)
{
	pEntity->_CE_flags_E &= ~(0x0F << 4); pEntity->_CE_flags_E |= (st & 0x0F) << 4; pEntity->CE_flags_F &= ~0x01; pEntity->CE_flags_F |= (st >> 4) & 0x01;
}

void TeleportEntity(CEntity* pE, CVuVector pos, bool updrw = true)
{
	//if (pE) { pE->CPlaceable.m_pMat.pos.x = pos.x; pE->CPlaceable.m_pMat.pos.y = pos.y; pE->CPlaceable.m_pMat.pos.z = pos.z; }
	if (pE) { SetCVector4VU(&pE->CPlaceable.m_pMat.pos, &pos); }
	if (pE && updrw && pE->CPlaceable.m_pMat.m_pRwMat) { SetRWV3D(&pE->CPlaceable.m_pMat.m_pRwMat->pos, &pos); } // todo update rw stuff
}
//inline CBaseModelInfo* GetModelInfo(int index) { return EMUPOINTER<CBaseModelInfo*>(CModelInfo_ms_modelInfoPtrs[index]); }
CEntity* FindPlayerPed() { return EMUPOINTER<CEntity*>(((CPlayerInfo*)CWorld_Players)[CWorld_PlayerInFocus].m_pPed); }
//CEntity* FindPlayerVehicle() {
//	CPed* pPed = EMUPOINTER<CPed*>(((CPlayerInfo*)CWorld_Players)[CWorld_PlayerInFocus].m_pPed);
//	return EMUPOINTER<CVehicle*>(pPed ? pPed->m_pMyVehicle : null);
//}
//CVector FindPlayerPos() { return FindPlayerPed() ? (*(CVuVector*)&FindPlayerPed()->CPed.CPhysical.CEntity.CPlaceable.m_pMat.pos) : CVector{ 0, 0, 0 }; }
//CVector FindPlayerMenuTarget() {
//	CVuVector pos = { 0, 0, 0 };
//	for (int i = 0; i < 75; i++) { // NUMRADARBLIPS
//		if (TheRadar->ms_RadarTrace[i].m_eRadarSprite == eRadarSprite::RADAR_SPRITE_MP_OBJECTIVE) { pos = TheRadar->ms_RadarTrace[i].m_vecPos; break; }
//	}
//	return { pos.x, pos.y, (pos.z == 0.0f && pos.x != 0.0f && pos.y != 0.0f) ? 12.0f : pos.z };
//}

void UpdPad()
{
	bool w = BTN('W');
	bool s = BTN('S');
	bool a = BTN('A');
	bool d = BTN('D');
	//bool esc = BTN(VK_ESCAPE); // used by ppsspp
	bool esc = BTN('R');

	int nPad = 0;
	CPad_Pads[nPad].NewState.DPadUp = w ? 0xFF : 0;
	CPad_Pads[nPad].NewState.DPadDown = s ? 0xFF : 0;
	CPad_Pads[nPad].NewState.DPadLeft = a ? 0xFF : 0;
	CPad_Pads[nPad].NewState.DPadRight = d ? 0xFF : 0;
	CPad_Pads[nPad].NewState.Start = esc ? 0xFF : 0;

	CPad_Pads[nPad].OldState.DPadUp = w ? 0xFF : 0;
	CPad_Pads[nPad].OldState.DPadDown = s ? 0xFF : 0;
	CPad_Pads[nPad].OldState.DPadLeft = a ? 0xFF : 0;
	CPad_Pads[nPad].OldState.DPadRight = d ? 0xFF : 0;
	CPad_Pads[nPad].OldState.Start = esc ? 0xFF : 0;

	int* pUpdJal = EMUPOINTER<int*>(0x0898B45C);
	*pUpdJal = 0; // disable pad update, todo reset recompiler cache?
}

void UpdMP()
{
	// dis timeout
}

void UpdTest1()
{
	CEntity* pPlayer = FindPlayerPed();
	printf("pPlayer: 0x%p\n", pPlayer);
	if (!pPlayer)
		return;
	RpClump* pClump = EMUPOINTER<RpClump*>(pPlayer->m_urwObject.m_rpClump);
	//printf("rwobj type id: %d %s\n", rwo->type, rwo->type == rpATOMIC ? "atomic" : "clump");
	printf("pClump: 0x%p %s\n", pClump, GetRwObjectDescByType(pClump->object.type));


	RwLLLink* head = &pClump->atomicList.link;
	//debug("head : 0x%p\n", head); // field clump
	RpAtomic a;
	assert(((uint8_t*)&a) == ((uint8_t*)&a.inElementGroupLink - 0x1C)); // stru test

	int i = 0;
	//for (RwLinkList* link = EMUPOINTER<RwLinkList*>(clump->atomicList.link.next); link; link = EMUPOINTER<RwLinkList*>(link->link.next)) // wrong looping
	for (RwLLLink* link = EMUPOINTER<RwLLLink*>(head->next); link != head; link = EMUPOINTER<RwLLLink*>(link->next)) // FORLIST
	{
		RpAtomic* atomic = RpAtomic_fromClump(link); // FORLIST
		//printf("ATOMIC!!! [%d] : 0x%p   lnk 0x%p\n", i++, atomic, link);
		printf("atomic: 0x%p %s\n", atomic, GetRwObjectDescByType(atomic->object.object.type));
		RpGeometry* geo = EMUPOINTER<RpGeometry*>(atomic->geometry);
		//float s = GetRandomFloatInc(0.5f, 2.5f);
		float s = 1.2f;
		//geo->msPspGeometry.scale[0] = s;
		//geo->msPspGeometry.scale[1] = s;
		//geo->msPspGeometry.scale[2] = s;
		//geo->msPspGeometryMesh.matID -= -1;
		geo->msPspGeometry.numStrips = 5;
		//geo->msPspGeometry.pos[0] = 2.3f;
		//geo->msPspGeometry.pos[1] = 2.3f;
		//geo->msPspGeometry.pos[2] = 2.3f;
		geo->msPspGeometryMesh.numTriangles = 0;
		printf("geo->msPspGeometry: 0x%X\n", &geo->msPspGeometry);
	}



	//{
	//	for (int32_t i = CPools_ms_pPedPool->m_nSize - 1; i >= 0; i--)
	//	{
	//		if (CPools_GetSlotIsFree(CPools_ms_pPedPool, i)) { continue; }
	//		CPed* p = (CPed*)CPools_GetSlot(CPools_ms_pPedPool, i, 3360);
	//		if (p) {
	//			int32_t mindex = p->CPhysical.CEntity.m_modelIndex;
	//			CBaseModelInfo* mi = GetModelInfo(mindex);
	//			printf("ped[%d]: 0x%p, mi: 0x%p\n", p->CPhysical.CEntity.m_modelIndex, p, mi);
	//			// test code
	//			{
	//				int8_t& b1D8 = *OFFSET(p, 0x1D8, int8_t*);
	//				SWAP_BIT(b1D8, 3);
	//				//bool IsDBY = b1D8 & BIT(3);
	//				//b1D8 = IsDBY ? (b1D8 & (~BIT(3))) : (IsDBY | BIT(3));
	//			}
	//		}
	//	}
	//}
}

bool quit = false;
void PluginInit()
{
	U_SetCurrentDirectory();
	InitConsole();

}

void PluginLoop()
{
	quit = BTN('Q');
	//if (BTN('R'))
	if (!PPSSPP_BASE) // todo OnLoadGame/State Event!!!!
	{ // temp hack to find region
		//Sleep(1);
		//Sleep(500);
		std::vector<MemoryRegion> regions = FindRegions(0, MEM_MAPPED, 0, 0);
		//printf("total find: %d\n", regions.size());
		for (int32_t i = 0; i < regions.size(); i++) {
			//printf("%d  0x%X  %d\n", i, regions[i].baseAddress, regions[i].size);
			if ((i - 3) < regions.size() && regions[i].size == 32505856 && regions[i + 1].size == 32505856 && regions[i + 2].size == 2097152) {
				PPSSPP_BASE = (uintptr_t)regions[i].baseAddress + ELF_BASE_OFFSET;
				break;
			}
		}
	}
	if (!PPSSPP_BASE)
		return;
	//printf("find: 0x%X\n", PPSSPP_BASE);
	if (strcmp(EMUPOINTER<char*>(0x08BB153C), "ULUS10160") != 0) { // ppsspp disable asserts
		printf("assert failed\n");
		return;
	}

	UpdPad();
	UpdMP();
	UpdTest1();

	//printf("%s\n", PSPPOINTER(0x08BB153C));
	//printf("%s\n", PSPPOINTER(0x08BAD5C0));
	Sleep(1);
}

DWORD CALLBACK ThreadEntry(LPVOID)
{
	if(BTN(VK_SHIFT))
		return TRUE;

	//WaitElf();
	PluginInit();
	while (!quit)
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