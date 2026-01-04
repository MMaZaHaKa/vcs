#include "Windows.h"
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
#include "mips.hpp"

#include "hdr.h"
#include "test.hpp"
#include "AudioSamples.h"
#include "tools/magic_enum/magic_enum.hpp"

#define null NULL
#define nil NULL

#define rwVENDORID_ROCKSTAR 0x0253F2

#define Max(a, b) ((a) > (b) ? (a) : (b))
#define Min(a, b) ((a) < (b) ? (a) : (b))

// Use this to add const that wasn't there in the original code
#define Const const

typedef uint8_t uint8;
typedef int8_t int8;
typedef uint16_t uint16;
typedef uint16 float16;
typedef int16_t int16;
#ifndef __MWERKS__
typedef uint32_t uint32;
typedef int32_t int32;
#else
typedef unsigned int uint32;
typedef int int32;
#endif
typedef uintptr_t uintptr;
typedef intptr_t intptr;
typedef uint64_t uint64;
typedef int64_t int64;
// hardcode ucs-2
typedef char16_t wchar;

typedef uint8 bool8;
typedef uint16 bool16;
typedef uint32 bool32;

#if defined(_MSC_VER) || defined(__MWERKS__)
typedef uint8 u8;
typedef uint16 u16;
typedef uint32 u32;
typedef uint64 u64;
typedef int8 i8;
typedef int16 i16;
typedef int32 i32;
typedef int64 i64;
#endif

typedef uintptr_t uintptr;


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

// 0x20100000 1st bytes *.elf binary
// 0x20000000 + poiner -> physical address
#define P_PCSX2_BASE 0x20000000
#define P_PCSX2_END  0x21FFFFFF
#define P_ELF_BASE  0x00100000 // P_PCSX2_BASE + P_ELF_BASE = 1st byte elf program
//#define IDATRANSLATE(p) (((uintptr_t)p) + P_PCSX2_BASE) // IDA -> PCSX2
#define PCSX2POINTER(p)  ( (p) ? (((uintptr_t)p) + P_PCSX2_BASE) : null ) // pPS2(ida) -> pPCSX2(win) (null saving)
#define PCSXTRANSLATE(p) ( (p) ? (((uintptr_t)p) - P_PCSX2_BASE) : null ) // pPCSX2(win) -> pPS2(ida) (null saving)
#define IDATRANSLATE(p) (((uintptr_t)p) + P_PCSX2_BASE) /*PCSX2POINTER(p)*/ // null non save
void* gpPCSX2base = NULL;
uint32_t inline TranslatePCSX2IDAPTR(uint32_t idap) {
	if (!gpPCSX2base) { gpPCSX2base = FindRegions(0x6C4000, MEM_IMAGE, 0, MEM_COMMIT)[0].baseAddress; }
	return (idap - 0x401000) + (uint32_t)gpPCSX2base;
}
#define IDA2PCSX2160(p) TranslatePCSX2IDAPTR((uint32_t)p) // pcsx2 asm (patch pcsx2)
#define RESET_RECOMP_EE() { auto recResetEE = (void(__stdcall*)())IDA2PCSX2160(0x665570); recResetEE(); } // reset recompiler EE (prevent cached exec)(4patch)
bool is_valid_pointer(void* ptr)
{
	if (!ptr) { return false; }
	MEMORY_BASIC_INFORMATION mbi;
	if (VirtualQuery(ptr, &mbi, sizeof(mbi)) == 0) { return false; }
	return (mbi.State == MEM_COMMIT) && !(mbi.Protect & PAGE_NOACCESS);
}

bool inline WaitElf(bool check = false) {
	if (check) { return (is_valid_pointer((void*)P_PCSX2_BASE) && ((uint32_t*)P_PCSX2_BASE)[0]); }
	while (!(is_valid_pointer((void*)P_PCSX2_BASE) && ((uint32_t*)P_PCSX2_BASE)[0])) { Sleep(1); }
}
// TODO ADD SUPPORT PCSX2 pnach (need?)
bool& eeCpuExecuting = *(bool*)IDA2PCSX2160(0x02FD4EB8);
bool& eeRecNeedsReset = *(bool*)IDA2PCSX2160(0x02FD4E13);
//auto fdbg = (void(__stdcall*)())IDA2PCSX2160(0x665490);
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
#define SWAP_BIT(num, n) SET_BIT(num, n, !GET_BIT(num, n)) // flags ^= (1 << 3);
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
#define OFFSET(base, off, type) ((type)&(((uint8_t*)base)[off]) )
#define TEST_OFFSET(obj, field, offset) assert(((uint8_t*)&obj) == (((uint8_t*)&field) - offset))
#define TEST_POFFSET(pobj, pfield, offset) assert(((uint8_t*)pobj) == (((uint8_t*)pfield) - offset))
// PDP-10 like byte functions
#define MASK(p, s) (((1 << (s)) - 1) << (p))
inline uint32_t dpb(uint32_t b, uint32_t p, uint32_t s, uint32_t w) { uint32_t m = MASK(p, s); return (w & ~m) | ((b << p) & m); } // Deposit Bit Field
inline uint32_t ldb(uint32_t p, uint32_t s, uint32_t w) { return w >> p & (1 << s) - 1; } // Load Bit Field
#define INRANGE(x,a,b) (x >= a && x <= b) // xarex1337
#define getBits( x ) (INRANGE((x&(~0x20)),'A','F') ? ((x&(~0x20)) - 'A' + 0xa) : (INRANGE(x,'0','9') ? x - '0' : 0))
#define getByte( x ) (getBits(x[0]) << 4 | getBits(x[1]))
//template<typename T> T inline EMUPOINTER(void* p) { return (T)(p ? PCSX2POINTER(p) : null); } // need?
template<typename T> T inline EMUPOINTER(void* p) { return (T)(PCSX2POINTER(p)); } // need? moved to define
template<typename T> T inline EMUPOINTER(uintptr_t p) { return EMUPOINTER<T>((void*)p); } // need?
#define VU2V(v) (*(CVector*)(&v))
#define V2VU(v) (*(CVuVector*)(&v))
#define debug(f, ...) printf(f, ##__VA_ARGS__)


//uint64_t& flags = *OFFSET(p, 0x1D8, uint64_t*);
//if (flags & (1ULL << 3)) { flags &= ~(1ULL << 3); }
//else { flags |= (1ULL << 3); }

#define STRU_PAD_PASTE(a,b) a##b
#define STRU_PAD(num, size) \
    uint8_t STRU_PAD_PASTE(_pad, num)[size];
#define STRU_PAD_LINE(size) \
    uint8_t STRU_PAD_PASTE(_pad, __LINE__)[size];
#if defined(__COUNTER__)
#define STRU_PAD_C(size) \
    uint8_t STRU_PAD_PASTE(_pad, __COUNTER__)[size];
#else
#define STRU_PAD_C(size) STRU_PAD_LINE(size)
#endif

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
inline bool BTN(int c) { return (GetAsyncKeyState(c) & 0x8000); }


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

uintptr_t inline UNJAL(uintptr_t pos, uintptr_t val) {
	// Извлекаем 26-битный адрес из инструкции JAL
	uintptr_t target = (val & 0x03FFFFFF) << 2;
	// Восстанавливаем старшие 4 бита на основе адреса следующей инструкции
	uintptr_t upper = (pos + 4) & 0xF0000000;
	return upper | target;
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

void PrintCurrDir()
{
	char buffer[MAX_PATH];  // MAX_PATH is typically 260
	DWORD length = GetCurrentDirectoryA(MAX_PATH, buffer);
	if (!length || length > MAX_PATH) { return; }
	printf("Current directory: %s\n", buffer);
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

// Empire
#define EmpireHud ((CEmpireHud*)PCSX2POINTER(*(uintptr_t**)IDATRANSLATE(0x48F370)))
#define EmpireMgr ((CEmpireMgr*)PCSX2POINTER(*(uintptr_t**)IDATRANSLATE(0x48F050)))
#define InteriorManager ((CInteriorManager*)PCSX2POINTER(*(uintptr_t**)IDATRANSLATE(0x48FF48))) // cInteriorPlacement
#define CObjectData_ms_aObjectInfo ((char*)PCSX2POINTER(*(uintptr_t**)IDATRANSLATE(0x488054)))

#define C3dMarkers_m_aMarkerArray ((C3dMarker*)IDATRANSLATE(0x4E01D0)) // 32
#define C3dMarkers_m_pRslElementGroupArray ((int32_t*)PCSX2POINTER(*(uintptr_t**)IDATRANSLATE(0x4CD160))) // 9
#define CTheScripts_ScriptSphereArray ((script_sphere_struct*)IDATRANSLATE(0x50CE10)) // 16

#define TheRadar ((CRadar*)PCSX2POINTER(*(uintptr_t**)IDATRANSLATE(0x48F548)))
#define TheDollarParticleFX ((char*)PCSX2POINTER(*(uintptr_t**)IDATRANSLATE(0x488784)))
#define FrontEndMenuManager  ((char**)IDATRANSLATE(0x729480))									// CMenuManager (page, ismenuactive)
#define FrontEndMenuManagerSettings ((char*)PCSX2POINTER(*(uintptr_t**)IDATRANSLATE(0x48F3D0))) // CMenuManagerSettings (settings val, prefs)
#define CFont_Details ((void*)IDATRANSLATE(0x711F30)) // ?

#define CPopulation_ms_pPedGroups ((char*)PCSX2POINTER(*(uintptr_t**)IDATRANSLATE(0x488014)))
#define cWorldStream ((cWorldStream*)PCSX2POINTER(*(uintptr_t**)IDATRANSLATE(0x48F4F8)))
#define CRopes_aRopes ((CRope*)IDATRANSLATE(0x505C00)) // 8

#define gpSkidTex ((char*)PCSX2POINTER(*(uintptr_t**)IDATRANSLATE(0x48A5B0)))
#define currentTexDict ((void*)PCSX2POINTER(*(uintptr_t**)IDATRANSLATE(0x4887B8)))
#define musicNameIdAssoc ((tMusicNameIdAssoc*)IDATRANSLATE(0x48A788))
#define gPhoneInfo ((char*)IDATRANSLATE(0x51F710))
#define CMuzzleFlashes_aMuzzleFlashes ((CMuzzleFlash*)IDATRANSLATE(0x004DE650)) // 8
#define ThePaths ((CPathFind*)PCSX2POINTER(*(uintptr_t**)IDATRANSLATE(0x487490)))

#define CutsceneMgr ((CCutsceneMgr*)PCSX2POINTER(*(uintptr_t**)IDATRANSLATE(0x48F820)))
#define TheAnimManager ((CAnimManager*)PCSX2POINTER(*(uintptr_t**)IDATRANSLATE(0x48F048)))

#define MusicManager ((cMusicManager*)IDATRANSLATE(0x4E41E0))
#define AudioManager ((cAudioManager*)IDATRANSLATE(0x514A90))
#define SampleManager ((cSampleManager*)IDATRANSLATE(0x4CDA08))
#define gAm_sfxgxt ((sMissionAudioManager*)IDATRANSLATE(0x4CDA40))
#define aEngineSounds ((tEngineSounds*)IDATRANSLATE(0x4A9538))
#define TOTAL_AUDIO_SAMPLES 7721

#define CClock_ms_nGameClockHours ((uint8_t*)IDATRANSLATE(0x4CD148))
//#define CTimer_ms_fTimeScale ((float*)IDATRANSLATE(0x4CD168))

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
#define CColStore_ms_pColPool ((CPool*)PCSX2POINTER(*(uintptr_t**)IDATRANSLATE(0x4818F8))) // 72

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
#define CTheScripts_ScriptsArray ((CRunningScript*)IDATRANSLATE(0x72F8D0)) // script pool
#define CTheScripts_pActiveScripts ((CRunningScript**)IDATRANSLATE(0x4CD3C4))
#define CTheScripts_pIdleScripts ((CRunningScript**)IDATRANSLATE(0x4CD3C8))
#define CTheScripts_ActiveScripts ((CRunningScript*)PCSX2POINTER(*CTheScripts_pActiveScripts))
#define CTheScripts_IdleScripts ((CRunningScript*)PCSX2POINTER(*CTheScripts_pIdleScripts))
#define CTheScripts_MainScriptSize ((*(uint32_t*)IDATRANSLATE(0x4CD3D8))) // COMMAND_LOAD_AND_LAUNCH_MISSION_INTERNAL
#define CTheScripts_LargestMissionScriptSize ((*(uint32_t*)IDATRANSLATE(0x4CD3E8))) // COMMAND_LOAD_AND_LAUNCH_MISSION_INTERNAL
#define CTheScripts_MultiScriptArray ((int32_t*)IDATRANSLATE(0x7404D0)) // missions offsets
#define CTheScripts_NextProcessId ((int32_t*)IDATRANSLATE(0x48742C))
#define CTheScripts_bAlreadyRunningAMissionScript ((bool*)IDATRANSLATE(0x4CD3E7))
#define CTheScripts_InTheScripts ((bool*)IDATRANSLATE(0x487428))
//#define CTheScripts_ScriptSphereArray ((script_sphere_struct*)IDATRANSLATE(0x50CE10)) // 16
#define ScriptParams ((int32_t*)IDATRANSLATE(0x50DEF8))
bool& gbGlassCheat = *(bool*)IDATRANSLATE(0x489EB4);
bool& CSpecialFX_bLiftCam = *(bool*)IDATRANSLATE(0x481CD8);
bool& CPad_bHasPlayerCheated = *(bool*)IDATRANSLATE(0x487A54);
float& CTimer_ms_fTimeScale = *(float*)IDATRANSLATE(0x4CD168);
uint32_t& CTimer_m_snTimeInMilliseconds = *(uint32_t*)IDATRANSLATE(0x4CD104);
int& CGame_currArea = *(int*)IDATRANSLATE(0x489F7C);
int& CGame_currLevel = *(int*)IDATRANSLATE(0x4CD118);
int& globalRenderFlags = *(int*)IDATRANSLATE(0x4882A0);
int& CRenderer_ms_nNoOfVisibleEmpires = *(int*)IDATRANSLATE(0x4CD5A0);
int8_t& CMuzzleFlashes_NumMuzzleFlashes = *(int8_t*)IDATRANSLATE(0x00481CC0);
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
#define DUMPVEC(vec) printf("%f %f %f\n", ((vec).x), ((vec).y), ((vec).z))
#define DUMPSVEC(s, vec) printf("%s %f %f %f\n", s, ((vec).x), ((vec).y), ((vec).z))
//#define DUMPPOS(vec) DUMPVEC(vec) // tmp. todo remove
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
CVector FindPlayerMenuTarget() {
	CVuVector pos = {0, 0, 0};
	for (int i = 0; i < 75; i++) { // NUMRADARBLIPS
		if (TheRadar->ms_RadarTrace[i].m_eRadarSprite == eRadarSprite::RADAR_SPRITE_MP_OBJECTIVE) { pos = TheRadar->ms_RadarTrace[i].m_vecPos; break; }
	}
	return {pos.x, pos.y, (pos.z == 0.0f && pos.x != 0.0f && pos.y != 0.0f) ? 12.0f : pos.z};
}

void DumpScriptList(CRunningScript* first_in_list, const char* desc = "") {
	CRunningScript* pScript = first_in_list;
	while (pScript)
	{
		printf("%sSC index %d, IP: %d, SC name: %s\n", desc, ((uintptr_t)pScript - ((uintptr_t)CTheScripts_ScriptsArray)) / sizeof(CRunningScript),
			pScript->m_nIp, pScript->m_abScriptName);
		pScript = EMUPOINTER<CRunningScript*>(pScript->m_pNext);
	}
}

void InitScript(CRunningScript* pScript, const char* custom_name = "", bool oldscrname = true)
{
	if (!pScript) { return; }
	int id = pScript->m_nId;
	char buff[8];
	strcpy(buff, pScript->m_abScriptName);
	memset(pScript, 0, sizeof(CRunningScript));
	if (custom_name && custom_name[0] != '\0')
	{
		strncpy(pScript->m_abScriptName, custom_name, sizeof(pScript->m_abScriptName) - 1);
		pScript->m_abScriptName[sizeof(pScript->m_abScriptName) - 1] = '\0';
	}
	else if (oldscrname && buff[0]) { sprintf(pScript->m_abScriptName, "%s", buff); }
	else { sprintf(pScript->m_abScriptName, "id%02i", id); }
	pScript->m_bDeatharrestEnabled = true;
}

void RemoveScriptFromList(CRunningScript* removable_script, CRunningScript** ppListScript)
{
	if (!removable_script || !ppListScript) { return; }
	if (removable_script->m_pPrev) { ((CRunningScript*)PCSX2POINTER(removable_script->m_pPrev))->m_pNext = removable_script->m_pNext; }
	else { *ppListScript = removable_script->m_pNext; }
	if (removable_script->m_pNext) { ((CRunningScript*)PCSX2POINTER(removable_script->m_pNext))->m_pPrev = removable_script->m_pPrev; }
}

void AddScriptToList(CRunningScript* addable_script, CRunningScript** ppListScript)
{
	if (!addable_script || !ppListScript) { return; }
	addable_script->m_pNext = *ppListScript; // ps2 ptr into
	addable_script->m_pPrev = null;
	if (*ppListScript) { ((CRunningScript*)PCSX2POINTER(*ppListScript))->m_pPrev = (CRunningScript*)PCSXTRANSLATE(addable_script); }
	*ppListScript = (CRunningScript*)PCSXTRANSLATE(addable_script);
}

void ClearActiveList()
{
	//CRunningScript* pScript = CTheScripts_ActiveScripts;
	//while (pScript)
	//{
	//	RemoveScriptFromList(pScript, CTheScripts_pActiveScripts);
	//	InitScript(pScript);
	//	AddScriptToList(pScript, CTheScripts_pIdleScripts);
	//	pScript = EMUPOINTER<CRunningScript*>(pScript->m_pNext);
	//}
	//*CTheScripts_pActiveScripts = null;
}

void ResetScripts()
{
	*CTheScripts_bAlreadyRunningAMissionScript = false;
	*CTheScripts_NextProcessId = 0;
	*CTheScripts_pIdleScripts = null;
	*CTheScripts_pActiveScripts = null;
	for (int32_t i = 0; i < 128; i++)
	{
		CTheScripts_ScriptsArray[i].m_nId = (*CTheScripts_NextProcessId)++;
		InitScript(&CTheScripts_ScriptsArray[i], "", false);
		CTheScripts_ScriptsArray[i].m_nIp = 0;
		AddScriptToList(&CTheScripts_ScriptsArray[i], CTheScripts_pIdleScripts);
	}
}

CRunningScript* StartNewScript(uint32_t ip = 0, bool mission = false, const char* custom_name = "")
{
	CRunningScript* pNew = CTheScripts_IdleScripts;
	if (!pNew) { return null; }
	{
		RemoveScriptFromList(pNew, CTheScripts_pIdleScripts);
		pNew->m_nId = (*CTheScripts_NextProcessId)++;
		InitScript(pNew, custom_name);
		pNew->m_nIp = ip;
		AddScriptToList(pNew, CTheScripts_pActiveScripts);
		pNew->m_bIsActive = true;
	}
	if (mission)
	{
		pNew->m_bIsMissionScript = true;
		pNew->m_bMissionFlag = true;
		*CTheScripts_bAlreadyRunningAMissionScript = true;
	}
	return pNew;
}

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

float halfFloatToFloat(uint16_t half) // Precision
{
	uint32_t sign = half & 0x8000;
	int32_t exp = (half >> 10) & 0x1F;
	uint32_t mant = half & 0x3FF;
	uint32_t f = sign << 16 | (exp + 127 - 15) << 23 | mant << 13;
	return *(float*)&f;
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
std::vector<std::string> coms;

class ModelInfoExt
{
public:
	bool inited;
	int index;
	uint32_t hash;
	std::string name; // br
	//---EXT
	std::vector<CColModel*> allcolls; // ld steps

	static std::vector<ModelInfoExt> modelInfoExt;
	static void Init(const char* path)
	{
		std::vector<std::string> rows = FileReadAllLines(path);
		int max_index = -1;
		for (const auto& row : rows)
		{
			if (row.empty()) continue;
			int index;
			std::istringstream iss(row);
			char comma;
			if (iss >> index >> comma)
				if (index > max_index)
					max_index = index;
		}
		if (max_index >= 0)
			modelInfoExt.resize(max_index + 1);
		for (const auto& row : rows)
		{
			if (row.empty()) continue;
			int index;
			uint32_t hash;
			std::string name;
			std::istringstream iss(row);
			char comma;
			if (iss >> index >> comma >> std::hex >> hash >> comma >> name)
			{
				if (index >= 0 && index < modelInfoExt.size())
				{
					modelInfoExt[index].inited = true;
					modelInfoExt[index].index = index;
					modelInfoExt[index].hash = hash;
					modelInfoExt[index].name = name;
				}
				else
					throw std::out_of_range("Invalid index in ModelInfoExt data");
			}
		}

		printf("ModelInfoExt: total %zu\n", modelInfoExt.size());
	}
};
std::vector<ModelInfoExt> ModelInfoExt::modelInfoExt;
ModelInfoExt* GetModelInfoExt(int mi) { return &ModelInfoExt::modelInfoExt[mi]; }


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

		//MemoryPatcher(IDATRANSLATE(0x26BD34), 0, 4), // nop test menu draw empires
		//MemoryPatcher(IDATRANSLATE(0x35D208), 0, 8), // empire bool
		//MemoryPatcher(IDATRANSLATE(0x0035D16C), 0, 4), // empire ren1 (primary)
		//MemoryPatcher(IDATRANSLATE(0x0035D21C), 0, 4), // empire ren2  (bool1) // окна
		//MemoryPatcher(IDATRANSLATE(0x0035D208), 0, 4*2), // empire ren bool not
		//MemoryPatcher(IDATRANSLATE(0x0035D0BC), 0, 4*3), // empire geo2mat cb zero nop  // не заметил

		//MemoryPatcher(IDATRANSLATE(0x003F3C3C), 0, 0x003F3D08 - 0x003F3C3C), // fonts reset
		//MemoryPatcher(IDATRANSLATE(0x003F4478), 0, 0x003F448C - 0x003F4478), // fonts setcol3

		//MemoryPatcher(IDATRANSLATE(0x00343E68), 0, 4), // bike siren prerender test
		//MemoryPatcher(IDATRANSLATE(0x00343E0C), 0, 4), // bike siren prerender test
		//MemoryPatcher(IDATRANSLATE(0x0033FEF8), 0, 4), // bike siren prerender test

		//MemoryPatcher(IDATRANSLATE(0x0021F404), 0, 4), // cutscene draw stuff test
		//MemoryPatcher(IDATRANSLATE(0x1B6004), 0, 4), // jetski audio test nop
		//MemoryPatcher(IDATRANSLATE(0x1B6288), 0, 4), // forklift audio test nop
		//MemoryPatcher(IDATRANSLATE(0x1DD4EC), 0, 4), // bike anim test nop
		//MemoryPatcher(IDATRANSLATE(0x002F7540), 0, 4), // vu0 nop
		//MemoryPatcher(IDATRANSLATE(0x002F7448), 0, 4), // vu0 nop

		MemoryPatcher(IDATRANSLATE(0x003BAB44), 0, 4), // nop peds gen
		MemoryPatcher(IDATRANSLATE(0x003BAC88), 0, 4), // nop cars gen
		MemoryPatcher(IDATRANSLATE(0x3BAAC0), 0, 4), // nop CTheScripts::Process() //--------------------------------------------------------------
		//MemoryPatcher(IDATRANSLATE(0x21E9D0), 0, 4), // nop CGame::Process() //--------------------------------------------------------------
		//MemoryPatcher(IDATRANSLATE(0x0021EE38), 0, 4), // nop Idle() //--------------------------------------------------------------
		//MemoryPatcher(IDATRANSLATE(0x0021E978), 0, 0x0021ED5C - 0x0021E978), // nop Idle() //--------------------------------------------------------------
		//MemoryPatcher(IDATRANSLATE(0x2D8840), 0, 4), // nop driveby
		//MemoryPatcher(IDATRANSLATE(0x002E5188), 0, 0x002E51F8 - 0x002E5188), // nop empire flags // (all) падает
		//MemoryPatcher(IDATRANSLATE(0x002E51EC), 0, 0x002E51F8 - 0x002E51EC), // nop empire flags // mi, не падает, остались окна
		//MemoryPatcher(IDATRANSLATE(0x002E51B0), 0, 0x002E51C0 - 0x002E51B0), // nop empire flags // (flags2) падает, при ударе бред
		//MemoryPatcher(IDATRANSLATE(0x002E5188), 0, 0x002E51B0 - 0x002E5188), // nop empire flags // (flags1) не падает, бред с колизией
		//MemoryPatcher(IDATRANSLATE(0x0026BD34), 0, 4), // nop map render stuff empire
		//MemoryPatcher(IDATRANSLATE(0x332458), 0, 4), // nop interior init

		//MemoryPatcher(IDATRANSLATE(0x21EBF4), 0, 4), // nop effects
		//MemoryPatcher(IDATRANSLATE(0x21EC24), 0, 4), // nop 2dstuff
		//MemoryPatcher(IDATRANSLATE(0x21EBCC), 0, 4), // nop renderscene
		//MemoryPatcher(IDATRANSLATE(0x1093DC), 0, 4), // nop rebderscene cWS1
		//MemoryPatcher(IDATRANSLATE(0x1093F8), 0, 4), // nop rebderscene cWS2
		//MemoryPatcher(IDATRANSLATE(0x23E7AC), 0, 0x0023E804 - 0x0023E7AC), // nop barbershop
		//MemoryPatcher(IDATRANSLATE(0x0021F11C), 0,4), // nop barbershop

		//MemoryPatcher(IDATRANSLATE(0x0021F158), 0, 4), // render empire build 3d
		//MemoryPatcher(IDATRANSLATE(0x0035D16C), 0, 4), // render empire build PASS1 (сам empire)
		//MemoryPatcher(IDATRANSLATE(0x0035D21C), 0, 4), // render empire build PASS2 (broken geo)
		//MemoryPatcher(IDATRANSLATE(0x0035D2B8), 0, 4), // render empire build PASS3 (неон, светящееся окно)
		//MemoryPatcher(IDATRANSLATE(0x440E5C), 0, 4), // nop upd AUDIO
		//MemoryPatcher(IDATRANSLATE(0x18A08C), 0, 4), // nop upd ambience 1
		//MemoryPatcher(IDATRANSLATE(0x18A1E8), 0, 4), // nop upd ambience 2

		//MemoryPatcher(IDATRANSLATE(0x003F5018), 0, 0x003F5058 - 0x003F5018), // 
		//MemoryPatcher(IDATRANSLATE(0x31C6CC), 0, 4), // hud lift cam
		//MemoryPatcher(IDATRANSLATE(0x11AD44), 0, 4), // water nose
		//MemoryPatcher(IDATRANSLATE(0x0018EB88), 0, 0x0018EC34 - 0x0018EB88), // anm
		//MemoryPatcher(IDATRANSLATE(0x00280894), 0, 0x00280934 - 0x00280894), // anm jaw
		//MemoryPatcher(IDATRANSLATE(0x0028084C), 0xFF, 1), //

		//MemoryPatcher(IDATRANSLATE(0x003C5120), 0, 4), // upd anim
		//MemoryPatcher(IDATRANSLATE(0x003C51A0), 0, 4), // upd anim
		//MemoryPatcher(IDATRANSLATE(0x00154DC4), 0, 4), // anim node set
		//MemoryPatcher(IDATRANSLATE(0x00215988), 0, 4), // anim add cped
		//MemoryPatcher(IDATRANSLATE(0x00215908), 0, 4), // anim blend cped
		//MemoryPatcher(IDATRANSLATE(0x0018E8F0), 0, 4), // nop find jaw
		//MemoryPatcher(IDATRANSLATE(0x003BAC04), 0, 4), // nop rope upd

		//MemoryPatcher(IDATRANSLATE(0x00109420), 0, 4), // nop cWS Render pass 0
		//MemoryPatcher(IDATRANSLATE(0x00109460), 0, 4), // nop cWS Render pass 1
		//MemoryPatcher(IDATRANSLATE(0x0021F11C), 0, 4), // nop cWS Render pass 2

		//MemoryPatcher(IDATRANSLATE(0x0039F098), 0, 4), // nop render 3.0
		//MemoryPatcher(IDATRANSLATE(0x002260E8), 0, 0x00226140 - 0x002260E8), // nop rope -z
		//MemoryPatcher(IDATRANSLATE(0x0013A224), 0, 4), // nop rope cutomobile process controll shit
	};
}
void inline SetPatchesState(bool state) { for (size_t i = 0; i < patches.size(); i++) { state ? patches[i].ApplyPatch() : patches[i].RemovePatch(); } }



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
void TeleporterTester()
{
	//return;
	bool prev = (GetAsyncKeyState(VK_OEM_COMMA) & 0x8000);
	bool next = (GetAsyncKeyState(VK_OEM_PERIOD) & 0x8000);
	bool allow = (GetAsyncKeyState(VK_CONTROL) & 0x8000);
	bool tp = false;
	if (prev && (!gbTeleportHold || allow))
	{
		--giTeleportIndex;
		gbTeleportHold = tp = true;
	}
	else if (next && (!gbTeleportHold || allow))
	{
		++giTeleportIndex;
		gbTeleportHold = tp = true;
	}
	else if (!prev && !next) { gbTeleportHold = false; }

	//// cargens
	//if (giTeleportIndex < 0) { giTeleportIndex = CTheCarGenerators_NumOfCarGenerators - 1; }
	//if (giTeleportIndex >= CTheCarGenerators_NumOfCarGenerators) { giTeleportIndex = 0; }
	//if (tp)
	//{
	//	CCarGenerator* gen = &CTheCarGenerators_CarGeneratorArray[giTeleportIndex];
	//	CVector pos = gen->m_vecPos;
	//	if (gen->m_nModelIndex != 213) // maverik
	//		return;
	//	printf("[TP id_%d, mi_%d]: %f %f %f\n", giTeleportIndex, gen->m_nModelIndex, pos.z, pos.y, pos.z);
	//	pos.x -= 2.0f;
	//	pos.y -= 2.0f;
	//	pos.z -= 0.5f;
	//	TeleportPlayer(pos);
	//}

	// cWS
	//if (tp)
	//{
	//	sLevelChunk* pWorldData = EMUPOINTER<sLevelChunk*>(cWorldStream->m_pWorldDataLevelChunk);
	//	CGroupedBuilding* m_pSwapBuildingGroups = EMUPOINTER<CGroupedBuilding*>(pWorldData ? pWorldData->swapInfos : null);

	//	if (giTeleportIndex < 0) {
	//		giTeleportIndex = pWorldData ? pWorldData->numSwapInfos - 1 : 0;
	//	}
	//	if (giTeleportIndex >= (pWorldData ? pWorldData->numSwapInfos : 0)) { giTeleportIndex = 0; }

	// swaps
	//	CVector pos = { 0, 0, 0 };
	//	if (m_pSwapBuildingGroups)
	//	{
	//		CEntity* pB = EMUPOINTER<CEntity*>(m_pSwapBuildingGroups[giTeleportIndex].m_pActualBuilding);
	//		if (pB) {
	//			pos = *(CVector*)&pB->CPlaceable.m_pMat.pos; 
	//			printf("%d [TP id_%d, mi_%d]: %f %f %f\n", pWorldData->numSwapInfos, giTeleportIndex,
	//				//CTheCarGenerators_CarGeneratorArray[giTeleportIndex].m_nModelIndex,
	//				0,
	//				pos.z, pos.y, pos.z);
	//			pos.x -= 2.0f;
	//			pos.y -= 2.0f;
	//			pos.z -= 0.5f;
	//			TeleportPlayer(pos);
	//		}
	//	}
	//}

	// PathNode
	//if (tp)
	//{
	//	int max = ThePaths->m_numPathNodes;
	//	if (giTeleportIndex < 0) { giTeleportIndex = max - 1; }
	//	if (giTeleportIndex >= max) { giTeleportIndex = 0; }
	//	int index = giTeleportIndex;
	//	CVector pos = { 0.0f, 0.0f, 15.0f };
	//	CPathNode* n = &EMUPOINTER<CPathNode*>(ThePaths->m_pathNodes)[index];
	//	pos.x = (n->posX / 8); // or (pos*2) / 16
	//	pos.y = (n->posY / 8); // or (pos*2) / 16
	//	printf("[%d] %f %f %f\n", index, pos.x, pos.y, pos.z);
	//	printf("orig %d %d\n\n", n->posX, n->posY);

	//	TeleportPlayer(pos);
	//}

	// ScriptSphere and 3dMarker
	if (tp)
	{
		int max = 16; // script sphere
		if (giTeleportIndex < 0) { giTeleportIndex = max - 1; }
		if (giTeleportIndex >= max) { giTeleportIndex = 0; }
		int index = giTeleportIndex;
		CVector pos = { 0.0f, 0.0f, 15.0f };

#if 1
		// script sphere
		pos = VU2V(CTheScripts_ScriptSphereArray[index].m_vecCenter);
		printf("index: %d, 0x%p type: %d, inuse: %d, pos: %f %f\n", index, &CTheScripts_ScriptSphereArray[index],
		CTheScripts_ScriptSphereArray[index].m_Type, CTheScripts_ScriptSphereArray[index].m_bInUse, pos.x, pos.y);
#else
		max = 32; // 3d marker
		pos = VU2V(C3dMarkers_m_aMarkerArray[index].m_Matrix.pos);
		printf("index: %d, 0x%p  %f %f %f\n", index, &C3dMarkers_m_aMarkerArray[index], pos.x, pos.y, pos.z);
#endif

		if (pos.x == 0.0f && pos.y == 0.0f)
			return;

		TeleportPlayer(pos);
	}
}

void PatchCustomSCM()
{
	//for (int i = 0; i < 50; i++) { printf("%X ", CTheScripts_ScriptSpace[i]); }
	std::vector<uint8_t> buffer = FileReadAllBytes("Plugins\\_VCS_\\freeroam_vcs.scm");
	//std::vector<uint8_t> buffer = FileReadAllBytes("Plugins\\_VCS_\\main.scm");
	//std::vector<uint8_t> buffer = FileReadAllBytes("Plugins\\_VCS_\\1.scm"); // no size
	MemoryPatcher patcher = MemoryPatcher(IDATRANSLATE(0x3BAAC0), 0, 4); // CTheScripts::Process()
	//MemoryPatcher patcher = MemoryPatcher(IDATRANSLATE(0x21E9D0), 0, 4); // CGame::Process()
	if (!buffer.size()) { return; }
	printf("Read scm: %d\n", buffer.size());
	uint32_t MainScriptSize = *(uint32_t*)buffer.data();
	int nLargestMissionSize = *(((uint32_t*)buffer.data()) + 1);
	//printf("main: %d, mission %d\n", MainScriptSize, nLargestMissionSize);
	buffer.erase(buffer.begin(), buffer.begin() + (4 + 4));
	patcher.ApplyPatch();
	RESET_RECOMP_EE();
	do { Sleep(2 * 1000); } while (*CTheScripts_InTheScripts);
	//Sleep(2 * 1000);
	//ClearActiveList();
	ResetScripts();
	//DumpScriptList(CTheScripts_ActiveScripts, "(activelist) ");
	//DumpScriptList(CTheScripts_IdleScripts, "(idlelist) ");
	//return;
	//*CTheScripts_bAlreadyRunningAMissionScript = false;
	//*CTheScripts_pIdleScripts = null;
	//*CTheScripts_pActiveScripts = null;
	//*CTheScripts_NextProcessId = 0;
	//{
	//	DWORD vp[2];
	//	void* p = (void*)CTheScripts_ScriptSpace;
	//	int32_t size = CTheScripts_MainScriptSize + CTheScripts_LargestMissionScriptSize;
	//	VirtualProtect(p, size, PAGE_EXECUTE_READWRITE, &vp[0]); // !!
	//	memset(p, 0, size);
	//	memcpy(p, buffer.data(), buffer.size());
	//	VirtualProtect(p, size, vp[0], &vp[1]);
	//}
	//for (int32_t i = 0; i < 128; i++)
	//{
	//	memset(&CTheScripts_ScriptsArray[i], 0, sizeof(CRunningScript));
	//	CTheScripts_ScriptsArray[i].m_bDeatharrestEnabled = true;
	//	sprintf(CTheScripts_ScriptsArray[i].m_abScriptName, "id00");
	//	AddScriptToList(&CTheScripts_ScriptsArray[i], CTheScripts_pIdleScripts);
	//}
	memcpy(&CTheScripts_ScriptSpace[0], buffer.data(), CTheScripts_MainScriptSize);
	CRunningScript* pNew = StartNewScript(0, false, "cleo");
	//memcpy(&CTheScripts_ScriptSpace[CTheScripts_MainScriptSize], buffer.data(), buffer.size());
	//CRunningScript* pNew = StartNewScript(CTheScripts_MainScriptSize, false, "mzhk1");
	//printf("pNew: 0x%p\n", pNew);
	//DumpScriptList(CTheScripts_ActiveScripts, "(activelist) ");
	//DumpScriptList(CTheScripts_IdleScripts, "(idlelist) ");
	patcher.RemovePatch();
	RESET_RECOMP_EE();
	//Sleep(2 * 1000);
	printf("ok\n");
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
	DUMPVEC(pedpos);
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

	//FileWriteAllBytes("C:\\1.scm", buff, sizeof(buff));
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


std::vector<RwTexture*> GetRwTexturesFromTxd(RwTexDictionary* pRwTexDictionary)
{
	std::vector<RwTexture*> textures;
	if (pRwTexDictionary)
	{
		RwLinkList* pRLL = EMUPOINTER<RwLinkList*>(pRwTexDictionary->textures__texturesInDict.link.next);
		RwTexDictionary* pEndDic;

		do
		{
			pEndDic = EMUPOINTER<RwTexDictionary*>(pRLL->link.next);
			RwTexture* pTex = (RwTexture*)&pRLL[-1];

			textures.push_back(pTex);

			pRLL = (RwLinkList*)pEndDic;
		} while (pEndDic != (RwTexDictionary*)&pRwTexDictionary->textures__texturesInDict);
	}

	return textures;
}



void EmpireTest(int mode)
{
	if (mode == 1) { return; } // dll loop
	//system("cls");
	CEmpireHud* pEmpireHudInstance = EmpireHud;
	printf("EmpireHud: 0x%p\n", pEmpireHudInstance);
	if (!pEmpireHudInstance) { return; }
	//printf("start: 0x%p  end: 0x%p\n", pEmpireHudInstance->pHudNodeArrayList, pEmpireHudInstance->pHudNodeArrayListEnd);
	tHudElement** current = EMUPOINTER<tHudElement**>(pEmpireHudInstance->hudElementList); // fixed array pointer
	tHudElement** end = EMUPOINTER<tHudElement**>(pEmpireHudInstance->hudElementListEnd);
	if (!current || !*current || !end || !*end) { return; }
	int i = 0;
	int maxi = 0, maxj = 0;
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

				//----test wrap
				{
					//infoCurrent->field_8 = 0; // не выходит из меню
				}

				// Move to the next element in pHudElementInfoList
				++infoCurrent;
				++j;
				if (j > maxj) maxj = j;
			}
		}
		//else printf("!node\n");
		++current;
		++i;
		if (i > maxi) maxi = i;
	}
	printf("[INFO]: maxi: %d, maxj: %d\n", maxi, maxj);
	printf("\n\n");
}

CPed* GetRandomPedFromPool(bool canplayer = false)
{
	int32_t sz = CPools_ms_pPedPool->m_nSize;
	CPed* playa = (CPed*)FindPlayerPed();
	if (sz <= 0) {
		printf("Ped pool is empty!\n");
		return nil;
	}
	//if (!playa && !canplayer) {
	//	printf("Player ped not found but canplayer=false!\n");
	//	return nil;
	//}

	int attempts = 0;
	const int MAX_ATTEMPTS = 50;

	while (true) {
		attempts++;

		// Генерируем случайный стартовый индекс от 1 до размера пула
		int32_t startIdx = (rand() % sz) + 1;

		// Проходим по пулу, начиная со случайного индекса
		for (int32_t i = startIdx; i < sz; i++) {
			if (CPools_GetSlotIsFree(CPools_ms_pPedPool, i)) { continue; }
			CPed* e = (CPed*)CPools_GetSlot(CPools_ms_pPedPool, i, 3360);
			if (e && (canplayer || playa != e)) {
				return e;
			}
		}

		// Если не нашли с текущего индекса, проверяем начало
		for (int32_t i = 0; i < startIdx; i++) {
			if (CPools_GetSlotIsFree(CPools_ms_pPedPool, i)) { continue; }
			CPed* e = (CPed*)CPools_GetSlot(CPools_ms_pPedPool, i, 3360);
			if (e && (canplayer || playa != e)) {
				return e;
			}
		}

		// Если достигли максимального количества попыток
		if (attempts >= MAX_ATTEMPTS) {
			printf("Failed to find random ped after %d attempts!\n", attempts);
			return nil;
		}
	}
	return nil;
}

CVehicle* GetRandomVehicleFromPool(bool canplayer = false)
{
	int32_t sz = CPools_ms_pVehiclePool->m_nSize;
	CVehicle* pV = FindPlayerVehicle();

	if (sz <= 0) {
		printf("Vehicle pool is empty!\n");
		return nil;
	}
	//if (!pV && !canplayer) {
	//	printf("Player ped not found but canplayer=false!\n");
	//	return nil;
	//}

	int attempts = 0;
	const int MAX_ATTEMPTS = 50;

	while (true) {
		attempts++;

		// Генерируем случайный стартовый индекс от 0 до размера пула-1
		int32_t startIdx = rand() % sz;

		// Проходим по пулу, начиная со случайного индекса
		for (int32_t i = startIdx; i < sz; i++) {
			if (CPools_GetSlotIsFree(CPools_ms_pVehiclePool, i)) { continue; }
			CVehicle* e = (CVehicle*)CPools_GetSlot(CPools_ms_pVehiclePool, i, 2240);
			if (e && (canplayer || pV != e)) {
				return e;
			}
		}

		// Если не нашли с текущего индекса, проверяем начало
		for (int32_t i = 0; i < startIdx; i++) {
			if (CPools_GetSlotIsFree(CPools_ms_pVehiclePool, i)) { continue; }
			CVehicle* e = (CVehicle*)CPools_GetSlot(CPools_ms_pVehiclePool, i, 2240);
			if (e && (canplayer || pV != e)) {
				return e;
			}
		}

		// Если достигли максимального количества попыток
		if (attempts >= MAX_ATTEMPTS) {
			printf("Failed to find random vehicle after %d attempts!\n", attempts);
			return nil;
		}
	}

	return nil;
}

void TestingPools()
{
	struct tId { int id, poolid; };
	std::vector<tId> ids;

	int pedpoolfree = 0;
	for (int32_t i = CPools_ms_pPedPool->m_nSize - 1; i >= 0; i--)
	{
		if (CPools_GetSlotIsFree(CPools_ms_pPedPool, i)) { ++pedpoolfree; continue; }
		CPed* e = (CPed*)CPools_GetSlot(CPools_ms_pPedPool, i, 3360);
		if (e)
		{
			int mi1 = e->CPhysical.CEntity.m_modelIndex;
			int mi2 = e->CPhysical.CEntity.m_modelIndex2;
			ids.push_back({ mi1, 0 });
			if (mi1 != mi2) { MboxSTD("!mi"); }
		}
	}
	int vehpoolfree = 0;
	for (int32_t i = CPools_ms_pVehiclePool->m_nSize - 1; i >= 0; i--)
	{
		if (CPools_GetSlotIsFree(CPools_ms_pVehiclePool, i)) { ++vehpoolfree; continue; }
		CVehicle* e = (CVehicle*)CPools_GetSlot(CPools_ms_pVehiclePool, i, 2240);
		if (e)
		{
			int mi1 = e->CPhysical.CEntity.m_modelIndex;
			ids.push_back({ mi1, 1 });
			int mi2 = e->CPhysical.CEntity.m_modelIndex2;
			if (mi1 != mi2) { MboxSTD("!mi"); }
		}
	}
	int bldpoolfree = 0;
	for (int32_t i = CPools_ms_pBuildingPool->m_nSize - 1; i >= 0; i--)
	{
		if (CPools_GetSlotIsFree(CPools_ms_pBuildingPool, i)) { ++bldpoolfree; continue; }
		CEntity* e = (CEntity*)CPools_GetSlot(CPools_ms_pBuildingPool, i, 96);
		if (e)
		{
			int mi1 = e->m_modelIndex;
			ids.push_back({ mi1, 2 });
			int mi2 = e->m_modelIndex2;
			if (mi1 != mi2) { MboxSTD("!mi"); }
		}
	}
	int trpoolfree = 0;
	for (int32_t i = CPools_ms_pTreadablePool->m_nSize - 1; i >= 0; i--)
	{
		if (CPools_GetSlotIsFree(CPools_ms_pTreadablePool, i)) { ++trpoolfree; continue; }
		CEntity* e = (CEntity*)CPools_GetSlot(CPools_ms_pTreadablePool, i, 96);
		if (e)
		{
			int mi1 = e->m_modelIndex;
			ids.push_back({ mi1, 3 });
			int mi2 = e->m_modelIndex2;
			if (mi1 != mi2) { MboxSTD("!mi"); }
		}
	}
	int objpoolfree = 0;
	for (int32_t i = CPools_ms_pObjectPool->m_nSize - 1; i >= 0; i--)
	{
		if (CPools_GetSlotIsFree(CPools_ms_pObjectPool, i)) { ++objpoolfree; continue; }
		CEntity* e = (CEntity*)CPools_GetSlot(CPools_ms_pObjectPool, i, 544);
		if (e)
		{
			int mi1 = e->m_modelIndex;
			ids.push_back({ mi1, 4 });
			int mi2 = e->m_modelIndex2;
			if (mi1 != mi2) { MboxSTD("!mi"); }
		}
	}
	int emppoolfree = 0;
	for (int32_t i = CPools_ms_pEmpirePool->m_nSize - 1; i >= 0; i--)
	{
		if (CPools_GetSlotIsFree(CPools_ms_pEmpirePool, i)) { ++emppoolfree; continue; }
		CEntity* e = (CEntity*)CPools_GetSlot(CPools_ms_pEmpirePool, i, 352);
		if (e)
		{
			int mi1 = e->m_modelIndex;
			ids.push_back({ mi1, 5 });
			int mi2 = e->m_modelIndex2;
			if (mi1 != mi2) { MboxSTD("!mi"); }
		}
	}
	int dumpoolfree = 0;
	for (int32_t i = CPools_ms_pDummyPool->m_nSize - 1; i >= 0; i--)
	{
		if (CPools_GetSlotIsFree(CPools_ms_pDummyPool, i)) { ++dumpoolfree; continue; }
		CEntity* e = (CEntity*)CPools_GetSlot(CPools_ms_pDummyPool, i, 96);
		if (e)
		{
			int mi1 = e->m_modelIndex;
			ids.push_back({ mi1, 6 });
			int mi2 = e->m_modelIndex2;
			if (mi1 != mi2) { MboxSTD("!mi"); }
		}
	}
	int texpoolfree = 0;
	for (int32_t i = CTexListStore_ms_pTexListPool->m_nSize - 1; i >= 0; i--)
	{
		if (CPools_GetSlotIsFree(CTexListStore_ms_pTexListPool, i)) { ++texpoolfree; continue; }
		TxdDef* e = (TxdDef*)CPools_GetSlot(CTexListStore_ms_pTexListPool, i, 28);
		if (e && e->name)
		{
			//printf("%s\n", e->name);
		}
	}
	int colpoolfree = 0;
	for (int32_t i = CColStore_ms_pColPool->m_nSize - 1; i >= 0; i--)
	{
		if (CPools_GetSlotIsFree(CColStore_ms_pColPool, i)) { ++colpoolfree; continue; }
		ColDef* c = (ColDef*)CPools_GetSlot(CColStore_ms_pColPool, i, 72);
		if (c)
		{
			printf("col%d 0x%p  %s\n", i, c, c->name);
		}
	}
	CW_B();
	int pedpoolsize = CPools_ms_pPedPool->m_nSize;
	int vehpoolsize = CPools_ms_pVehiclePool->m_nSize;
	int bldpoolsize = CPools_ms_pBuildingPool->m_nSize;
	int trpoolsize = CPools_ms_pTreadablePool->m_nSize;
	int objpoolsize = CPools_ms_pObjectPool->m_nSize;
	int emppoolsize = CPools_ms_pEmpirePool->m_nSize;
	int dumpoolsize = CPools_ms_pDummyPool->m_nSize;
	int texpoolsize = CTexListStore_ms_pTexListPool->m_nSize;
	int colpoolsize = CColStore_ms_pColPool->m_nSize;
	printf("Ped: %d/%d\n", pedpoolsize - pedpoolfree, pedpoolsize);
	printf("Vehicle: %d/%d\n", vehpoolsize - vehpoolfree, vehpoolsize);
	printf("Building: %d/%d\n", bldpoolsize - bldpoolfree, bldpoolsize);
	printf("Treadable: %d/%d\n", trpoolsize - trpoolfree, trpoolsize);
	printf("Object: %d/%d\n", objpoolsize - objpoolfree, objpoolsize);
	printf("Empire: %d/%d\n", emppoolsize - emppoolfree, emppoolsize);
	printf("Dummy: %d/%d\n", dumpoolsize - dumpoolfree, dumpoolsize);
	printf("Txd: %d/%d\n", texpoolsize - texpoolfree, texpoolsize);
	printf("Col: %d/%d\n", colpoolsize - colpoolfree, colpoolsize);
	CW_G();

	//std::vector<int> modelIndexes = {
	//7700, 7701, 7702, 7703, 7704, 7705, 7706, 7707, 7708, 7709,
	//7710, 7711, 7712, 7713, 7714, 7715, 7716, 7717, 7718, 7719,
	//7720, 7721, 7722, 7723, 7724, 7725, 7726, 7727, 7728, 7729,
	//7730, 7731, 7732, 7733, 7734, 7735, 7736, 7737, 7738, 7739,
	//7740, 7741, 7742, 7743, 7744, 7745, 7746, 7747
	//};
	//for (int32_t i = 0; i < modelIndexes.size(); i++)
	//{
	//	for (int32_t j = 0; j < ids.size(); j++)
	//	{
	//		if (modelIndexes[i] == ids[j].id) { MboxSTD("!!!! mi" + std::to_string(ids[j].id) + " pool:" + std::to_string(ids[j].poolid) ); }
	//	}
	//}
}

Resource* GetResourseFromChunk(sLevelChunk* pChunk, int index) // pChunk normalized
{
	if (!pChunk) { return null; }
	Resource* res = &EMUPOINTER<Resource*>(pChunk->resourceTable)[index]; // 1716 dff
	return res;
}



//inline int getNumLinks(CPathNode n) { return  n.numLinks_AndFlags8 & 0x0F; }
//inline bool isDeadEnd(CPathNode n) { return (n.numLinks_AndFlags8 & 0x10) != 0; }
//inline bool isDisabled(CPathNode n) { return (n.numLinks_AndFlags8 & 0x20) != 0; }
//inline bool isBetweenLvls(CPathNode n) { return (n.numLinks_AndFlags8 & 0x40) != 0; }
//inline bool useInRoadBlock(CPathNode n) { return (n.numLinks_AndFlags8 & 0x80) != 0; }
//inline bool bWaterPath(CPathNode n) { return (n.flags9 & 0x01) != 0; }
//inline bool onlySmallBoats(CPathNode n) { return (n.flags9 & 0x02) != 0; }
//inline bool selected(CPathNode n) { return (n.flags9 & 0x04) != 0; }
//inline int speedLimit(CPathNode n) { return (n.flags9 >> 3) & 0x03; }
//inline int spawnRate(CPathNode n) { return (n.flags9 >> 5) & 0x07; }
//
//// in VC path files, coords = world*16.  Our posX/Y are in 1/8 units,
//// so world units = pos*0.125, times 16 pos*2.0.
//inline float nodeX(CPathNode n) { return n.posX * 2.0f; } // (x / 8) * 16
//inline float nodeY(CPathNode n) { return n.posY * 2.0f; }
//inline float nodeZ(CPathNode n) {
//	// note: VCS stores Z coarsely, you may need to adjust if its
//	// actually signed and offset.  Here we just do same *2.0f.
//	return (float)n.posZ/* * 2.0f*/; // auto find z or model z offset
//}
//// This prints the end delimiter after each group:
//static void PrintEmptyLine(FILE* f) {
//	fprintf(f, "\t0, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1\n");
//}
//void DumpPathsToFile(CPathFind* paths, const char* filename)
//{
//#define GROUP_SIZE 12
//
//	setlocale(LC_NUMERIC, "C");
//	FILE* f = fopen(filename, "w");
//	int total = paths->m_numPathNodes;
//	int carMax = paths->m_numCarPathNodes;
//	// Пешеходные узлы идут сразу после carMax, до total – их здесь:
//	int pedStart = carMax;
//	int pedMax = total;
//
//	for (int base = 0; base < total; base += GROUP_SIZE)
//	{
//		//int groupType = (base < carMax) ? 1 : 0;
//		CPathNode firstNode = EMUPOINTER<CPathNode*>(ThePaths->m_pathNodes)[base];
//		int groupType;
//		if (bWaterPath(firstNode))
//			groupType = 2;  // водная группа
//		else if (base < carMax)
//			groupType = 1;  // автомобильная группа
//		else
//			groupType = 0;  // пешеходная группа
//
//		fprintf(f, "%d, -1\n", groupType);
//
//		int limit = min(GROUP_SIZE, total - base);
//		for (int j = 0; j < limit; j++)
//		{
//			CPathNode n = EMUPOINTER<CPathNode*>(ThePaths->m_pathNodes)[base + j];
//
//			int numL = getNumLinks(n);
//
//			// determine NodeType: 1 = external (link outside this group),
//			//                      2 = internal (link inside group),
//			//                      0 = null (no link)
//			int   nodeType = 0;
//			int   nextNode = -1;
//			bool  isCross = false;
//
//			for (int k = 0; k < numL; k++)
//			{
//				// absolute link index in connections array:
//				int connIdx = n.firstLink + k;
//				int16_t nodeID = EMUPOINTER<short*>(ThePaths->m_carPathConnections)[connIdx];
//
//				// group-relative index if in same group:
//				int rel = nodeID - base;
//				if (rel >= 0 && rel < limit) {
//					nodeType = 2;
//					nextNode = rel;
//				}
//				else {
//					nodeType = 1;
//					nextNode = -1;    // or nodeID if you want absolute
//				}
//				// cross-road flag stored in bit7 of that connection (v7>>15):
//				// here we just check if this link has the high bit set
//				{
//					int raw = EMUPOINTER<int*>(ThePaths->m_carPathLinks)[connIdx];
//					if (raw & 0x8000) isCross = true;
//				}
//
//				//// for simplicity, dump only first link per node
//				break;
//			}
//
//			// write: NodeType, NextNode, IsCrossRoad,
//			//        X, Y, Z,
//			//        Median (width), LeftLanes, RightLanes,
//			//        SpeedLimit, Flags, SpawnRate
//			fprintf(f,
//				"\t%d, %d, %d, %.1f, %.1f, %.1f, %.1f, %d, %d, %d, %d, %.2f\n",
//				nodeType, nextNode, isCross,
//				nodeX(n), nodeY(n), nodeZ(n),
//				n.width * /* for ped: WIDTH_TO_PED_NODE_WIDTH; for road: divider */
//				(31.0f / (500.0f * 8.0f)),
//				/* LeftLanes */   1,  // you’ll need real data if you have it
//				/* RightLanes */  1,
//				speedLimit(n),
//				/* Flags */      (int)n.flags9,
//				spawnRate(n) / 7.0f
//			);
//		}
//
//		PrintEmptyLine(f);
//	}
//	fclose(f);
//}
//


RwRaster*& gpWaterRaster     = *(RwRaster**)IDATRANSLATE(0x4874C8);
RwTexture*& gpWaterTex       = *(RwTexture**)IDATRANSLATE(0x4874CC);
RwRaster*& gpWaterEnvRaster  = *(RwRaster**)IDATRANSLATE(0x4874D8);
RwTexture*& gpWaterEnvTex    = *(RwTexture**)IDATRANSLATE(0x4874DC);
RwRaster*& gpWaterWakeRaster = *(RwRaster**)IDATRANSLATE(0x4874E0);
RwTexture*& gpWaterWakeTex   = *(RwTexture**)IDATRANSLATE(0x4874E4);
RwRaster*& gpBoatwakeRaster  = *(RwRaster**)IDATRANSLATE(0x4874E8);
RwTexture*& gpBoatwakeTex    = *(RwTexture**)IDATRANSLATE(0x4874EC);
RwRaster*& gpSandRaster      = *(RwRaster**)IDATRANSLATE(0x4874D0);
RwTexture*& gpSandTex        = *(RwTexture**)IDATRANSLATE(0x4874D4);
RwTexture** gpCoronaTexture  = (RwTexture**)IDATRANSLATE(0x487758);
RwTexture* tmptex;
RwRaster* tmpras;


struct tWeaponFilenames
{
	const char* name;
	const char* mask;
};
void DumptWeaponFilenames()
{
	tWeaponFilenames* pT = EMUPOINTER<tWeaponFilenames*>(0x4883F8);
	for (int i = 0; i < 72; i++)
	{
		const char* name = EMUPOINTER<const char*>((void*)pT[i].name);
		const char* mask = EMUPOINTER<const char*>((void*)pT[i].mask);
		printf("[%d] { \"%s\", \"%s\" },\n", i, name[0] != '\0' ? name : "", mask[0] != '\0' ? mask : "");
	}
}
struct tRopeFactor
{
	int32 type;
	float lengthfactor;
};
void DumpRopesHueta()
{
	tRopeFactor* pR = EMUPOINTER<tRopeFactor*>(0x00486EA0);
	printf("{");
	for (int i = 0; i < 5; i++){
		printf("%d, %f, ", pR[i].type, pR[i].lengthfactor);
	}
	printf("}");
}
void ik()
{
	CPlayerPed* pp = FindPlayerPed();
	if (!pp) return;
	CPedIK* ik = &pp->CPed.m_ik__m_pedIK;
	uint8_t* pik = OFFSET(pp, 0x330, uint8_t*);
	assert((CPedIK*)pik == ik);
	
	static int mode = 0;
	mode++;
	ik->m_flags = 0;
	if (mode > 1)
		printf("mode ik: %d\n", mode);
	float v = 3.14f;
	switch (mode)
	{
		case 2:
			ik->m_headOrient.pitch = v;
			break;
		case 3:
			ik->m_headOrient.yaw = v;
			break;

		case 4:
			ik->m_torsoOrient.pitch = v;
			break;
		case 5:
			ik->m_torsoOrient.yaw = v;
			break;

		case 6:
			ik->m_lowerArmOrient.pitch = v;
			break;
		case 7:
			ik->m_lowerArmOrient.yaw = v;
			break;

		case 8:
			ik->m_headOrient.pitch = v;
			break;
		case 9:
			ik->m_headOrient.yaw = v;
			break;

		case 10:
			ik->unkLO.pitch = v;
			break;
		case 11:
			ik->unkLO.yaw = v;
			break;
	}

}

void IK()
{
	CPlayerPed* pp = FindPlayerPed();
	if (!pp) return;
	pp->CPed.m_ik__m_pedIK.m_flags = 0xFFFFFFFF;

	//pp->CPed.m_ik__m_pedIK.m_headOrient.pitch = 3.14f / 2;
	//pp->CPed.m_ik__m_pedIK.m_headOrient.yaw = 3.14f / 2;

	//pp->CPed.m_ik__m_pedIK.m_torsoOrient.pitch = 3.14f / 2;
	//pp->CPed.m_ik__m_pedIK.m_torsoOrient.yaw = 3.14f / 2;

	//pp->CPed.m_ik__m_pedIK.m_upperArmOrient.pitch = 3.14f / 2;
	//pp->CPed.m_ik__m_pedIK.m_upperArmOrient.yaw = 3.14f / 2;

	//pp->CPed.m_ik__m_pedIK.m_lowerArmOrient.pitch = 3.14f / 2;
	//pp->CPed.m_ik__m_pedIK.m_lowerArmOrient.yaw = 3.14f / 2;

	pp->CPed.m_ik__m_pedIK.unkLO.pitch = 3.14f / 2;
	pp->CPed.m_ik__m_pedIK.unkLO.yaw = 3.14f / 2;

}


void dump77()
{
	int size;
	uintptr_t address;
	printf("enter ptr hex: ");
	scanf("%x", &address);
	printf("Enter size in decimal: ");
	scanf("%d", &size);
	printf("ptr: 0x%p\n", address);

	CVuVector* p = (CVuVector*)address;
	for (int i = 0; i < 18; i++)
	{
		printf("[%d] %f %f %f\n", i, p[i].x, p[i].y, p[i].z);
		p[i].x = p[i].y = p[i].z = 0.5f;
	}
}

void Ropes()
{
	tRopeFactor* pR = EMUPOINTER<tRopeFactor*>(0x00486EA0);
	//pR[2].lengthfactor = 30.0f; // z40 

	for (int32_t i = CPools_ms_pVehiclePool->m_nSize - 1; i >= 0; i--)
	{
		if (CPools_GetSlotIsFree(CPools_ms_pVehiclePool, i)) { continue; }
		CVehicle* v = (CVehicle*)CPools_GetSlot(CPools_ms_pVehiclePool, i, 2240);
		if (v)
		{
			TEST_OFFSET(v->CPhysical.CEntity, v->CPhysical.CEntity.CE_flags_K, 0x4E);
			//v->CPhysical.CEntity.CE_flags_K
			SET_BIT(v->CPhysical.CEntity.CE_flags_K, 5, 1); // 
		}
	}

	for (int32 i = 0; i < 8; i++)
	{
		CRope* rope = &CRopes_aRopes[i];
		//if (!rope->m_bActive) continue;
		printf("rope[%d]: ac: %d reg: %d  m_nTimeToBeKeptAliveTill %d  0x%p\n", i, rope->m_bActive, rope->m_bWasRegistered, rope->m_nTimeToBeKeptAliveTill, rope);
		if (!rope->m_bActive)
			continue;

		bool btn = GetAsyncKeyState('J') & 0x8000;
		if (btn) {
			rope->m_bActive = false;
		}

		//CEntity* WinchHookObject = EMUPOINTER<CEntity*>(rope->m_pWinchHookObject); // магнит
		//CEntity* OwnerVehicle = EMUPOINTER<CEntity*>(rope->m_pOwnerVehicle); // вертик
		//if (WinchHookObject)
		//	printf("WinchHookObject: %f %f %f\n", ((CMatrix*)WinchHookObject)->pos.x, ((CMatrix*)WinchHookObject)->pos.y, ((CMatrix*)WinchHookObject)->pos.z);
		//if (OwnerVehicle)
		//	printf("OwnerVehicle: %f %f %f\n", ((CMatrix*)OwnerVehicle)->pos.x, ((CMatrix*)OwnerVehicle)->pos.y, ((CMatrix*)OwnerVehicle)->pos.z);

		//for (int32 j = 0; j < 32; j++)
		//{
		//	printf("lnk %d: %f %f %f\n", j, rope->m_pos[j].x, rope->m_pos[j].y, rope->m_pos[j].z);
		//}
	}
}

void mlotest()
{
	CSimpleModelInfo* miwithatomics = nil;
	for (int32_t i = 0; i < CModelInfo_msNumModelInfos; i++) {
		CBaseModelInfo* modelInfo = GetModelInfo(i);
		if (modelInfo && modelInfo->m_type == MITYPE_SIMPLE && ((CSimpleModelInfo*)modelInfo)->m_atomics_objects)
		{
			miwithatomics = ((CSimpleModelInfo*)modelInfo);
			break;
		}
	}
	if (!miwithatomics)
		return;

	for (int32_t i = 0; i < CModelInfo_msNumModelInfos; i++) {
		CBaseModelInfo* modelInfo = GetModelInfo(i);
		if (modelInfo && modelInfo->m_type == MITYPE_SIMPLE)
		{
			CSimpleModelInfo* mi = (CSimpleModelInfo*)modelInfo;
			RpAtomic** at = EMUPOINTER<RpAtomic**>(mi->m_atomics_objects);
			//if(mi->m_numAtomics)
			//	printf("mi %d: simple m_numAtomics %d at  0x%p, 1st 0xp\n", i, mi->m_numAtomics, mi->m_atomics_objects/*, EMUPOINTER<RpAtomic*>(at[0])*/);
			//GetModelInfoExt(i)->allcolls.push_back(modelInfo->m_colModel);
			//if (!at)
			{
				mi->m_atomics_objects = miwithatomics->m_atomics_objects; // ps2 ptr
				mi->m_numAtomics = miwithatomics->m_numAtomics;

				//mi->m_atomics_objects = (RwObject**)0x1234567;
				//mi->m_numAtomics = 7777;
				printf("%d patched\n", i);
			}
		}
	}
}

void Muzzle()
{
	for (int i = 0; i < CMuzzleFlashes_NumMuzzleFlashes; i++) // 8
	{
		//CMuzzleFlashes_aMuzzleFlashes[];
	}
}

void TestRoll()
{
	CPlayerPed* pp = FindPlayerPed();
	if (!pp) return;
}

void TestMI()
{

	int fmi = 170;
	for (int32_t i = 0; i < CModelInfo_msNumModelInfos; i++)
	{
		CBaseModelInfo* mi = GetModelInfo(i);

		//if (mi) { printf("%d %s\n", i, mi->m_name); continue; }
		//else { continue; }

		if (mi)
		{
			//printf("%d  0x%p type: %d\n", i, mi, mi->m_type);
			switch (mi->m_type)
			{
				case MITYPE_VEHICLE:
				{
					CVehicleModelInfo* vmi = (CVehicleModelInfo*)mi;
					printf("%d flags: %X  ex&0x40? %d\n", i, vmi->flags_field_27C, vmi->flags_field_27C & 0x40);
					break;
				}
			}
		}
	}
}

void LogAnimOnce();
bool HW()
{
	setlocale(LC_NUMERIC, "C");

	//LogAnimOnce(); //---------------------------------------------

	//IK();
	//TestRoll(); 
	TestMI();
	//mlotest();
	//Ropes();
	//Muzzle();

	return true;
	///dump77(); return true;

	//gpWaterTex = gpWaterEnvTex;
	//gpWaterRaster = gpWaterEnvRaster; // krasivo

	gpWaterTex = gpCoronaTexture[2];
	gpWaterRaster = EMUPOINTER<RwTexture*>(gpWaterTex)->raster;
	//DumptWeaponFilenames();
	//DumpRopesHueta();
	//ik();

	////int& CRenderer_ms_nNoOfVisibleEmpires = *(int*)IDATRANSLATE(0x4CD5A0);

	//struct col
	//{
	//	CRGBA c;
	//	float f;
	//};
	//col* colarr = EMUPOINTER<col*>((char*)0x486990); // 47
	//for (int i = 0; i < 47; i++)
	//{
	//	printf("    { %hhu, %hhu, %hhu, %.1ff },\n", colarr[i].c.red, colarr[i].c.green, colarr[i].c.blue, colarr[i].f);
	//}


	////*(int*)IDATRANSLATE(0x48A1DC) = 2; // bank
	////*(int*)IDATRANSLATE(0x48A1E0) = 1; // bool mode unk
	////*(float*)IDATRANSLATE(0x48A1E4) = 2.0f; // scale
	////*(int*)IDATRANSLATE(0x48A1E8) = 0; // растояние widemodestuff
	////*(int*)IDATRANSLATE(0x48A1EC) = 1; // centre
	////*(int*)IDATRANSLATE(0x48A200) = 10; // unk

	//CEmpireBuildingInfo* pFirstInfo = EMUPOINTER<CEmpireBuildingInfo*>(EmpireMgr->m_pEmpiresInfosStart);
	////if (pFirstInfo && pFirstInfo->m_nBuildingState)
	////{
	////	CW_R();
	////	printf("0x%p  %d\n", pFirstInfo, pFirstInfo->m_nBuildingState);
	////	CW_G();
	////}

	//tWorldData* pWorldData = EMUPOINTER<tWorldData*>(cWorldStream->pWorldData);
	//CGroupedBuilding* m_pSwapBuildingGroups = EMUPOINTER<CGroupedBuilding*>(pWorldData ? pWorldData->m_pSwapBuildingGroups : null);
	////if (m_pSwapBuildingGroups)
	////{
	////	for (int i = 0; i < pWorldData->m_nSwapBuildingGroupsCount; i++)
	////	{
	////		//if (pSwapEntries[i].origModelIndex == 2169) { pSwapEntries[i].modelHash = 0; }
	////		//printf("i %d mi: %d, hash 0x%X\n", i, pSwapEntries[i].origModelIndex, pSwapEntries[i].modelHash);
	////		printf("%d: pActualBuilding=%d, groupNameHash=0x%X, swapState=%d, origModelIndex=%d, replacementModelIndex=%d, currentID=%d, parentArrayPtr=%p\n",
	////			i,
	////			m_pSwapBuildingGroups[i].m_pActualBuilding,
	////			m_pSwapBuildingGroups[i].m_nGroupNameHash,
	////			m_pSwapBuildingGroups[i].m_nSwapState,
	////			m_pSwapBuildingGroups[i].m_nOrigModelIndex,
	////			m_pSwapBuildingGroups[i].m_nReplacementModelIndex,
	////			m_pSwapBuildingGroups[i].m_nCurrentID,
	////			(void*)m_pSwapBuildingGroups[i].m_pParentArrayPtr);
	////	}
	////	printf("\n");
	////}


	tHandlingData* h = EMUPOINTER<tHandlingData*>(((CVehicleModelInfo*)GetModelInfo(207))->m_pHandlingData);
	printf("%f\n", h->fDragCoeff);


	sLevelChunk* chunk = EMUPOINTER<sLevelChunk*>(cWorldStream ? cWorldStream->m_pWorldDataLevelChunk : null);
	uint8_t* pBuff = EMUPOINTER<uint8_t*>(cWorldStream ? cWorldStream->m_pUncompressedLvzBuffer : null);
	printf("cws wd level chunk: 0x%p\n", chunk);
	sChunkHeader* chk = EMUPOINTER<sChunkHeader*>(chunk->sectorRows[0].header);
	printf("buff 0x%p, CHK: 0x%p\n", pBuff, chk);
	printf("0x%p\n", pBuff);


	//*(int*)pWorldData = 0;
	//for (size_t i = 0; i < 25; i++)
	//{
	//	pWorldData->m_animGroups[i].numEntries = 0;
	//	pWorldData->m_animGroups[i].listPtr = 0;
	//}
	//float& U = cWorldStream->m_fTextureAnimCurrentU;
	//float& V = cWorldStream->m_fTextureAnimCurrentV;
	//printf("U  0x%p  V  0x%p  \n", &U, &V);
	printf("7436 mi: 0x%p  col 0x%p\n", GetModelInfo(7436), EMUPOINTER<char*>(GetModelInfo(7436)->m_colModel)); // loana
	printf("7510 mi: 0x%p  col 0x%p\n", GetModelInfo(7510), EMUPOINTER<char*>(GetModelInfo(7510)->m_colModel)); // loanb
	printf("C 7466 mi: 0x%p  col 0x%p\n", GetModelInfo(7466), EMUPOINTER<char*>(GetModelInfo(7466)->m_colModel)); // loanc

	printf("6432 mi: 0x%p  col 0x%p\n", GetModelInfo(6432), EMUPOINTER<char*>(GetModelInfo(6432)->m_colModel)); // save
	printf("840 mi: 0x%p  col 0x%p\n", GetModelInfo(840), EMUPOINTER<char*>(GetModelInfo(840)->m_colModel)); // rd
	//*f1 = 0.0f;
	//TeleportPlayer({ -1527.52f, 1347.35f, -223.06f }); // colinz

	Resource* texRes83 = GetResourseFromChunk(chunk, 83); // barbershop
	Resource* texRes81 = GetResourseFromChunk(chunk, 81); // barbershop
	RwRaster* raster83 = EMUPOINTER<RwRaster*>(texRes83 ? texRes83->raster : null);
	RwRaster* raster81 = EMUPOINTER<RwRaster*>(texRes81 ? texRes81->raster : null);
	printf("Raster81: 0x%p\n", raster81);
	printf("Raster83: 0x%p\n", raster83);

	EmpireTest(0);


	//int i = (CPools_ms_pBuildingPool->m_nSize - 1);
	//while (i--) {
	//	CEntity* e = (CEntity*)CPools_GetSlot(CPools_ms_pBuildingPool, i, 96);
	//	if(e->m_rwObject)
	//	printf("%d 0x%p   rw 0x%p\n", i, e, e->m_rwObject);
	//}

	//CSpecialFX_bLiftCam = true;
	//uint8_t* pl = (uint8_t*)(EMUPOINTER<CCarPathLink*>(ThePaths->m_carPathLinks));
	//uint8_t* st = (pl + (ThePaths->m_numCarPathLinks * 4));
	////printf("%p %p @@@@@\n", pl, st);
	//memset(st, 0, (EMUPOINTER<uint8_t*>(ThePaths->m_distances) - st));

	CAutomobile* pAuto = EMUPOINTER<CAutomobile*>(FindPlayerPed() ? FindPlayerPed()->CPed.m_pMyVehicle : null);
	if (pAuto)
	{
		int s1 = 12; // from
		int s2 = 13; // to LF

		RwFrame* tmp = pAuto->m_aCarNodes[s1];
		pAuto->m_aCarNodes[s1] = pAuto->m_aCarNodes[s2];
		pAuto->m_aCarNodes[s2] = tmp;
		RESET_RECOMP_EE();
	}

	//C3dMarkers_m_pRslElementGroupArray[6] = C3dMarkers_m_pRslElementGroupArray[7];
	//for (int i = 0; i < 9; i++) // nil 0, 4, 5
	//{
	//	int select = 7;
	//	if (i == select) continue;
	//	if(C3dMarkers_m_pRslElementGroupArray[i])
	//		C3dMarkers_m_pRslElementGroupArray[i] = C3dMarkers_m_pRslElementGroupArray[select];
	//}
	return false;

	for (int32_t i = CPools_ms_pVehiclePool->m_nSize - 1; i >= 0; i--)
	{
		if (CPools_GetSlotIsFree(CPools_ms_pVehiclePool, i)) { continue; }
		CEntity* e = (CEntity*)CPools_GetSlot(CPools_ms_pVehiclePool, i, 2240);
		if (e)
		{
			//printf("0x%p AUTO:0x%p\n", e, &((CVehicle*)e)->AutoPilot);
		}
	}


	//FILE* outputFile = fopen("C:\\vcstmp\\path2.txt", "w");
	//if (!outputFile) {
	//	perror("Failed to open file");
	//	return 1;
	//}
	////DumpAllPaths(outputFile);
	//fclose(outputFile);
	if(0)
	{

		for (int i = 0; i < ThePaths->m_numCarPathLinks; i++)
		{
			ThePaths->m_carPathLinks[i].unk4 = 0;
			ThePaths->m_carPathLinks[i].unk5 = 0;
		}

		//FILE* js = fopen("C:\\vcstmp\\js.txt", "w");
		// https://ehgames.com/gta/map/
		int count = 0;
		const char* header = "{\"name\":\"VCS\",\"activeLayer\":0,\"layers\":[{\"name\":\"Default Layer\",\"mapId\":\"VCS\",\"mapScale\":0.5,\"mapOffset\":{\"x\":0,\"y\":0},\"blips\":[],\"paths\":[],\"areas\":[";
		char* buffer = (char*)malloc(2 * 1024 * 1024); // 2MB
		memset(buffer, 0, 2 * 1024 * 1024);
		int offset = 0;
		bool firstElement = true;
		offset += sprintf(buffer + offset, "%s", header);
		for (int i = 0; i < ThePaths->m_numPathNodes; i++)
		{
			//bool last = (i + 1 == ThePaths->m_numPathNodes);
			CPathNode* n = &EMUPOINTER<CPathNode*>(ThePaths->m_pathNodes)[i];
			////int l = getNumLinks(*n);
			////auto n = EMUPOINTER<CPathNodeTest*>(ThePaths->m_pathNodes);
			////DUMP_BITS2(n->flags);
			////printf("%d: ", i); DUMP_BITS(n->flags);
			//printf("%d %d\n", i, n->firstLink);
			//int l = n->numLinks_AndFlags8 & 0x0F;
			//if(l != 2)
			//printf("%d \n", l);


			//!!!!!!!!
			//{
			//	CPathNode* nodes = EMUPOINTER<CPathNode*>(ThePaths->m_pathNodes);
			//	// Текущий и следующий узел
			//	CPathNode& cur = nodes[i];
			//	int       start = cur.firstLink;
			//	int       end;

			//	if (i + 1 < ThePaths->m_numPathNodes) {
			//		// следующий узел даёт границу
			//		end = nodes[i + 1].firstLink;
			//	}
			//	else {
			//		// для последнего — общее число связей
			//		end = ThePaths->m_numConnections;
			//	}

			//	int numLinks = end - start;
			//	printf("%d Node %4d: %2d links%s\n",
			//		i, numLinks,
			//		(cur.numLinks_AndFlags8 & 0x02) ? "  [disabled]" : ""
			//	);
			//}

			uint16_t flags = *(uint16_t*)&n->numLinks_AndFlags8;
			uint16_t flags8 = n->numLinks_AndFlags8;
			uint16_t flags9 = n->flags9;

			//if (GET_BIT(flags9, 0)) { continue; }
			//if (flags8 != 2) { continue; } // разные
			//if (flags8 != 2) { printf("%d  %d\n", i, flags8); }
			//if (n->firstLink) { printf("%d \n", i); } // exists

			//if (!GET_BIT(flags9, 0)) { continue; } // water
			//if (GET_BIT(flags9, 1)) { continue; } // NONE
			uint8_t numlinks = flags8 & 0b00001111;
			//printf("%d %d\n", i, numlinks);

			//if (!GET_BIT(flags8, 0)) { continue; } //
			//if (!(numlinks > 2)) { continue; }
			//if (numlinks != 1) { continue; }
			//if (!GET_BIT(flags8, 3)) { continue; } // EMPTY, 
			//if (!GET_BIT(flags8, 4)) { continue; } // 4_bDeadEnd?, 
			//SET_BIT(n->numLinks_AndFlags8, 5, 0); // turn on all
			//SET_BIT(n->numLinks_AndFlags8, 4, 1); //
			//SET_BIT(n->numLinks_AndFlags8, 7, 0); //
			//if (!GET_BIT(flags8, 7)) { continue; } //
			//if (!GET_BIT(flags9, 6)) { continue; } // 0 always
			//if (GET_BIT(flags9, 7)) { continue; } // 1 always
			if (!GET_BIT(flags9, 4) && !GET_BIT(flags9, 5)) { continue; } // 1 always
			//if (!n->width) { continue; } // 1 always?

			//if (n->width) { continue; } //
			//if (n->firstLink) { continue; } // 
			printf("%d %d   0x%p   0x%p\n", i, (int)n->width, n, &n->width);
			//printf("%d ", i);
			//DUMP_BITS2(flags9);


			float x = n->posX / 8;
			float y = n->posY / 8;
			//fprintf(js, "{\"active\":false,\"name\":\"n_%d\",\"x\":%.2f,\"y\":%.2f,\"color\":\"#ff0000\",\"radius\":3.0},\n", i, x, y);
			if (!firstElement) { offset += sprintf(buffer + offset, ","); }
			firstElement = false;
			offset += sprintf(buffer + offset, "{\"active\":false,\"name\":\"n_%d\",\"x\":%.2f,\"y\":%.2f,\"color\":\"#ff0000\",\"radius\":3.0}", i, x, y);
			++count;
		}
		offset += sprintf(buffer + offset, "],\"zones\":[]}],\"editMode\":\"info\",\"showLayerMenu\":false,\"showToolMenu\":true}");
		copyToClipboard(buffer);
		//FILE* js = fopen("C:\\vcstmp\\json.txt", "w");
		//fprintf(js, "%s", buffer);
		//fclose(js);
		printf("res: %d/%d\n", count, ThePaths->m_numPathNodes);
		free(buffer);
	}

	//memset(&EMUPOINTER<CPathNode*>(ThePaths->m_pathNodes)[0], 0, 10 * ThePaths->m_numCarPathNodes); // no roads
	//memset(&EMUPOINTER<CPathNode*>(ThePaths->m_pathNodes)[ThePaths->m_numCarPathNodes], 0, 10 * ThePaths->m_numPathNodes); //
	//DumpPathsToFile(ThePaths, "C:\\vcstmp\\paths.txt");


	//printf("\n\n\nRaster%d: 0x%p\n", 504, EMUPOINTER<RwRaster*>(GetResourseFromChunk(chunk, 504)->raster));
	//printf("Raster%d: 0x%p\n", 502, EMUPOINTER<RwRaster*>(GetResourseFromChunk(chunk, 502)->raster));
	//printf("Raster%d: 0x%p\n", 1399, EMUPOINTER<RwRaster*>(GetResourseFromChunk(chunk, 1399)->raster));
	//printf("Raster%d: 0x%p\n", 2033, EMUPOINTER<RwRaster*>(GetResourseFromChunk(chunk, 2033)->raster));
	//printf("Raster%d: 0x%p\n", 559, EMUPOINTER<RwRaster*>(GetResourseFromChunk(chunk, 559)->raster));
	//printf("Raster%d: 0x%p\n", 496, EMUPOINTER<RwRaster*>(GetResourseFromChunk(chunk, 496)->raster));
	//printf("Raster%d: 0x%p\n", 83, EMUPOINTER<RwRaster*>(GetResourseFromChunk(chunk, 83)->raster));
	//printf("Raster%d: 0x%p\n", 3664, EMUPOINTER<RwRaster*>(GetResourseFromChunk(chunk, 3664)->raster));
	//printf("Raster%d: 0x%p\n", 874, EMUPOINTER<RwRaster*>(GetResourseFromChunk(chunk, 874)->raster));
	//printf("Raster%d: 0x%p\n", 5532, EMUPOINTER<RwRaster*>(GetResourseFromChunk(chunk, 5532)->raster));
	//printf("Raster%d: 0x%p\n", 2747, EMUPOINTER<RwRaster*>(GetResourseFromChunk(chunk, 2747)->raster));

	//for (int32_t i = 0; i < CModelInfo_msNumModelInfos; i++)
	//{
	//	CColModel* model = GetModelInfo(i)->m_colModel;
	//}

	{
		std::unordered_map<CColModel*, std::vector<int32_t>> colModelGroups;
		for (int32_t i = 0; i < CModelInfo_msNumModelInfos; i++) {
			CBaseModelInfo* modelInfo = GetModelInfo(i);
			if (modelInfo && modelInfo->m_colModel)
			{
				GetModelInfoExt(i)->allcolls.push_back(modelInfo->m_colModel);
				colModelGroups[EMUPOINTER<CColModel*>(modelInfo->m_colModel)].push_back(i);
			}
		}
		//for (const auto& group : colModelGroups) {
		//	if (group.second.size() <= 1) continue;  // Пропускаем уникальные
		//	printf("COL 0x%p\n", group.first);
		//	for (int32_t id : group.second) {
		//		//std::cout << "  " << id << std::endl;
		//		std::cout << "  " << GetModelInfoExt(id)->name << std::endl;
		//	}
		//	std::cout << "";  // Разделяем группы
		//}
		//for (int32_t i = 0; i < CModelInfo_msNumModelInfos; i++)
		//{
		//	ModelInfoExt* modelInfo = GetModelInfoExt(i);
		//	if (modelInfo && modelInfo->allcolls.size() > 1)
		//	{
		//		bool pass = true;
		//		CColModel* ps = modelInfo->allcolls[0];
		//		for (size_t j = 1; j < modelInfo->allcolls.size(); j++)
		//		{
		//			if (ps != modelInfo->allcolls[j]) { pass = false; break; }
		//		}
		//		if (!pass)
		//		{
		//			for (size_t j = 0; j < modelInfo->allcolls.size(); j++)
		//			{
		//				printf("[%d] 0x%p, %s\n", i, modelInfo->allcolls[j], GetModelInfoExt(i)->name.c_str());
		//			}
		//		}
		//	}
		//}
	}



	//{
		////struct Link
		////{
		////	int worldId; // & 7FFF    //3193
		////	int iplId;
		////	int modelId;
		////} m1716{ 0x20c79, 0x17f8, 1716 };

		//////==== Initial data for ModelInfo 1716 ====
		//////	Building ID : 3193
		//////	Sector : X = 9, Y = 16
		//////	Sector ID : 252
		//////	Instance ID : 3193
		//////	Resource ID : 71
		//////	====================================



	//	//Resource* res = &EMUPOINTER<Resource*>(chunk->resourceTable)[71]; // 1716 dff
	//	//Resource* res = GetResourseFromChunk(chunk, 71); // 1716 dff
	//	Resource* res = GetResourseFromChunk(chunk, 3659); // dff
	//	printf("1716 DFF(MDL): 0x%p\n", res);
	//	sBuildingGeometry* geom = EMUPOINTER<sBuildingGeometry*>(res->geometry);
	//	printf("geom: 0x%p\n", geom);

	//	for (int i = 0; i < geom->numMeshes; i++)
	//	{
	//		sClippableBuildingMesh* mesh = (sClippableBuildingMesh*)((uint8_t*)geom + sizeof(sBuildingGeometry) + i * sizeof(sClippableBuildingMesh));

	//		// Дамп информации о текстуре
	//		printf("Mesh %d:\n", i);
	//		printf("  Original Texture ID: %d\n", mesh->texID);
	//		mesh->texID = 83; // anim barbershop
	//		if (mesh->texID < chunk->numResources) {
	//			//Resource* texRes = &chunk->resourceTable[mesh->texID];
	//			Resource* texRes = GetResourseFromChunk(chunk, mesh->texID);
	//			RwRaster* raster = EMUPOINTER<RwRaster*>(texRes->raster);
	//			printf("  Raster pointer: %p\n", raster);
	//		}
	//		else {
	//			printf("  Invalid texture ID!\n");
	//		}
	//		printf("  New Texture ID: %d\n", mesh->texID);

	//	}
	//}

	//for (int32_t i = CTexListStore_ms_pTexListPool->m_nSize - 1; i >= 0; i--)
	//{
	//	if (CPools_GetSlotIsFree(CTexListStore_ms_pTexListPool, i)) { continue; }
	//	TxdDef* t = (TxdDef*)CPools_GetSlot(CTexListStore_ms_pTexListPool, i, 28);
	//	if (t)
	//	{
	//		RwTexDictionary* pTD = EMUPOINTER<RwTexDictionary*>(t->texDict);
	//		if (pTD)
	//		{
	//			std::vector<RwTexture*> textures = GetRwTexturesFromTxd(pTD);
	//			for (int i = 0; i < textures.size(); i++)
	//			{
	//				RwRaster* pRast = EMUPOINTER<RwRaster*>(textures[i]->raster);
	//				if (!pRast) { continue; }
	//				char* data = EMUPOINTER<char*>(pRast->data);
	//				//pRast->unk1 = 0;
	//				//pRast->unk2 = 0;
	//				//pRast->flags = 0xFFFFFFFF;
	//				//pRast->flags = 0x0;
	//				pRast->data = null;
	//				printf("rast: 0x%p   data: 0x%p\n", pRast, data);

	//			}
	//			printf("\n");
	//			//printf("count %d \n", textures.size());
	//			//printf("0x%p  0x%p \n", t, pTD);
	//			//if(textures.size())
	//			//	printf("tex0 0x%p  \n", textures[0]);
	//		}
	//	}
	//}


	//CSimpleModelInfo* p = (CSimpleModelInfo*)GetModelInfo(1716); // mainland172
	////RpAtomic* pA = EMUPOINTER<RpAtomic*>(EMUPOINTER<RpAtomic**>(p->m_atomics_objects)[0]);
	//printf("0x%p   atomic 0x%p\n", p, 0);

	//CEntity* ent = null;
	//for (int32_t i = CPools_ms_pBuildingPool->m_nSize - 1; i >= 0; i--)
	//{
	//	if (CPools_GetSlotIsFree(CPools_ms_pBuildingPool, i)) { continue; }
	//	CEntity* e = (CEntity*)CPools_GetSlot(CPools_ms_pBuildingPool, i, 96);
	//	if (e && e->m_modelIndex == 1716)
	//	{
	//		ent = e;
	//		break;
	//	}
	//}
	//if (ent)
	//{
	//	printf("0x%p   rwo 0x%p\n", ent, EMUPOINTER<RwObject*>(ent->m_rwObject));

	//}

	//for (int32_t i = 0; i < CModelInfo_msNumModelInfos; i++)
	//{
	//	CBaseModelInfo* mi = GetModelInfo(i);
	//	if (mi && (mi->m_nInteriorGroupIndex > -1))
	//	{
	//		printf("%d %d\n", i, mi->m_nInteriorGroupIndex);
	//	}
	//}
	//return false;

	//for (int32_t i = 0; i < 32; i++)
	//{
	//	*SCRVAR(1823 + i) = -1;
	//	DUMPSCRVAR(1823 + i);
	//}

	//{
	//	CInteriorPool* pool = EMUPOINTER<CInteriorPool*>(InteriorManager->m_interiorPool);
	//	printf("numgroups: %d\n", pool->m_numGroups);
	//	//pool->m_numGroups = 0;
	//	//InteriorManager->m_interiorPool = null; // нет enex
	//	int max_enties = 0;

	//	for (int32_t i = 0; i < pool->m_numGroups; i++)
	//	{
	//		CInteriorGroup* group = &pool->m_groups[i];
	//		//group->m_count = 0; // влияет
	//		//group->m_count = 1;
	//		printf("group %d\n", i);
	//		//if (i != 61) { continue; }

	//		for (int32_t j = 0; j < group->m_count; j++)
	//		{
	//			if (max_enties < group->m_count) { max_enties = group->m_count; }
	//			//group->m_pEntries = null; // не влияет лол
	//			CInteriorInfo* info = EMUPOINTER<CInteriorInfo*>(&(group->m_pEntries[j]));
	//			//memset(info, 0, sizeof(CInteriorInfo) *( group->m_count - 1));
	//			//break;
	//			//if (i != 61) { continue; }


	//			//if (info)
	//			{
	//				//printf(
	//				//	"Int [%d][%d]:\n"
	//				//	"  Type: %d\n"
	//				//	"  : %d, %d\n"
	//				//	"  Field3: %.13s\n"
	//				//	"  Coords: (%.2f, %.2f, %d, %d)\n"
	//				//	"  Float20: %.2f\n"
	//				//	"  AA_fields: %d, %d, %d\n\n",
	//				//	i, j,
	//				//	info->type_field_0,
	//				//	info->field_1, info->field_2,
	//				//	info->field_3,
	//				//	info->ffield_10, info->ffield_14, info->z_field_18, info->z_field_1C,
	//				//	info->ffield_20,
	//				//	info->AA_field_24, info->AA_field_28, info->AA_field_2C
	//				//);

	//				//if(0) // checkup
	//				//{
	//				//	if (info->type_field_0 < 0 || info->type_field_0 > 42) { MboxSTD("!! 0"); } // 
	//				//	if (info->field_1 < 0 || info->field_1 > 5) { MboxSTD("!! 1"); } // 0 1 2 3 4 5
	//				//	if (info->field_2 != 97 && info->field_2 != 98) { MboxSTD("!! 2"); } // z_field_1C -1.57
	//				//	//if (info->ffield_10 != 0.0f) { MboxSTD("!! 3"); } ffield_10 ffield_14  is x y, -6.4472 -0.15
	//				//	//if (info->z_field_18 && info->z_field_18 != 0x40933333) { MboxSTD("!! 4"); } // wtf 0.0, 0xB851B717 0x40933333 0x40866666 hash??
	//				//	if (info->z_field_1C) { MboxSTD("!! 5"); } // always 0
	//				//	//if (info->ffield_20) { MboxSTD("!! 6"); } // 1.5, -3.14

	//				//	// pads
	//				//	if (info->field_3[0] != 0xAA) { MboxSTD("!! 7"); } // AAAAA
	//				//	if (info->AA_field_24 != 0xAAAAAAAA) { MboxSTD("!! 8"); }
	//				//	if (info->AA_field_28 != 0xAAAAAAAA) { MboxSTD("!! 9"); }
	//				//	if (info->AA_field_2C != 0xAAAAAAAA) { MboxSTD("!! 10"); }
	//				//	//printf("0x%p\n ", &info->AA_field_24);
	//				//}


	//				////------------
	//				///info->m_nType = 0;
	//				//info->m_nSubType = 0; // 0-1  не влияет на магаз и на empire
	//				//info->m_chFilter = 0; // не влияет на магаз и на empire
	//				////////// pad 13 AAAA
	//				//info->m_vecOffset.x = 0.0f; // x?
	//				//info->m_vecOffset.y = 0.0f;
	//				//info->m_vecOffset.z = 0.0f;

	//				////info->z_field_18 = 0xAAAAAAAA; // hash?
	//				////// pad 0
	//				////info->ffield_20 = DEGTORAD(0); // done

	//				//// pad 4*3 AAAAAA
	//				//info->AA_field_24 = 0;
	//				//info->AA_field_28 = 0;
	//				//info->AA_field_2C = 0;
	//				//memset(info, 0, sizeof(CInteriorInfo));
	//				////memset(info, 0, 96+1);
	//				////--------------


	//				setlocale(LC_NUMERIC, "C"); // 3.14
	//				//printf("[g %d, e %d] %d|%d|%d|%f|%f|%f  |%f |%f[%d]\n",
	//				//printf("0x%p [g %d, e %d] %d %d %d    %f %f[%d]\n",
	//				//printf("%d,\t%hhu,\t%hhu,\t%hhu,\t%f,\t%f,\t%f,\t%f\n",
	//				printf("%5d, %5hhu, %5hhu, %5hhu, %12.6f, %12.6f, %12.6f, %12.6f\n",
	//					j,
	//					(uint8_t)info->m_nType,
	//					(uint8_t)info->m_nSubType,
	//					(uint8_t)info->m_chFilter,
	//					info->m_vecOffset.x,
	//					info->m_vecOffset.y,
	//					info->m_vecOffset.z,
	//					info->m_fHeading
	//				);
	//			}
	//		}
	//		printf("\n"); // group pad
	//	}
	//	printf("max ent: %d\n", max_enties);

	//}

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
bool quit = false;
int itmp = 0;
bool OnKey(int mode) // ret bool isallowhold
{
	if (!IsCurrentProcessWindowIsFocused()) { return false; }
	int32_t escalator_dtzLD = *EMUPOINTER<int32_t*>(0x004CD2EC);
	if (!escalator_dtzLD)
		return false; // no dtz load  003325C4

	if(mode == 2) {
		tmp = !tmp;
		SetPatchesState(tmp);
		RESET_RECOMP_EE(); // update pcsx2 cached mips
		return false;
	}

	//if(0)
	{
		if (HW()) { return true; }
		return false;// tmp here
	}

	switch (mode)
	{
	case 0:
	{
		if (HW()) { return true; }
		return false; // only in R

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

		//PatchCustomSCM();
		//ClearActiveList();

		// weapon test
		//{
		//	CPlayerPed* pp = FindPlayerPed();
		//	for (int i = 0; i < 20; i++) { // 10
		//		printf("%d: 0x%X\n", i, pp->CPed.m_weapons[i].field_0);
		//		pp->CPed.m_weapons[i].field_0 = 0x0;
		//	}
		//}



		CPlayerPed* pp = FindPlayerPed();
		//if (pp)
		//{
		//	pp->CPed.m_weapons[0].m_eWeaponType = 4;
		//}




		{
			for (int32_t i = CPools_ms_pPedPool->m_nSize - 1; i >= 0; i--)
			{
				if (CPools_GetSlotIsFree(CPools_ms_pPedPool, i)) { continue; }
				CPed* p = (CPed*)CPools_GetSlot(CPools_ms_pPedPool, i, 3360);
				if (p) {
					int32_t mindex = p->CPhysical.CEntity.m_modelIndex;
					CBaseModelInfo* mi = GetModelInfo(mindex);
					printf("ped[%d]: 0x%p, mi: 0x%p\n", p->CPhysical.CEntity.m_modelIndex, p, mi);
					// test code
					{
						int8_t& b1D8 = *OFFSET(p, 0x1D8, int8_t*);
						SWAP_BIT(b1D8, 3);
						//bool IsDBY = b1D8 & BIT(3);
						//b1D8 = IsDBY ? (b1D8 & (~BIT(3))) : (IsDBY | BIT(3));
					}
				}
			}
		}



		static int max1 = 0;
		static int max2 = 0;
		//for (int32_t i = 0; i < CModelInfo_msNumModelInfos; i++)
		//{
		//	CBaseModelInfo* mi = GetModelInfo(i); 

		//	//if (mi) { printf("%d %s\n", i, mi->m_name); continue; }
		//	//else { continue; }
		//	char zero[16] = { 0 };
		//	if (mi)
		//	{
		//		//printf("%d  0x%p type: %d\n", i, mi, mi->m_type);
		//		//printf("%d  0x%p e_enex: %d\n", i, mi, mi->e_enex); // enex
		//		//mi->e_enex = -1;
		//		//mi->e_enex = 9; // 0

		//		if (mi->m_type == MITYPE_PED) {
		//			CPedModelInfo* pmi = (CPedModelInfo*)mi;
		//			if (pmi->m_nNumColorVariations > max1)
		//				max1 = pmi->m_nNumColorVariations;
		//			if (pmi->m_nNumScriptColorVariations > max2)
		//				max2 = pmi->m_nNumScriptColorVariations;

		//			//int find = 0; // check if we have 4+ mat ped models
		//			//for (int i = 0; i < 6; i++) {
		//			//	if (pmi->renderMaterials[i].material) {
		//			//		find++;
		//			//	}
		//			//}
		//			//if (find <= 4)
		//			//	continue;

		//			printf("??????[%d]pmi: 0x%p\n", i, pmi);
		//			printf("??????[%d] %d\n", i, pmi->m_nNumColorVariations);
		//			printf("??????[%d] %d\n\n", i, pmi->m_nNumScriptColorVariations);
		//			//printf("???????[%d]???? 0x%p\n", i, pmi->m_anScriptColorVariationIndices);
		//			memset(zero, 0, ARRAY_SIZE(zero));
		//			//assert(!memcmp(zero, pmi->field_99, 3*9));
		//			//for (int i = 0; i < 9; i++) {
		//			//	pmi->field_99[i] = {0xFF, 0xFF, 0xFF};
		//			//}
		//				//pmi->field_99[0] = { 0xFF, 0xFF, 0xFF };
		//				//pmi->field_99[8] = { 0xFF, 0xFF, 0xFF };
		//				//pmi->field_99[9] = { 0xFF, 0xFF, 0xFF };
		//			//for (int i = 0; i < 64; i++)
		//			//{
		//			//	pmi->m_anColorVariationIndices[i] = 240; // 240 - 0
		//			//}
		//			//memset(pmi->renderMaterials, 0, 8 * 6);
		//		}
		//		//if (mi->m_type == MITYPE_VEHICLE) {
		//		//	CVehicleModelInfo* pmi = (CVehicleModelInfo*)mi;
		//		//	if (pmi->m_nNumColorVariations > max1)
		//		//		max1 = pmi->m_nNumColorVariations;
		//		//	if (pmi->m_nNumScriptColorVariations > max2)
		//		//		max2 = pmi->m_nNumScriptColorVariations;
		//		//	printf("???????[%d]???? %d\n", i, pmi->m_nNumColorVariations);
		//		//	printf("???????[%d]???? %d\n", i, pmi->m_nNumScriptColorVariations);
		//		//	printf("???????[%d]???? 0x%p\n", i, pmi->m_anScriptColorVariationIndices);
		//		//	//memset(zero, 0, ARRAY_SIZE(zero));
		//		//	//assert(!memcmp(zero, pmi->field_99, 16));
		//		//}

		//	}
		//}
		//printf("########### %d\n", max1);
		//printf("########### %d\n", max2);

		//int ssid = 0;
		//printf("enter streamedid (113 NO_SOUND): ");
		//scanf("%d", &ssid);
		//for (int i = 0; i < 57; i++) // dump cutscene audio assoc
		//{
		//	tMusicNameIdAssoc* assoc = &musicNameIdAssoc[i];
		//	char* name = EMUPOINTER<char*>(assoc->szTrackName);
		//	//if (name) { printf("%d 0x%p %s\t%d\n", i, assoc, name, assoc->iTrackId); }
		//	//if (name && (!strcmp(name, "LOUA3"))) { printf("%s\t%d\n", name, assoc->iTrackId); }
		//	if (name) { patch<int>(&(assoc->iTrackId), ssid); } // vp!
		//}
		//RESET_RECOMP_EE(); printf("ok\n"); return false;


		//for (int i = 0; i < 113; i++) // streamed 113
		//{
		//	printf("pos %d / len %d\n", MusicManager->m_aTracks[i].m_nPosition, MusicManager->m_aTracks[i].m_nLength); // fixedpointticks8.8
		//}

		//for (int i = 0; i < 75; i++) // NUMRADARBLIPS
		//{
		//	char* ppos = (((char*)TheRadar) + 0x290) + (i * 48);
		//	DUMPVEC((*(CVectorVU_align16*)ppos));
		//}

		//return false;
		tmp3 = !tmp3;
		//can_update = !can_update;
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
		if (mi) { printf("%d 0x%p\n", 7508, &mi->m_nameHashKey); }

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

		int fmi = 170;
		for (int32_t i = 0; i < CModelInfo_msNumModelInfos; i++)
		{
			CBaseModelInfo* mi = GetModelInfo(i); 

			//if (mi) { printf("%d %s\n", i, mi->m_name); continue; }
			//else { continue; }

			if (mi)
			{
				//printf("%d  0x%p type: %d\n", i, mi, mi->m_type);
				switch (mi->m_type)
				{
				case MITYPE_VEHICLE:
				{
					CVehicleModelInfo* vmi = (CVehicleModelInfo*)mi;
					printf("%d flags: %X  ex&0x40? %d\n", i, vmi->flags_field_27C, vmi->flags_field_27C & 0x40);

					//printf("%d  0x%p type: %d\n", i, mi, mi->m_type);
					//printf("mi: %d   type : %d\n", i, vmi->m_vehicleType);
					//printf("%d, %d, %d, %d, %d, %d, %d, %d\n",
					//printf("%d,\t%d,\t%d,\t%d,\t%d,\t%d,\t%d,\t%d\n",
					//	i,
					//	vmi->m_nAccelerationSampleIndex,
					//	vmi->m_nBank,
					//	vmi->m_nHornSample,
					//	vmi->m_nHornFrequency,
					//	vmi->m_nSirenOrAlarmSample,
					//	vmi->m_nSirenOrAlarmFrequency,
					//	vmi->m_bDoorType);
					//printf("%d %s\n", i, names[i - fmi].c_str());
					//fprintf(file, "%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n", i,
					//fprintf(file, "\t\t%d\t%hhu\t%d\t%d\t%d\t%d\t%hhu\n",
					//	vmi->m_SampleData.m_nAccelerationSampleIndex,
					//	vmi->m_SampleData.m_nBank,
					//	vmi->m_SampleData.m_nHornSample,
					//	vmi->m_SampleData.m_nHornFrequency,
					//	vmi->m_SampleData.m_nSirenOrAlarmSample,
					//	vmi->m_SampleData.m_nSirenOrAlarmFrequency,
					//	vmi->m_SampleData.m_bDoorType);

					if (vmi->m_vehicleType == 4 || vmi->m_vehicleType == 5) // heli || plane
					{
						//printf("%d, %s, id %d, ws %f, wsr %f\n", i, vmi->m_gameName, vmi->m_wheelId_Or_m_planeLodId_union, vmi->m_wheelScale, vmi->m_wheelScaleRear);
					}

					//vmi->m_nHornFrequency = 24000;
					break;
				}
				case MITYPE_PED:
				{

					break;
				}
				}
			}
		}
		printf("\n\n\n");
		//fclose(file);




		//int cnt_enginesets = 25;
		//for (size_t i = 0; i < cnt_enginesets; i++)
		//{
		//	printf("%s, %s\n", ((std::string)magic_enum::enum_name((eSfxSample)(aEngineSounds[i].val1))).c_str(),
		//		((std::string)magic_enum::enum_name((eSfxSample)(aEngineSounds[i].val2))).c_str());
		//}

		// dump tBounceData
		for (int32_t i = 0; i < CModelInfo_msNumModelInfos; i++)
		{
			CBaseModelInfo* mi = GetModelInfo(i); 

			//if (mi) { printf("%d %s\n", i, mi->m_name); continue; }
			//else { continue; }

			if (mi)
			{
				//printf("%d  0x%p type: %d\n", i, mi, mi->m_type);
				switch (mi->m_type)
				{
				case MITYPE_VEHICLE:
				{
					CVehicleModelInfo* vmi = (CVehicleModelInfo*)mi;
					char* tBounceData = EMUPOINTER<char*>(*(uint32_t*)(((char*)vmi) + (0x44)) );
					float* fBoatVolumeDistribution = (float*) (tBounceData ? (tBounceData + 0x40) : null);
					float* scaleMax = (float*) (tBounceData ? (tBounceData + 0x70) : null);
					float* scaleMin = (float*) (tBounceData ? (tBounceData + 0x80) : null);
					//printf("mi %d, 0x%p\n", i, vmi);
					//printf("mi %d, 0x%p 0x%p\n", i, vmi, fBoatVolumeDistribution);
					//if (fBoatVolumeDistribution) { printf("mi %d, %f %f %f %f %f %f %f %f %f\n", i,
					//	fBoatVolumeDistribution[0], fBoatVolumeDistribution[1], fBoatVolumeDistribution[2],
					//	fBoatVolumeDistribution[3], fBoatVolumeDistribution[4], fBoatVolumeDistribution[5],
					//	fBoatVolumeDistribution[6], fBoatVolumeDistribution[7], fBoatVolumeDistribution[8]
					//);
					//if (fBoatVolumeDistribution) {
					//	//printf("mi %d %s\n", i, (fBoatVolumeDistribution[8] == 0.6f ? "fBoatVolumeDistributionSpeed" : "fBoatVolumeDistribution"));
					//	printf("mi %d  MAX %f %f %f    MIN %f %f %f\n", i, scaleMax[0], scaleMax[1], scaleMax[2],
					//		scaleMin[0], scaleMin[1], scaleMin[2]);
					//}

					//printf("mi %d, 0x%p %s, t %d ws %f\n", i, vmi, vmi->m_gameName, vmi->m_vehicleType,  vmi->m_wheelScale);
					break;
				}

				}
			}
		}

		//EmpireTest(0);
		//return false;


		//dump_debug_string_array(0x4AB328, 71); // ped states
		//dump_debug_string_array(0x4ACAF8, 40); // personality
		//dump_debug_string_array(0x4B6588, 68); // sfx banks
		
		// dbg
		CPlayerPed* pPlayer = FindPlayerPed();
		int playerhandle = CPools_GetIndex(CPools_ms_pPedPool, pPlayer, 3360); // pool handle
		printf("player : 0x%p handle %d\n\n", pPlayer, playerhandle);

		tSample* pSamples = EMUPOINTER<tSample*>(SampleManager->m_aSamples);
		printf("SampleManager 0x%p\n", SampleManager);
		printf("SampleManager m_aSamples 0x%p\n", pSamples);
		printf("sfxgxt 0x%p\n", gAm_sfxgxt);


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

		int val = 0;
		printf("[~]: Enter val: ");
		scanf("%d", &val);
		//patch<char>(PCSX2POINTER(0x2EC38C), (char)val);
		//RESET_RECOMP_EE();
		can_update = !can_update;
		return false;

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
		//printf("\n");
		//const char* desc[] = { "carIds", "boatIds", "jetskiIds", "trainIds", "heliIds", "planeIds", "bikeIds", "ferryIds", "bmxIds", "quadIds" };
		//for (int i = 0; i < ARRAY_SIZE(desc); i++)
		//{
		//	//printf("%s------\n", desc[i]);
		//	printf("RwObjectNameIdAssocation %s[] = {\n", desc[i]);
		//	RwObjectNameIdAssocation* pIdAssoc = (EMUPOINTER<RwObjectNameIdAssocation*>(CVehicleModelInfo_ms_vehicleDescs[i]));
		//	while(pIdAssoc && (*(uint32_t*)pIdAssoc))
		//	{
		//		//printf("\t%s\n", EMUPOINTER<char*>(pIdAssoc->name));
		//		printf("    { \"%s\", %d, 0x%x },\n", EMUPOINTER<char*>((char*)pIdAssoc->name), pIdAssoc->hierId, pIdAssoc->flags);
		//		++pIdAssoc;
		//	}
		//	++pIdAssoc;
		//	//printf("\n");
		//	printf("    { nil, 0, 0 }\n");
		//	printf("};\n\n");
		//}

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

		//EmpireTest(0); // move upper

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
			//printf("====================VFTABLE====================\n");
			//printf("VFTABLE pPed: 0x%p\n", pPed->CPhysical.CEntity.vftable);
			//if (pVehicle) { printf("VFTABLE pVehicle: 0x%p\n", pVehicle->CPhysical.CEntity.vftable); }
			//printf("===============================================\n\n");

			printf("===============================================\n");
			printf("pPed: 0x%p  mi: %d\n", pPed, pPed->CPhysical.CEntity.m_modelIndex);
			if (pVehicle) { printf("pVehicle: 0x%p  mi: %d\n", pVehicle, pVehicle->CPhysical.CEntity.m_modelIndex); }
			if (pVehicle) { printf("pVehicle nodes: 0x%p\n", ((CAutomobile*)pVehicle)->m_aCarNodes); }
			printf("===============================================\n\n");
			//DUMPVEC("ped pos:", pPed->CPhysical.CEntity.CPlaceable.m_matrix.p);
		}

		//printf("pinfpPED: 0x%p\n", pPed);
		//printf("pinfpPEDVEHICLE: 0x%p\n", pVehicle);
		printf("gpModelIndices: 0x%p\n", gpModelIndices);
		printf("CModelInfo::ms_modelInfoPtrs: 0x%p\n", CModelInfo_ms_modelInfoPtrs);
		printf("ms_modelInfoPtrs[250]: 0x%p\n", EMUPOINTER<char*>(CModelInfo_ms_modelInfoPtrs[250]));
		printf("ms_modelInfoPtrs[214]: 0x%p\n", EMUPOINTER<char*>(CModelInfo_ms_modelInfoPtrs[214]));
		printf("ms_modelInfoPtrs[278]: 0x%p\n", EMUPOINTER<char*>(CModelInfo_ms_modelInfoPtrs[278]));
		printf("ms_modelInfoPtrs[233]: 0x%p\n", EMUPOINTER<char*>(CModelInfo_ms_modelInfoPtrs[233]));
		printf("CPopulation::ms_pPedGroups: 0x%p\n", CPopulation_ms_pPedGroups);
		//printf("empireinst: 0x%p\n", (PCSX2POINTER(*(uintptr_t**)IDATRANSLATE(0x48FF48)))); // interhyeta
		printf("cWorldStream: 0x%p\n", cWorldStream);
		printf("MusicManager: 0x%p\n", MusicManager);
		printf("AudioManager: 0x%p\n", AudioManager);
		printf("SampleManager: 0x%p\n", SampleManager);
		printf("gPhoneInfo: 0x%p\n", gPhoneInfo);
		printf("MusicManager->m_nFrontendTrack: %d\n", MusicManager->m_nFrontendTrack);
		printf("&MusicManager->m_nFrontendTrack: 0x%p\n", &MusicManager->m_nFrontendTrack);
		CW_B();
		printf("ThePaths: 0x%p\n", ThePaths);
		printf("ThePaths nodes: 0x%p %d\n", EMUPOINTER<char*>(ThePaths->m_pathNodes), ThePaths->m_numPathNodes);
		printf("ThePaths carl: 0x%p %d\n", EMUPOINTER<char*>(ThePaths->m_carPathLinks), ThePaths->m_numCarPathLinks); //? links?
		printf("ThePaths carpc: 0x%p %d\n", EMUPOINTER<char*>(ThePaths->m_carPathConnections), ThePaths->m_numConnections); //? links?
		printf("ThePaths dist: 0x%p\n", EMUPOINTER<char*>(ThePaths->m_distances));
		printf("ThePaths conn: 0x%p\n", EMUPOINTER<char*>(ThePaths->m_connections));
		CW_G();
		printf("TheDollarParticleFX: 0x%p\n", TheDollarParticleFX);
		printf("CObjectData_ms_aObjectInfo: 0x%p\n", CObjectData_ms_aObjectInfo);
		//CW_B();
		//printf("m_currentWeapon: %d\n", pPed->m_currentWeapon);
		//printf("weaptr[0]: 0x%p\n", pPed->m_weapons);
		//printf("weaptr[1]: 0x%p\n", &pPed->m_weapons[1]);
		//CW_G();
		printf("TheRadar: 0x%p\n", TheRadar);
		printf("FrontEndMenuManager: 0x%p\n", FrontEndMenuManager);
		printf("FrontEndMenuManagerSettings/Prefs: 0x%p\n", FrontEndMenuManagerSettings);
		printf("TheAnimManager: 0x%p\n", TheAnimManager);
		printf("gpSkidTex: 0x%p\n", gpSkidTex);
		printf("currentTexDict: 0x%p\n", currentTexDict);
		printf("CFont::Details: 0x%p\n", CFont_Details);
		printf("CTheScripts_ScriptSphereArray: 0x%p\n", CTheScripts_ScriptSphereArray);
		printf("C3dMarkers::m_pRslElementGroupArray: 0x%p\n", C3dMarkers_m_pRslElementGroupArray);
		printf("C3dMarkers_m_aMarkerArray: 0x%p\n", C3dMarkers_m_aMarkerArray);
		CW_B();
		printf("EmpireHud: 0x%p\n", EmpireHud);
		printf("EmpireMgr: 0x%p\n", EmpireMgr);
		printf("EmpireMgr->m_pEmpires: 0x%p\n", EMUPOINTER<CEmpireBuildingInfo*>(EmpireMgr->m_pEmpiresInfosStart));
		printf("EmpireMgr->m_pEmpires[0]->empire: 0x%p\n", EMUPOINTER<char*>(EMUPOINTER<CEmpireBuildingInfo*>(EmpireMgr->m_pEmpiresInfosStart)->
			m_pActualEmpireBuilding));
		//printf("EmpireMgr->pointer_field_14: 0x%p\n", EMUPOINTER<void*>(EmpireMgr->pointer_field_14));
		//printf("EmpireMgr->pointer_field_1C: 0x%p\n", EMUPOINTER<void*>(EmpireMgr->pointer_field_1C));
		//printf("EmpireMgr+0x10: 0x%p\n", EmpireMgr+0x10);
		//printf("EmpireMgr+0x144: 0x%p\n", EmpireMgr+0x144);
		for (int32_t i = 0; i < 16; i++)
		{
			script_sphere_struct* sp = &CTheScripts_ScriptSphereArray[i];
			if (sp->m_bInUse && sp->m_Type == 7) // MARKERTYPE_ENTERCONE
				printf("ENEX7[%d]: 0x%p, pos: 0x%p\n", i, sp, &sp->m_vecCenter);
		}
		CW_R();
		CInteriorPool* pool = EMUPOINTER<CInteriorPool*>(InteriorManager->m_interiorPool);
		CInteriorInfo* info = EMUPOINTER<CInteriorInfo*>(pool->m_groups[0].m_pEntries);
		CInteriorInfo* info61 = EMUPOINTER<CInteriorInfo*>(pool->m_groups[61].m_pEntries);
		printf("InteriorManager: 0x%p\n", InteriorManager);
		printf("InteriorManager->m_interiorPool: 0x%p\n", pool);
		printf("InteriorManager m_numGroups: %d\n", pool->m_numGroups);
		printf("InteriorManager Groups: 0x%p\n", pool->m_groups);
		printf("InteriorManager Groups[0] num: %d\n", pool->m_groups[0].m_count);
		printf("InteriorManager Groups[0] Entry: 0x%p\n", info);
		printf("InteriorManager Groups[61] num: %d\n", pool->m_groups[61].m_count);
		printf("InteriorManager Groups[61] Entry: 0x%p\n", info61);
		CW_G();
		printf("CGame::currArea:  %d   0x%p\n", CGame_currArea, &CGame_currArea);
		printf("CGame::currLevel: %d   0x%p\n", CGame_currLevel, &CGame_currLevel);

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

		CRunningScript* scr = CTheScripts_ActiveScripts;
		printf("CTheScripts::ScriptSpace: 0x%p\n", CTheScripts_ScriptSpace);
		printf("CTheScripts::ScriptsArray: 0x%p\n", CTheScripts_ScriptsArray);
		printf("CTheScripts::pActiveScripts: 0x%p\n", CTheScripts_pActiveScripts);
		printf("CTheScripts::pIdleScripts: 0x%p\n", CTheScripts_pIdleScripts);
		printf("CTheScripts::ActiveScripts: 0x%p\n", CTheScripts_ActiveScripts);
		printf("CTheScripts::IdleScripts: 0x%p\n", CTheScripts_IdleScripts);
		printf("CTheScripts::pMissionScript: 0x%p\n", CTheScripts_pMissionScript);
		//scr->m_nWakeTime = 9999999; // disable active
		printf("SCRIPT_NAME: %s IP: %d TIMERA %d\n", scr->m_abScriptName, scr->m_nIp, scr->m_anLocalVariables[TIMERA]);
		int16_t op = *((int16_t*)SCRBYTEVAR(scr->m_nIp)) & 0x7FFF;
		printf("OP[%d](%s): ip: %d %s\n", op, scr->m_abScriptName, scr->m_nIp, op < coms.size() ? coms[op].c_str() : "");
		printf("CTheScripts::MainScriptSize: %d\n", CTheScripts_MainScriptSize);
		printf("CTheScripts::LargestMissionScriptSize: %d\n", CTheScripts_LargestMissionScriptSize);
		//if (command < coms.size())
		//sprintf(tmp, "n: %s, MIP %d OP %s:0x%04X Cmp %d Not %d", m_abScriptName, oldip - mzhkstartip, coms[command].c_str(), command, m_bCondResult, m_bNotFlag);
		break;
	}
	//case 2: // dbg // mv in up
	//{
	//	tmp = !tmp;
	//	SetPatchesState(tmp);
	//	//if (tmp) { patches[ePATCH1].ApplyPatch(); }
	//	//else { patches[ePATCH1].RemovePatch(); }
	//	//printf("d: %d\n", 0);
	//	RESET_RECOMP_EE(); // update pcsx2 cached mips
	//	break;
	//}
	case 3:
	{
		testhpp();
		break;
	}
	case 4:
	{
		PatchCustomSCM();
		break;
	}
	}
	return false;
}

CAnimBlendTree* GetAnimByName(const char* name, int32_t* idx = nil)
{
	CAnimManagerInst* mgr = EMUPOINTER<CAnimManagerInst*>(TheAnimManager->mspInst);
	CAnimBlendTree* anims = EMUPOINTER<CAnimBlendTree*>(mgr->m_aAnimations); // массив

	for (int i = 0; i < mgr->m_numAnimations; i++)
	{
		//printf("%s\n", anims[i].name);
		if (!strcmp(name, anims[i].name)) {
			if (idx)
				*idx = i;
			return &anims[i];
		}
	}
	return nil;
}
CAnimBlendSequence* GetAnimKost(CAnimBlendTree* arganims, int boneID)
{
	CAnimBlendSequence* kosti = EMUPOINTER<CAnimBlendSequence*>(arganims->blendSequences); // массив костей
	for (int kostidx = 0; kostidx < arganims->numSequences; kostidx++)
		if (kosti[kostidx].boneTag == boneID)
			return kosti;
	return nil;
}
CAnimBlendSequence* GetAnimKost(int animidx, int boneID)
{
	CAnimManagerInst* mgr = EMUPOINTER<CAnimManagerInst*>(TheAnimManager->mspInst);
	CAnimBlendTree* anims = EMUPOINTER<CAnimBlendTree*>(mgr->m_aAnimations); // массив всех анимок
	CAnimBlendTree* arganims = &anims[animidx];
	return GetAnimKost(arganims, boneID);
}
CAnimBlendAssocGroup* GetAnimationGroup(int group)
{
	CAnimManagerInst* mgr = EMUPOINTER<CAnimManagerInst*>(TheAnimManager->mspInst);
	CAnimBlendAssocGroup* pGrp = &EMUPOINTER<CAnimBlendAssocGroup*>(mgr->m_aAnimAssocGroups)[group]; // 368
	return pGrp;
}
CAnimBlendAssociation* GetAnimation(int group, int animID) // 30, 2  [grp 30 firstAnimId 368]
{
	CAnimManagerInst* mgr = EMUPOINTER<CAnimManagerInst*>(TheAnimManager->mspInst);
	CAnimBlendAssocGroup* pGrp = &EMUPOINTER<CAnimBlendAssocGroup*>(mgr->m_aAnimAssocGroups)[group]; // 368
	CAnimBlendAssociation* assoc = EMUPOINTER<CAnimBlendAssociation*>(pGrp->m_aAssociationArray);
	printf("GetAnimation(group %d,anim %d): grp 0x%p fi %d descrid %d, descridx %d\n", group, animID, pGrp, pGrp->firstAnimId,
		mgr->m_aAnimDescriptors[pGrp->firstAnimId].id, animID - mgr->m_aAnimDescriptors[pGrp->firstAnimId].id);
	return &assoc[animID - mgr->m_aAnimDescriptors[pGrp->firstAnimId].id];
}

//---------------------------------------------
void TestAnims()
{
	CAnimManagerInst* mgr = EMUPOINTER<CAnimManagerInst*>(TheAnimManager->mspInst);
	CAnimBlendTree* anims = EMUPOINTER<CAnimBlendTree*>(mgr->m_aAnimations); // массив всех анимок
	int32_t idx = 0;
	CAnimBlendTree* jaw_still = GetAnimByName("jaw_still", &idx);
	CAnimBlendSequence* jaw_stillkosti = EMUPOINTER<CAnimBlendSequence*>(jaw_still->blendSequences); // массив костей

	for (int animidx = 0; animidx < mgr->m_numAnimations; animidx++)
	{
		//if (!strcmp(anims[animidx].name, "jaw_still")) // skip owner
		//	continue;

		CAnimBlendSequence* kosti = EMUPOINTER<CAnimBlendSequence*>(anims[animidx].blendSequences); // массив костей
		if (jaw_stillkosti == kosti)
			printf("!!!!!!!!!!!!!!!!!!!!!!!! %d use jaw_still KF!!!!  js id %d", animidx, idx);
		//for (int kostidx = 0; kostidx < anims[animidx].numSequences; kostidx++)
		//{
		//	KeyFrame* fotkikosti = EMUPOINTER<KeyFrame*>(kosti[kostidx].keyFrames); // массив таймингов кости
		//	for (int fotkiidx = 0; fotkiidx < kosti[kostidx].numFrames; fotkiidx++)
		//	{
		//		KeyFrame* fotka = &fotkikosti[fotkiidx];
		//	}
		//}
	}
}

void DumpAnimFlags()
{
	FILE* file = fopen("C://anim_flags.txt", "w");
	if (file == NULL) {
		printf("fak DumpAnimFlags\n");
		return;
	}

	CAnimManagerInst* mgr = EMUPOINTER<CAnimManagerInst*>(TheAnimManager->mspInst);
	AnimDescriptor* desc = mgr->m_aAnimDescriptors; // массив всех анимок
	fprintf(file, "m_numAnimDescriptors %d\n", mgr->m_numAnimDescriptors);
	for (int animidx = 0; animidx < mgr->m_numAnimDescriptors; animidx++)
	{
		//printf("%d\n", desc[animidx].defaultFlags);
		///fprintf(file, "%d\n", desc[animidx].defaultFlags);
		printf("%s %d\n", desc[animidx].name, desc[animidx].defaultFlags);
		fprintf(file, "%s %d\n", desc[animidx].name, desc[animidx].defaultFlags);
	}

	fclose(file);
	printf("zaebis!\n");
}


void ForAllAnims()
{
	//DumpAnimFlags(); return;

	CAnimManagerInst* mgr = EMUPOINTER<CAnimManagerInst*>(TheAnimManager->mspInst);
	CAnimBlendTree* anims = EMUPOINTER<CAnimBlendTree*>(mgr->m_aAnimations); // массив всех анимок
	for (int animidx = 0; animidx < mgr->m_numAnimations; animidx++)
	{
		//if (!GetAnimKost(animidx, 6)) // skip anims with non jaw
		//	continue;

		//if (strcmp(anims[animidx].name, "run_player") && strcmp(anims[animidx].name, "WALK_player"))
		//	continue;
		if (strcmp(anims[animidx].name, "jaw_still"))
			continue;

		printf("%s -------------------------------------\n", anims[animidx].name);

		CAnimBlendSequence* kosti = EMUPOINTER<CAnimBlendSequence*>(anims[animidx].blendSequences); // массив костей
		for (int kostidx = 0; kostidx < anims[animidx].numSequences; kostidx++)
		{
			printf("\t bone %d,   frames %d \n", kosti[kostidx].boneTag, kosti[kostidx].numFrames);
			if (kosti[kostidx].boneTag != 6) // logical boneID
				continue;

			//if (kosti[kostidx].boneTag < 1) // binary brute
			//	continue;

			//kosti[kostidx].numFrames = 0;
			//kosti[kostidx].keyFrames = nil;

			KeyFrame* fotkikosti = EMUPOINTER<KeyFrame*>(kosti[kostidx].keyFrames); // массив таймингов кости
			for (int fotkiidx = 0; fotkiidx < kosti[kostidx].numFrames; fotkiidx++)
			{
				KeyFrame* fotka = &fotkikosti[fotkiidx];
				//printf("\t\t frame %d \n");

			}
		}
		printf("\n\n"); // end anim pack
	}
}

void LogAnimOnce() // once
{
	ForAllAnims(); // some log
	//TestAnims();

	{ // hier log

		CAnimManagerInst* mgr = EMUPOINTER<CAnimManagerInst*>(TheAnimManager->mspInst);
		printf("mgr: 0x%p, grps 0x%p 0x%p\n", mgr, EMUPOINTER<CAnimBlendAssocGroup*>(mgr->m_aAnimAssocGroups), &EMUPOINTER<CAnimBlendAssocGroup*>(mgr->m_aAnimAssocGroups)[30]);
		CAnimBlendTree* anims = EMUPOINTER<CAnimBlendTree*>(mgr->m_aAnimations); // массив
		CAnimBlendTree* IDLE_stance = GetAnimByName("IDLE_stance");
		CAnimBlendTree* WALK_start = GetAnimByName("WALK_start");
		CAnimBlendTree* run_player = GetAnimByName("run_player"); // no 6
		CAnimBlendTree* WALK_player = GetAnimByName("WALK_player"); // no 6

		CAnimBlendTree* jaw_still = GetAnimByName("jaw_still");
		//CAnimBlendTree* jaw_test = GetAnimByName("jaw_test");
		if (!IDLE_stance)
			return;
		//printf("!!!!findres: 0x%p\n", IDLE_stance);
		//printf("!!!!WALK_start: 0x%p\n", WALK_start);
		//printf("!!!!run_player: 0x%p\n", run_player);
		//printf("!!!!WALK_player: 0x%p\n", WALK_player);
		printf("!!!!anim jaw_still: 0x%p   num seq %d\n", jaw_still, jaw_still->numSequences); // numSequences
		//printf("!!!!jaw_test: 0x%p\n", jaw_test);
		//anm->unk = 0;
		CAnimBlendSequence* IDLE_stancekosti = EMUPOINTER<CAnimBlendSequence*>(IDLE_stance->blendSequences); // массив костей
		CAnimBlendSequence* WALK_startkosti = EMUPOINTER<CAnimBlendSequence*>(WALK_start->blendSequences); // массив костей
		CAnimBlendSequence* run_playerkosti = EMUPOINTER<CAnimBlendSequence*>(run_player->blendSequences); // массив костей
		CAnimBlendSequence* jaw_stillkosti = EMUPOINTER<CAnimBlendSequence*>(jaw_still->blendSequences); // массив костей
		//CAnimBlendSequence* jaw_testkosti = EMUPOINTER<CAnimBlendSequence*>(jaw_test->blendSequences); // массив костей
		printf("!!!!jaw_stillkosti: 0x%p %d  pstptr 0x%p\n", jaw_stillkosti, jaw_stillkosti->numFrames, jaw_still->blendSequences); // 003325C4   LD
		//printf("!!!!jaw_testkosti: 0x%p  %d\n", jaw_testkosti, jaw_testkosti->keyFrames);
		//kosti[6].numFrames = 0;
		//WALK_startkosti[6].numFrames = 0;
		//for (int kostidx = 0; kostidx < IDLE_stance->numSequences; kostidx++)
		//{
		//	IDLE_stancekosti[kostidx].numFrames = 0;
		//}
		//for (int kostidx = 0; kostidx < jaw_still->numSequences; kostidx++)
		//{
		//	jaw_stillkosti[kostidx].numFrames = 0;
		//}

		//memset(jaw_stillkosti, 0, sizeof(CAnimBlendSequence) * jaw_still->numSequences); // при ходьбе ломается челюсть
		//memset(jaw_still, 0, sizeof(CAnimBlendTree)); // не влияет
		//Sleep(1000 * 2);
		//return;

		{ // WALK_player
			// revcs firstAnimId 2
			// vcs firstAnimId 368  (grp 30)
			//CAnimBlendTree* jaw_still = GetAnimByName("jaw_still");
			//CAnimBlendSequence* jaw_stillkosti = EMUPOINTER<CAnimBlendSequence*>(jaw_still->blendSequences); // массив костей

			int g = 30;
			int a = 2;
			CAnimBlendAssocGroup* grp = GetAnimationGroup(g);
			CAnimBlendAssociation* assoc = GetAnimation(g, a);
			printf("base assoc 0x%p   grp 0x%p\n", assoc, grp);
			CAnimBlendNode* nodes = EMUPOINTER<CAnimBlendNode*>(assoc->m_pAnimBlendNodes);
			for (int i = 0; i < assoc->m_iNumAnimBlendNodes; i++)
			{
				// стёр все kf анимки челюсти сломались, в листе partial jaw_still не было я понял не хардкод поворот 6й кости
				// бинарным поиском стирания kf нашёл что в movement челюсть фикситься из kf jaw_still. в idle есть своя 6я кость
				// bp чтение kf jawstill вытащил стек кто читает - CAnimBlendNode m_pSequence, начал перебирать все seq анимок и нашёл тот же указатель на
				// jaw_still
				//стираю CAnimBlendTree(там где "jaw_still") всё работает
				//стираю кости(1) CAnimBlendSequence ломаеться  sizeof 0xC
				//при загрузке первое чтение кости в 00280FCC  CAnimBlendNode::FindKeyFrame  в  if (Sequence->numFrames <= 0i64)
				//from CAnimBlendAssociation::SetCurrentTime from CAnimBlendAssociation::Start from CAnimManager::AddAnimationAndSync
				//from CAnimManager::BlendAnimation from CPed::BlendAnimation from CPed::SetInTheAir
				CAnimBlendSequence* s = EMUPOINTER<CAnimBlendSequence*>(nodes[i].m_pSequence); // массив костей
				CAnimBlendSequence* ms = EMUPOINTER<CAnimBlendSequence*>(nodes[i].m_pMirroredSequence); // массив костей
				if (s == jaw_stillkosti || ms == jaw_stillkosti) // read jaw still bp stack
					printf("!!!!!FIND Sequence jaw_still in WALK_player idx %d\n", i); // exists kf from jaw still into WALK_player base assoc nodes
			}
			//CAnimBlendAssocGroup* pGrp = &EMUPOINTER<CAnimBlendAssocGroup*>(mgr->m_aAnimAssocGroups)[g]; // 368
			//printf("FI %d\n", pGrp->firstAnimId);

			for (int i = 0; i < mgr->m_numAnimations; i++)
			{
				CAnimBlendSequence* s = EMUPOINTER<CAnimBlendSequence*>(anims[i].blendSequences); // массив костей
				for (int j = 0; j < anims[i].numSequences; j++)
				{
					if (&(s[j]) == jaw_stillkosti) // chechk anims // only owner jaw_still
						printf("!!!!!FIND Sequence jaw_still idx %d  %d\n", i, j); // exists kf from jaw still into WALK_player base assoc nodes
				}
			}
		}



		{
			//for (int i = 0; i < 200; i++) { // AnimAssocDefinition
			//	printf("AnimAssocDefinition pName: %s, pBlockName %s\n", mgr->m_aAnimAssocDefinitions[i].pName, mgr->m_aAnimAssocDefinitions[i].pBlockName);
			//}
			//for (int i = 0; i < 990; i++) { // AnimDescriptor
			//	printf("AnimDescriptor name: %s, id %d\n", mgr->m_aAnimDescriptors[i].name, mgr->m_aAnimDescriptors[i].id);
			//}
			//CAnimBlendTree* tree = EMUPOINTER<CAnimBlendTree*>(mgr->m_aAnimations); // массив костей
			//for (int i = 0; i < mgr->m_numAnimations; i++) { // AnimDescriptor
			//	printf("CAnimBlendTree name: %s, numseq %d\n", tree[i].name, tree[i].numSequences);
			//}
			//CAnimBlock* blocks = EMUPOINTER<CAnimBlock*>(mgr->m_aAnimBlocks); // массив костей
			//for (int i = 0; i < mgr->m_numAnimBlocks; i++) { // AnimDescriptor
			//	printf("CAnimBlock name: %s, numanims %d\n", blocks[i].m_name, blocks[i].m_numAnims);
			//}
		}

		CPlayerPed* pp = FindPlayerPed();
		if(pp)
		{ // try link ------------------------------------------  003325C4

			RpClump* clump = EMUPOINTER<RpClump*>(pp->CPed.CPhysical.CEntity.m_urwObject.m_rpClump);
			if (!clump)
				return;
			CAnimBlendClumpData* pClumpext = EMUPOINTER<CAnimBlendClumpData*>(clump->pClumpAnimDataPlugin);
			AnimBlendFrameData* frames = EMUPOINTER<AnimBlendFrameData*>(pClumpext->frames);
			for (int i = 0; i < pClumpext->numFrames; i++)
			{
				int nid = frames[i].nodeID;
				//printf("%d  H 0x%p\n", nid, frames[i].uFrameData.hanimFrame);
				if (nid == 6) {
					frames[i].uFrameData.hanimFrame = nil;
					frames[i].nodeID = 0;
				}
				
			}
			printf("\n\n");
		}


		for (int animidx = 0; animidx < mgr->m_numAnimations / 2; animidx++)
		{
			//if (!GetAnimKost(animidx, 6)) // skip anims with non jaw
			//	continue;

			CAnimBlendSequence* kosti = EMUPOINTER<CAnimBlendSequence*>(anims[animidx].blendSequences); // массив костей
			for (int kostidx = 0; kostidx < anims[animidx].numSequences; kostidx++)
			{
				if (kosti[kostidx].boneTag != 6) // logical boneID
					continue;

				//kosti[kostidx].numFrames = 0;
			}
		}



		for (int i = 0; i < IDLE_stance->numSequences; i++)
		{
			KeyFrame* fotki = EMUPOINTER<KeyFrame*>(IDLE_stancekosti[i].keyFrames); // массив таймингов кости
			//IDLE_stancekosti[i].numFrames = 0;



			//printf("%d ", kosti[i].boneTag);
			//if (kosti[i].boneTag == 6)
			//	kosti[i].boneTag = 5;
			//kosti[i].numFrames = 0;
		}

		//for (int i = 0; i < mgr->m_numAnimations; i++)
		//{
		//	//printf("%s\n", anims[i].name);

		//}


		//CAnimBlendTree* hier = EMUPOINTER<CAnimBlendTree*>(assoc->m_pAnimBlendHierarchy); // вся инфа о анимке из ifp
		//AnimAssocDefinition* def = &(EMUPOINTER<CAnimManagerInst*>(TheAnimManager->mspInst))->m_aAnimAssocDefinitions[assoc->groupId];
		//AnimDescriptor* desc = nil;
		//if (def) {
		//	for (int idx = def->firstAnim; idx < def->firstAnim + def->numAnims; ++idx) {
		//		AnimDescriptor* d = &(EMUPOINTER<CAnimManagerInst*>(TheAnimManager->mspInst))->m_aAnimDescriptors[idx];
		//		if (d && d->id == assoc->animId) {
		//			desc = d;
		//			break;
		//		}
		//	}
		//}
		//printf("%s %d\n", hier->name, hier->unk);



		//	bool jaw = false;
		//	char boneBuff[256] = { 0 };
		//	int offset = 0;

		//	for (int i = 0; i < hier->numSequences; i++)
		//	{
		//		int boneTag = hier->blendSequences[i].boneTag;

		//		if (i == 0) {
		//			offset += sprintf(boneBuff + offset, "%d", boneTag);
		//		}
		//		else {
		//			offset += sprintf(boneBuff + offset, ", %d", boneTag);
		//		}

		//		if (!jaw)
		//			jaw = boneTag == 6;
		//	}
		//	//printf("Bone IDs [%d]: %s\n", hier->numSequences, boneBuff);
		//	//if (!jaw)
		//		//printf("!jaw\n");
	}
}

void PrintPedsAnimsData(CPed* ped) // quat
{
#define RpAtomic_fromClump(ptr) ( (RpAtomic*) (((uint8_t*)ptr) - 0x1C) ) // RslElementGroupForAllElements + inElementGroupLink__inClump: link ptr - 0x1C = pAtomic*

	char buf[256];

	CAnimManagerInst* mgr = EMUPOINTER<CAnimManagerInst*>(TheAnimManager->mspInst);
	RpClump* clump = EMUPOINTER<RpClump*>(ped->CPhysical.CEntity.m_urwObject.m_rpClump);
	if (!clump)
		return;


	// clump.list.start(next) -> atomic link -> last? -> clump.list
	RpHAnimHierarchy* pHier = nil;
	RwLLLink* head = &clump->atomicList.link; // наш линк откуда мы нашли голову листа, елемент листа последний некст указывает на вот это поле
	//debug("head : 0x%p\n", head); // field clump
	RpAtomic a;
	assert(((uint8_t*)&a) == ((uint8_t*)&a.inElementGroupLink - 0x1C) ); // stru test

	int i = 0;
	//for (RwLinkList* link = EMUPOINTER<RwLinkList*>(clump->atomicList.link.next); link; link = EMUPOINTER<RwLinkList*>(link->link.next)) // wrong looping
	for (RwLLLink* link = EMUPOINTER<RwLLLink*>(head->next); link != head; link = EMUPOINTER<RwLLLink*>(link->next)) // FORLIST
	{
		debug("link : 0x%p  0x%p\n", link, head->next);

		RpAtomic* atomic = RpAtomic_fromClump(link); // FORLIST
		debug("ATOMIC!!! [%d] : 0x%p   lnk 0x%p\n", i++, atomic, link);
		//debug("rc : 0x%p\n", atomic->renderCallBack);
		pHier = EMUPOINTER<RpHAnimHierarchy*>(atomic->pAtomicHAnimHierarchyPlugin);
	}
	if (!pHier)
		return;
	debug("pHier : 0x%p numNodes: %d\n", pHier, pHier->numNodes);


	CAnimBlendClumpData* pClumpext = EMUPOINTER<CAnimBlendClumpData*>(clump->pClumpAnimDataPlugin);
	debug("pClumpext 0x%p \n", pClumpext);
	AnimBlendFrameData* frames = EMUPOINTER<AnimBlendFrameData*>(pClumpext->frames);
	debug("frames 0x%p\n", frames);
	RpHAnimStdInterpFrame* kf = EMUPOINTER<RpHAnimStdInterpFrame*>(frames->uFrameData.hanimFrame);
	debug("hanimFrame kf 0x%p \n", kf);
	int idx = 6;
	//debug("[quat bone:%d]: %f %f %f %f\n", idx, kf[idx].quad.imag.x, kf[idx].quad.imag.y, kf[idx].quad.imag.z, kf[idx].quad.real);

	if(0)
	{
		kf[idx].quad.imag.x = 0.0f;
		kf[idx].quad.imag.y = 0.0f;
		kf[idx].quad.imag.z = 0.0f;
		kf[idx].quad.real = 0.0f;
	}
	//CTimer_ms_fTimeScale = 0.2f;

	//for (int i = 0; i < pHier->numNodes; i++)
	{
		//RpHAnimStdInterpFrame* kf = (RpHAnimStdInterpFrame*)rpHANIMHIERARCHYGETINTERPFRAME(hier, i);
		//sprintf(buf, "%6.3f %6.3f %6.3f  %6.3f  %6.3f %6.3f %6.3f  %d %s %d",
		//	kf->q.imag.x, kf->q.imag.y, kf->q.imag.z, kf->q.real,
		//	kf->t.x, kf->t.y, kf->t.z,
		//	HIERNODEID(hier, i),
		//	ConvertBoneTag2BoneName(HIERNODEID(hier, i)), i);
		//CDebug::PrintAt(buf, 10, 1 + i * 2);

		//RwMatrix* m = &RpHAnimHierarchyGetMatrixArray(hier)[i];
		//sprintf(buf, "%6.3f %6.3f %6.3f %6.3f",
		//	m->right.x, m->up.x, m->at.x, m->pos.x);
		//CDebug::PrintAt(buf, 80, 1 + i * 3 + 0);
		//sprintf(buf, "%6.3f %6.3f %6.3f %6.3f",
		//	m->right.y, m->up.y, m->at.y, m->pos.y);
		//CDebug::PrintAt(buf, 80, 1 + i * 3 + 1);
		//sprintf(buf, "%6.3f %6.3f %6.3f %6.3f",
		//	m->right.z, m->up.z, m->at.z, m->pos.z);
		//CDebug::PrintAt(buf, 80, 1 + i * 3 + 2);
	}
}

void PrintPedsAnims(CPed* ped)
{
#define CAnimBlendAssociation_FromLink(ptr) ( (CAnimBlendAssociation*) (((uint8_t*)ptr) - 0) ) // lnk is first field

	if (!ped)
		return;
	PrintPedsAnimsData(ped);
	//return;

	static const char* animFlagsNames[] = {
		"RUNNING",
		"REPEAT",
		"DELFADEDOUT",
		"FDOUTWHNDONE",
		"PARTIAL",
		"MOVEMENT",
		"TRANSL",
		"X_TRANS",
		"WALK",
		"IDLE",
		"NOWALK",
		"BLOCK",
		"FRONTAL",
		"DRIVING",
		"4000",
		"MIRROR",
		"10000",
		"20000",
		"SRPT",
		"80000",
		"JAW",
	};

	//CAnimBlendClumpData* clumpData = *RPANIMBLENDCLUMPDATA(pClump);
	//for (CAnimBlendLink* link = clumpData->link.next; link; link = link->next)
	//{
	//	CAnimBlendAssociation* assoc = CAnimBlendAssociation::FromLink(link);
	//}
	//static CAnimBlendAssociation* FromLink(CAnimBlendLink * l)
	//{
	//	return (CAnimBlendAssociation*)((uint8*)l - offsetof(CAnimBlendAssociation, link));
	//}
	if((CPed*)FindPlayerPed() == ped)
		printf("PLAYER \n");

	CAnimManagerInst* mgr = EMUPOINTER<CAnimManagerInst*>(TheAnimManager->mspInst);
	RpClump* clump = EMUPOINTER<RpClump*>(ped->CPhysical.CEntity.m_urwObject.m_rpClump);
	if (!clump)
		return;

	printf("m_numAnimAssocDefinitions %d\n", mgr->m_numAnimAssocDefinitions);
	printf("m_numAnimDescriptors %d\n", mgr->m_numAnimDescriptors);
	printf("m_numAnimationIds %d\n", mgr->m_numAnimationIds);
	printf("m_numAnimations %d\n", mgr->m_numAnimations);
	printf("m_numAnimBlocks %d\n", mgr->m_numAnimBlocks);

	CAnimBlendClumpData* pClumpext = EMUPOINTER<CAnimBlendClumpData*>(clump->pClumpAnimDataPlugin);
	debug("clump 0x%p ", clump);
	printf("mi [%d] clump 0x%p pClumpext 0x%p NF %d\n", ped->CPhysical.CEntity.m_modelIndex, clump, pClumpext, pClumpext->numFrames);
	int i = 0;
	for (CAnimBlendLink* link = EMUPOINTER<CAnimBlendLink*>(pClumpext->link.next); link; link = EMUPOINTER<CAnimBlendLink*>(link->next))
	{
		CAnimBlendAssociation* assoc = CAnimBlendAssociation_FromLink(link);
		if (!assoc) { ++i; continue; }
		//printf("0x%p\n", assoc);
		//printf("[%d] grp %d,  anim %d \n", i, assoc->groupId, assoc->animId);
		const char* grpname = mgr->m_aAnimAssocDefinitions[assoc->groupId].pName;
		//printf("assoc->m_pAnimBlendHierarchy 0x%p\n", assoc->m_pAnimBlendHierarchy);

		// extract anium name
		const char* animname = "UNKNOWN";
		CAnimBlendTree* hier = EMUPOINTER<CAnimBlendTree*>(assoc->m_pAnimBlendHierarchy);
		if (hier && hier->name && hier->name[0] != '\0') {
			animname = hier->name;
		}
		else {
			AnimAssocDefinition* def = &mgr->m_aAnimAssocDefinitions[assoc->groupId];
			if (def) {
				for (int idx = def->firstAnim; idx < def->firstAnim + def->numAnims; ++idx) {
					AnimDescriptor* d = &mgr->m_aAnimDescriptors[idx];
					if (d && d->id == assoc->animId) {
						animname = d->name;
						break;
					}
				}
			}
		}

		// extract anim szflags
		char flagsBuf[256] = { 0 };
		size_t left = sizeof(flagsBuf);
		int32_t first = 1;
		for (int32_t b = 0; b < (int32_t)(sizeof(animFlagsNames) / sizeof(animFlagsNames[0])); ++b) {
			uint32_t mask = BIT(b);
			if (assoc->m_bitsFlags & mask) {
				if (!first) { strncat(flagsBuf, " ", left - 1); left = sizeof(flagsBuf) - strlen(flagsBuf); }
				strncat(flagsBuf, animFlagsNames[b], left - 1); left = sizeof(flagsBuf) - strlen(flagsBuf);
				first = 0;
			}
		}
		if (flagsBuf[0] == '\0') strcpy(flagsBuf, "NONE");


		CAnimBlendTree* fhier = EMUPOINTER<CAnimBlendTree*>(assoc->m_pAnimBlendHierarchy); // вся инфа о анимке из ifp
		bool hasjawBoneID = !!GetAnimKost(fhier, 6);

		printf("assoc 0x%p %d: A:%.2f D:%.2f grp: %s[%d] anm: %s[%d] flgs [%s]\n", assoc, i, assoc->m_fBlendAmount, assoc->m_fBlendDelta, grpname,
			assoc->groupId, animname, assoc->animId, flagsBuf);
		printf("hasjawBoneID %d\n\n", hasjawBoneID);
		//assoc->m_bitsFlag__flags = 0;
		//assoc->m_bitsFlag__flags |= ASSOC_FADEOUTWHENDONE;

		// for all running anims
		{ // hier log
			//CAnimBlendTree* hier = EMUPOINTER<CAnimBlendTree*>(assoc->m_pAnimBlendHierarchy); // вся инфа о анимке из ifp
			//AnimAssocDefinition* def = &(EMUPOINTER<CAnimManagerInst*>(TheAnimManager->mspInst))->m_aAnimAssocDefinitions[assoc->groupId];
			//AnimDescriptor* desc = nil;
			//if (def) {
			//	for (int idx = def->firstAnim; idx < def->firstAnim + def->numAnims; ++idx) {
			//		AnimDescriptor* d = &(EMUPOINTER<CAnimManagerInst*>(TheAnimManager->mspInst))->m_aAnimDescriptors[idx];
			//		if (d && d->id == assoc->animId) {
			//			desc = d;
			//			break;
			//		}
			//	}
			//}
			//printf("%s %d\n", hier->name, hier->unk);


			CAnimBlendNode* nodes = EMUPOINTER<CAnimBlendNode*>(assoc->m_pAnimBlendNodes);
			CAnimBlendTree* jaw_still = GetAnimByName("jaw_still");
			CAnimBlendSequence* jaw_stillkosti = EMUPOINTER<CAnimBlendSequence*>(jaw_still->blendSequences); // массив костей
			printf("%d\n", TheAnimManager->mspInst);

			for (size_t i = 0; i < assoc->m_iNumAnimBlendNodes; i++)
			{
				//if (EMUPOINTER<CAnimBlendSequence*>(nodes[i].m_pSequence) == jaw_stillkosti || EMUPOINTER<CAnimBlendSequence*>(nodes[i].m_pMirroredSequence) == jaw_stillkosti)
				//	printf("!!!!!!!!!!!!!!! FIND IN RUNNING ANIM SEQ FROM in grp: %s[%d] anm: %s[%d] jaw_still!!!!!!\n\n", grpname,
				//		assoc->groupId, animname, assoc->animId); // hehe find


			}

			//{ // WALK_player
			//	// revcs firstAnimId 2
			//	// vcs firstAnimId 368  (grp 30)
			//	CAnimBlendTree* jaw_still = GetAnimByName("jaw_still");
			//	CAnimBlendSequence* jaw_stillkosti = EMUPOINTER<CAnimBlendSequence*>(jaw_still->blendSequences); // массив костей

			//	int g = 30;
			//	int a = 2;
			//	CAnimBlendAssocGroup* grp = GetAnimationGroup(g);
			//	CAnimBlendAssociation* assoc = GetAnimation(g, a);
			//	printf("base assoc 0x%p   grp 0x%p\n", assoc, grp);
			//	CAnimBlendNode* nodes = EMUPOINTER<CAnimBlendNode*>(assoc->m_pAnimBlendNodes);
			//	for (int i = 0; i < assoc->m_iNumAnimBlendNodes; i++)
			//	{
			//		CAnimBlendSequence* s = EMUPOINTER<CAnimBlendSequence*>(nodes[i].m_pSequence); // массив костей
			//		CAnimBlendSequence* ms = EMUPOINTER<CAnimBlendSequence*>(nodes[i].m_pMirroredSequence); // массив костей
			//		if (s == jaw_stillkosti || ms == jaw_stillkosti)
			//			printf("!!!!!FIND WALK_player\n"); // exists
			//	}
			//	//CAnimBlendAssocGroup* pGrp = &EMUPOINTER<CAnimBlendAssocGroup*>(mgr->m_aAnimAssocGroups)[g]; // 368
			//	//printf("FI %d\n", pGrp->firstAnimId);
			//}
			



		//	bool jaw = false;
		//	char boneBuff[256] = { 0 };
		//	int offset = 0;

		//	for (int i = 0; i < hier->numSequences; i++)
		//	{
		//		int boneTag = hier->blendSequences[i].boneTag;

		//		if (i == 0) {
		//			offset += sprintf(boneBuff + offset, "%d", boneTag);
		//		}
		//		else {
		//			offset += sprintf(boneBuff + offset, ", %d", boneTag);
		//		}

		//		if (!jaw)
		//			jaw = boneTag == 6;
		//	}
		//	//printf("Bone IDs [%d]: %s\n", hier->numSequences, boneBuff);
		//	//if (!jaw)
		//		//printf("!jaw\n");
		}


		++i;
	}
	//if (GetAsyncKeyState('S') & 0x8000) { // del 1st anim
	//	CAnimBlendLink* link = EMUPOINTER<CAnimBlendLink*>(clump->ClumpAnimDataPlugin.link.next);
	//	link = EMUPOINTER<CAnimBlendLink*>(link->next); // next
	//	clump->ClumpAnimDataPlugin.link.next = link; // CAnimBlendAssociation_ToLink
	//	Sleep(1000);
	//}
	if (GetAsyncKeyState('A') & 0x8000) {
		//clump->pClumpAnimDataPlugin->link.next = nil;
		CTimer_ms_fTimeScale = 0.1f;
	}
	printf("\n");
	//clump->ClumpAnimDataPlugin.link.next = null;


	//int nf = clump->ClumpAnimDataPlugin.numFrames;
	//printf("----------------0x%p----------%d\n", clump, nf);


		//	CPed* pPed = (CPed*)this;
		//	CAnimBlendClumpData* clumpData = *RPANIMBLENDCLUMPDATA(pPed->GetClump());
		//	for (CAnimBlendLink* link = clumpData->link.next; link; link = link->next)
		//	{
		//		CAnimBlendAssociation* assoc = CAnimBlendAssociation::FromLink(link);

		//		CAnimManager::AnimAssocDefinition* def = TheAnimManager->GetAnimDefinition((AssocGroupId)assoc->groupId);
		//		const char* animname = "UNKNOWN";
		//		if (assoc->hierarchy && assoc->hierarchy->name && assoc->hierarchy->name[0] != '\0')
		//			animname = assoc->hierarchy->name;
		//		else {
		//			CAnimManager::AnimAssocDefinition* def = TheAnimManager->GetAnimDefinition((AssocGroupId)assoc->groupId);
		//			if (def) {
		//				for (int idx = def->firstAnim; idx < def->firstAnim + def->numAnims; ++idx) {
		//					CAnimManager::AnimDescriptor* d = TheAnimManager->GetAnimDescriptor(idx);
		//					if (d && d->id == assoc->animId) {
		//						animname = d->name;
		//						break;
		//					}
		//				}
		//			}
		//		}
		//		const char* grpname = TheAnimManager->GetAnimGroupName((AssocGroupId)assoc->groupId);
		//		char flagsBuf[256] = { 0 };
		//		size_t left = sizeof(flagsBuf);
		//		int32 first = 1;
		//		for (int32 b = 0; b < (int32)(sizeof(animFlagsNames) / sizeof(animFlagsNames[0])); ++b) {
		//			uint32_t mask = BIT(b);
		//			if (assoc->flags & mask) {
		//				if (!first) { strncat(flagsBuf, " ", left - 1); left = sizeof(flagsBuf) - strlen(flagsBuf); }
		//				strncat(flagsBuf, animFlagsNames[b], left - 1); left = sizeof(flagsBuf) - strlen(flagsBuf);
		//				first = 0;
		//			}
		//		}
		//		if (flagsBuf[0] == '\0') strcpy(flagsBuf, "NONE");
		//		sprintf(buf, "%d: A:%.2f D:%.2f grp: %s[%d] anm: %s [%d] flgs [%s]\n", i, assoc->blendAmount, assoc->blendDelta, grpname, assoc->groupId, animname, assoc->animId, flagsBuf);
		//		AsciiToUnicode(buf, wbuf);
		//		CVector p = GetPosition();
		//		SetPosition(p + CVector(0, 0, 0.5f)); // tmp funny lazy hack
		//		PrintDebugString(this, wbuf, i++);
		//		SetPosition(p);
		//	}
		//}
}




CVehicle* pVlast = null;
void UpdNonSyncStuff()
{
	*CClock_ms_nGameClockHours = 0;
	if ((GetAsyncKeyState('Q') & 0x8000)) { CTimer_ms_fTimeScale = CTimer_ms_fTimeScale == 1.0f ? 0.0f : 1.0f; Sleep(1000); }
	if ((GetAsyncKeyState('U') & 0x8000)) { can_update ^= true; Sleep(1000); }
	if (!can_update) { return; }
	if (!(GetAsyncKeyState(VK_CONTROL) & 0x8000)) system("cls");

	//TestingPools();
	printf("UpdNonSyncStuff() \n");
	//globalRenderFlags = 0;

	//{
	//	char* CTheScripts_IntroTextLines = EMUPOINTER<char*>(0x0050D010);
	//	char* CTheScripts_NumberOfIntroTextLinesThisFrame = EMUPOINTER<char*>(0x004CD3F8);
	//	memset(CTheScripts_IntroTextLines, 0, 0x0050DD30 - 0x0050D010);
	//	memset(CTheScripts_NumberOfIntroTextLinesThisFrame, 0, 2);
	//}

	//printf("mp 0x%p\n", cWorldStream);
	//printf("mp 0x%p\n", ((char*)cWorldStream) + 0x248);
	//printf("mp 0x%p\n", *  (uint32_t*)   (((char*)cWorldStream) + 0x248)    );
	//printf("mp 0x%p\n", EMUPOINTER<char*>(*(uint32_t*)(((char*)cWorldStream) + 0x248)));


	CVector pos = FindPlayerPos();
	CVehicle* veh = FindPlayerVehicle();
	//return; //-------------------------------------
	CPlayerPed* pPlayer = FindPlayerPed();
	//PrintPedsAnims((CPed*)pPlayer);
	Ropes();


	// log pool
	if(0)
	{
		for (int32_t i = CPools_ms_pVehiclePool->m_nSize - 1; i >= 0; i--)
		{
			if (CPools_GetSlotIsFree(CPools_ms_pVehiclePool, i)) { continue; }
			CVehicle* e = (CVehicle*)CPools_GetSlot(CPools_ms_pVehiclePool, i, 2240);
			if (e)
			{
				int idx = e->CPhysical.CEntity.m_modelIndex;
				CBaseModelInfo* mi = GetModelInfo(idx);
				if (e->m_vehicleType == 0)
					printf("[CAR %d %s]: 0x%p [modelinfo 0x%p] [han: 0x%p]\n", idx, GetModelInfoExt(idx)->name.c_str(), e, mi, ((CVehicleModelInfo*)mi)->m_pHandlingData);
				if (e->m_vehicleType == 1 || e->m_vehicleType == 2)
					printf("[BOAT/JETSKI %d %s]: 0x%p [modelinfo 0x%p] [han: 0x%p]\n", idx, GetModelInfoExt(idx)->name.c_str(), e, mi, ((CVehicleModelInfo*)mi)->m_pHandlingBoat);
				if (e->m_vehicleType == 6)
					printf("[BIKE %d %s]: 0x%p [modelinfo 0x%p] [han: 0x%p]\n", idx, GetModelInfoExt(idx)->name.c_str(), e, mi, ((CVehicleModelInfo*)mi)->m_pHandlingBike);
				if (e->m_vehicleType == 8)
					printf("[BMX %d %s]: 0x%p [modelinfo 0x%p] [han: 0x%p]\n", idx, GetModelInfoExt(idx)->name.c_str(), e, mi, ((CVehicleModelInfo*)mi)->m_pHandlingBike);
				if (e->m_vehicleType == 9)
					printf("[QUAD %d $s]: 0x%p [modelinfo 0x%p] [han: 0x%p]\n", idx, GetModelInfoExt(idx)->name.c_str(), e, mi, ((CVehicleModelInfo*)mi)->m_pHandlingData);
			}
		}
	}


	if (veh)
	{
		float* plane = (float*)(((char*)veh) + 0x820);
		//printf("[0] %f, [1] %f, [2] %f, [3] %f, [4] %f\n", plane[0], plane[1], plane[2], plane[3], plane[4]);

		{
			float magnitude = sqrtf(
				((veh->CPhysical.m_vecMoveSpeed.x * veh->CPhysical.m_vecMoveSpeed.x)
					+ (veh->CPhysical.m_vecMoveSpeed.y * veh->CPhysical.m_vecMoveSpeed.y))
				+ (veh->CPhysical.m_vecMoveSpeed.z * veh->CPhysical.m_vecMoveSpeed.z));
			if (magnitude > 1.0f) { magnitude = 1.0f; }
			printf("magnitude: %f\n", magnitude); // skorost
		}
		if (veh->m_vehicleType == 8)
			printf("m_bIsFreewheeling: %d 0x%p\n", *OFFSET(veh, 0x6E0, bool*), OFFSET(veh, 0x6E0, bool*));

		//CRGBA* c = OFFSET(veh, 0x224, CRGBA*);
		//int8_t m_nActiveColorVariation = *OFFSET(veh, 0x380, int8_t*);
		//int8_t m_nActiveScriptColorVariation = *OFFSET(veh, 0x381, int8_t*);
		//printf("def[%d %d %d %d], def %d scr %d\n", c[0].red, c[0].green, c[0].blue, c[0].alpha, m_nActiveColorVariation, m_nActiveScriptColorVariation);
		//printf("def[%d %d %d %d], def %d scr %d\n", c[1].red, c[1].green, c[1].blue, c[1].alpha, m_nActiveColorVariation, m_nActiveScriptColorVariation);
		////c[0] = { 0, 0, 0, 0 };
		////c[1] = { 0, 0, 0, 0 };
	}

	CPlayerPed* pp = FindPlayerPed();
	if (pp)
	{
		printf("%f 0x%p\n", *OFFSET(pp, 0xCB0, float*), OFFSET(pp, 0xCB0, float*));
		printf("%f 0x%p\n", *OFFSET(pp, 0xCB4, float*), OFFSET(pp, 0xCB4, float*));
		printf("m_fFallHeight %f \n\n", pp->CPed.m_fFallHeight);
		pp->CPed.m_fFallHeight = 11.0f;
		CRunningScript* scr = CTheScripts_ActiveScripts;
		printf("SCRIPT_NAME: %s IP: %d TIMERA %d\n", scr->m_abScriptName, scr->m_nIp, scr->m_anLocalVariables[TIMERA]);
	}



	// find needed flags to debug
	if(0)
	{
		for (int32_t i = CPools_ms_pPedPool->m_nSize - 1; i >= 0; i--)
		{
			if (CPools_GetSlotIsFree(CPools_ms_pPedPool, i)) { continue; }
			CPed* p = (CPed*)CPools_GetSlot(CPools_ms_pPedPool, i, 3360);
			if (p)
			{
				//printf("pool [%d] 0x%p\n", i, p);
				PrintPedsAnims(p);
				//if ((*(((uint8_t*)p) + 0x1CD)) & BIT(7)) // test 6bGetUpAnimStarted  7bFleeAfterExitingCar bit
				//if ((*(((uint8_t*)p) + 0x1D9)) & BIT(5)) // 4bDoBloodyFootprints 5
				//if(*OFFSET(p, 0x1CD, uint8_t*) & BIT(7))
				//if(*OFFSET(p, 0x1D8, uint8_t*) & BIT(3))
				//{
				//	printf("FIND!!!!!! 0x%p\n", p);
				//}
				//printf("FIND!!!!!! %f  0x%p\n", *OFFSET(p, 0x8B0, float*), OFFSET(p, 0x8B0, float*));
			}
		}
	}

	return;//----------------------------------------------------------




	if (!FindPlayerPed()) { return; }
	CEmpireBuildingInfo* pFirstInfo = EMUPOINTER<CEmpireBuildingInfo*>(EmpireMgr->m_pEmpiresInfosStart);
	for (size_t i = 0; i < 30; i++)
	{
		if (pFirstInfo)
		{
			CPhysical* pB = EMUPOINTER<CPhysical*>(pFirstInfo->m_pActualEmpireBuilding);
			if (pB)
			{
				CW_R();
				//printf("0x%p  %d\n", pFirstInfo, pFirstInfo->m_nBuildingState);
				//printf("%d ", pFirstInfo->m_nBuildingState);
				//printf("%d ", pFirstInfo->m_nScaleLevel);
				//printf("pB 0x%p \n", pB);
				//printf("%d \n", GET_BIT(pB->field_EF, 6));
				CW_G();
				break;
			}
		}
		pFirstInfo++;
	}


	bool neednewline = false;
	CVehicle* pVehicle = FindPlayerVehicle();
	int i = 0;
	//if (pVehicle && pVehicle->pPassengers[i])
	//{
	//	CPed* ped = EMUPOINTER<CPed*>(pVehicle->pPassengers[i]);
	//	//pVehicle->pPassengers[1]->m_fHealth = 0.0f;
	//	printf("pedstate: %d, obj: %d\n", ped->m_nPedState, ped->m_objective);
	//	printf("co: 0x%p, po: 0x%p\n", ped->m_carInObjective, ped->m_pedInObjective);
	//	//printf("mi %d\n", ped->CPhysical.CEntity.m_modelIndex);
	//	printf("te: 0x%p\n", ped->m_threatEntity);
	//	printf("pte: 0x%p\n", &ped->m_threatEntity);
	//	printf("leader: 0x%p\n", ped->m_leader);
	//	printf("tf: %d\n", *(uint32_t*)&ped->m_fearFlags1);
	//	return;
	//}

	if (pVehicle) { pVlast = pVehicle; }
	if (pVlast) { printf("health: %f\n", pVlast->m_fHealth); }

	//PrintCurrDir(); // pcsx2

	//for (int i = 0; i < 75; i++) // NUMRADARBLIPS
	//{
	//	CVectorVU_align16* ppos = (CVectorVU_align16*)((((char*)TheRadar) + 0x290) + (i * 48));
	//	//uint8_t type = *(uint8_t*)((((char*)TheRadar) + 0x2A9) + (i * 48));
	//	//uint8_t type = *(uint8_t*)(&(TheRadar->ms_RadarTrace[0].m_eRadarSprite) + (i * 48));
	//	if (ppos->x == 0.0f) { continue; }
	//	//DUMPVEC(*ppos);
	//	//printf("%d  ", type);
	//	printf("%d  ", TheRadar->ms_RadarTrace[i].m_eRadarSprite);
	//}
	//printf("TheRadar->field_1AE0 %f\n", TheRadar->field_1AE0);
	//printf("TheRadar->field_1AE4 %f\n", TheRadar->field_1AE4);
	//DUMPVEC(FindPlayerMenuTarget());

	//printf("desc 0x%p\n", EMUPOINTER<tSample*>(SampleManager->n_pSampleDesc_stuff));
	//printf("sfxgxt 0x%p\n", gAm_sfxgxt);
	//CVehicle* pV = FindPlayerVehicle();
	//if (pV)
	//{
	//	float magnitude = sqrtf(
	//		((pV->CPhysical.m_vecMoveSpeed.x * pV->CPhysical.m_vecMoveSpeed.x)
	//			+ (pV->CPhysical.m_vecMoveSpeed.y * pV->CPhysical.m_vecMoveSpeed.y))
	//		+ (pV->CPhysical.m_vecMoveSpeed.z * pV->CPhysical.m_vecMoveSpeed.z));
	//	if (magnitude > 1.0f) { magnitude = 1.0f; }
	//	printf("magnitude: %f\n", magnitude);
	//}
	//printf("player: 0x%p\n", FindPlayerPed());
	//printf("VPROP field_425: %d\n", FindPlayerVehicle() ? *(((char*)FindPlayerVehicle()) + 0x425) : null); // boat propelerinwater counter
	//printf("JUD field_450: %f\n", FindPlayerVehicle() ? *(float*)(((char*)FindPlayerVehicle()) + 0x450) : null); //
	printf("JLR field_454: %f\n", FindPlayerVehicle() ? *(float*)(((char*)FindPlayerVehicle()) + 0x454) : null); //
	//printf("A field_47C: %f\n", FindPlayerVehicle() ? *(float*)(((char*)FindPlayerVehicle()) + 0x47C) : null); //
	//printf("B field_480: %f\n", FindPlayerVehicle() ? *(float*)(((char*)FindPlayerVehicle()) + 0x480) : null); //
	//printf("LR field_484: %f\n", FindPlayerVehicle() ? *(float*)(((char*)FindPlayerVehicle()) + 0x484) : null); //
	//return;
	//printf("V forks: %d\n", FindPlayerVehicle() ? *(((char*)FindPlayerVehicle()) + 0x7F0) : null); // m_bIsMovingFrokliftForks 
	printf("CutsceneMgr: 0x%p\n", CutsceneMgr);
	printf("Cutscene name: %s\n", CutsceneMgr->ms_cutsceneName);
	printf("cWorldStream: 0x%p\n", cWorldStream);
	//printf("cWorldStream->pSwapData: 0x%p\n", PCSX2POINTER(*(uintptr_t*)(cWorldStream+0x1C)));
	//printf("cWorldStream->SwapEntries: 0x%p\n", PCSX2POINTER(*(uintptr_t*)(PCSX2POINTER(*(uintptr_t*)(cWorldStream+0x1C))+0x2B4)) );
	sLevelChunk* pWorldData = EMUPOINTER<sLevelChunk*>(cWorldStream->m_pWorldDataLevelChunk);
	CGroupedBuilding* m_pSwapBuildingGroups = EMUPOINTER<CGroupedBuilding*>(pWorldData ? pWorldData->swapInfos : null);
	printf("cWorldStream->pWorldData: 0x%p\n", pWorldData);
	printf("cWorldStream->m_pSwapBuildingGroups: 0x%p\n", m_pSwapBuildingGroups);
	CRunningScript* scr = CTheScripts_ActiveScripts;
	int16_t op = *((int16_t*)SCRBYTEVAR(scr->m_nIp)) & 0x7FFF;
	printf("OP[%d](%s): ip: %d %s\n", op, scr->m_abScriptName, scr->m_nIp, op < coms.size() ? coms[op].c_str() : "");
	printf("MusicManager: 0x%p\n", MusicManager);
	printf("MusicManager m_aTracks[0]: 0x%p\n", &(MusicManager->m_aTracks[0]));
	printf("MusicManager m_nMusicMode: %d\n", MusicManager->m_nMusicMode);
	//printf("MusicManager _field_608: %d\n", MusicManager->musicstreamer_filepos_sub189150_field_608); // 0?
	printf("MusicManager 60c: %d\n", MusicManager->_189198_field_60C); // 0?
	//for (int i = 0; i < 15; i++) // streamed 113
	//{
	//	printf("%s [%d] pos %d / len %d\n", MusicManager->m_nPlayingTrack == i ? "-> " : "", i, MusicManager->m_aTracks[i].m_nPosition, MusicManager->m_aTracks[i].m_nLength);
	//	MusicManager->m_aTracks[i].m_nLastPosCheckTimer = 0;
	//}

	DUMPVEC(pos);
	printf("CGame::currArea: %d\n", CGame_currArea);
	printf("CGame::currLevel: %d\n", CGame_currLevel);
	//printf("TheCamera.ActiveCam: %hhu\n", *OFFSET(TheCamera, 0x50, uint8_t*));
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
		//CPtrList list = s->empire; // type 5 empire
		CPtrList list = s->empireover; // type 5? empire over
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
				short mi = pEntity->m_modelIndex;
				if (mi != 1716) { first = EMUPOINTER<CPtrNode*>(first->next); continue; }
				printf("modelindex: %hu\n", mi); // empire 0  mi 7436 type 5  status 4
				//printf("entity enex id: %d\n", mi ? mi->e_enex : -1);
				printf("entity mi: 0x%p\n", GetModelInfo(mi));
				printf("entity type: %d\n", GetEntityType(pEntity)); // (empires) 7508, 7436, 7520, 7443, 7436
				printf("entity status: %d\n", GetEntityStatus(pEntity));
				DUMPVEC(pEntity->CPlaceable.m_pMat.pos);

				// RW RSL test
				RwObject* rwo = EMUPOINTER<RwObject*>(pEntity->m_urwObject.m_rwObject);
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
							if (prp && prp->unk2) { printf("----------------------------\n0x%p\n\n", (void*)prp->unk2); }
						}
						//FORLIST(lnk, clump->atomics)
						//{
						//}

						break;
					}
					case rpCLUMP:
					{
						RpClump* rwc = (RpClump*)rwo;
						printf("CLUMP!!!!!!!!!!: 0x%p\n", rwc);

						break;
					}
					default:
					{

						break;
					}
					}

				}

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

int32_t DetectVersion()
{
	bool shift = (GetAsyncKeyState(VK_SHIFT) & 0x8000);

	//printf("%d\n", *EMUPOINTER<int*>(0x2EAAF8));
	//if (*EMUPOINTER<int*>(0x2EAAF8) == 0x24020001)
	return shift; // vcs
	//return 1; // lcs
	return 0; // vcs
}

//int __fastcall sub_7DC460(void* ecx, void* edx) { int res = ((int(__thiscall*)(void*))IDA2PCSX2160(0x7DC460))(ecx); OnLoadState(); return res; }
int ver = 0;
void PluginInit()
{
	setlocale(LC_NUMERIC, "C");
	InitConsole();
	ver = DetectVersion();
	std::string szver = "";
	if (ver == 0)
	{
		szver = "vcs";
		ModelInfoExt::Init("Plugins\\_VCS_\\models.txt");
		coms = FileReadAllLines("Plugins\\_VCS_\\coms.txt");
		printf("[COMS] %d\n", coms.size());
		InitPatches();
		//PatchTest();
		//patch<uint32_t>(IDA2PCSX2160(0x47A804 + 1), CalcOffset(IDA2PCSX2160(0x47A804), (uintptr_t)sub_7DC460)); // 1.6.0
	}
	else if (ver == 1)
	{
		szver = "lcs";

	}
	printf("MaZaHaKa PCSX2 Plugin Initialized!! (%s)\n", szver.c_str());
}

bool gbKeyHold = false;
struct CVectorLCS { float x, y, z; };
struct sLevelSwapLCS
{
	char timeOff;
	char timeOn;
	__int16 id;
};

struct sLevelChunkLCS
{
	void* resourceTable;
	char sectorRows[376];
	int numResources;
	CVectorLCS positions[32];
	int numLevelSwaps;
	sLevelSwapLCS* levelSwaps;
	int numDynamics;
	void* dynamics;
	int numInteriors;
	void* interiors;
	int numRadarSections;
	void* radarSections;
};
struct __declspec(align(1)) cWorldStreamLCS
{
	INSTANCE field_0;
	int field_C;
	int m_nLevelid;
	sLevelChunkLCS* m_pWorldDataLevelChunk;
	char field_18[44];
	int field_44;
	int field_48;
	char field_4C[40];
	int field_74;
	char field_78[180];
	int field_12C;
	char field_130[228];
	int m_aSwapStates[20];
	char field_264[777];
};
#define CWorld_PlayersLCS ((void*)IDATRANSLATE(0x408E40))
#define CWorld_PlayerInFocusLCS ((*(uint8_t*)IDATRANSLATE(0x3D9BC8)))
#define cWorldStreamLcs ((cWorldStreamLCS*)PCSX2POINTER(*(uintptr_t**)IDATRANSLATE(0x3CEF78)))
CPlayerPed* FindPlayerPedLCS() { return EMUPOINTER<CPlayerPed*>(((CPlayerInfo*)CWorld_PlayersLCS)[CWorld_PlayerInFocusLCS].m_pPed); }
void PluginLoopNew()
{


	bool r = (GetAsyncKeyState('R') & 0x8000);
	bool g = (GetAsyncKeyState('G') & 0x8000);
	bool b = (GetAsyncKeyState('B') & 0x8000);
	bool p = (GetAsyncKeyState('P') & 0x8000);
	bool o = (GetAsyncKeyState('O') & 0x8000);
	bool d = (GetAsyncKeyState('D') & 0x8000);
	bool t = (GetAsyncKeyState('T') & 0x8000);
	bool i = (GetAsyncKeyState('I') & 0x8000);
	static bool upd = false;

	if (r && !gbKeyHold)
	{
		{
			sLevelChunkLCS* lvl = EMUPOINTER<sLevelChunkLCS*>(cWorldStreamLcs->m_pWorldDataLevelChunk);
			memset(EMUPOINTER<char*>(lvl->levelSwaps), 0, sizeof(sLevelSwapLCS) * lvl->numLevelSwaps);
			lvl->numLevelSwaps = 0;
			lvl->numDynamics = 0;
			lvl->numInteriors = 0; // ломает интеры
			lvl->numRadarSections = 0;
			///lvl->numLevelSwaps = 0;
			cWorldStreamLcs->m_aSwapStates[12] = 1;
			for (size_t i = 0; i < 33; i++)
				cWorldStreamLcs->m_aSwapStates[i] = 1;
		}

		gbKeyHold = true;
	}
	else if (t && !gbKeyHold)
	{
		CVector pos = { 273.065f, -314.644f, 33.313f*1.2 };
		//pos = { 295.441f, -308.178f, 32.768f*1.2 };

		pos = { 294.150f, -307.459f, 21.082f*1.2 };
		//pos = { 265.483f, -295.163f, 22.655f*1.2 }; // правый край центра
		pos = { 251.483f, -295.163f, 22.655f*1.2 }; // прям центр дороги
		SetCVector4VU(&FindPlayerPedLCS()->CPed.CPhysical.CEntity.CPlaceable.m_pMat.pos, &pos);
	}
	else if (g && !gbKeyHold)
	{
		{
			// X40 Y46
			sLevelChunkLCS* lvl = EMUPOINTER<sLevelChunkLCS*>(cWorldStreamLcs->m_pWorldDataLevelChunk);
			uint32_t* sectors = (uint32_t*)lvl->sectorRows; // pChunk+offset t(8b)
			uint32_t* sector0chunk = EMUPOINTER<uint32_t*>(sectors[0 * 2]); // Y0 X0
			uint32_t* sector1chunk = EMUPOINTER<uint32_t*>(sectors[1 * 2]); // Y0 X1
			//uint32_t* sector46chunk = EMUPOINTER<uint32_t*>(sectors[46 * 2]); // Y1 X0
			printf("cWorldStreamLcs 0x%p\n", cWorldStreamLcs);
			printf("LevelChunk 0x%p\n", lvl);
			printf("LevelChunkSectors 0x%p\n", ((char*)lvl)+4);
			printf("test 0x%p\n", lvl+4);
			printf("sizeof(sChunkHeader) 0x20\n"); // 32
			printf("sectorchunk[Y0X0] 0x%p\n", sector0chunk);
			printf("sectorchunk[Y0X39] 0x%p\n", sector0chunk + (32 * 39));
			printf("sectorchunk[Y0X40] 0x%p\n", sector0chunk + (32 * 40)); // same y1x0
			printf("sectorchunk[Y1X0] 0x%p\n", sector1chunk); // 0 + 40header
			//printf("sectorchunk[Y46] 0x%p\n", sector46chunk);
			printf("numLevelSwaps %d\n", lvl->numLevelSwaps);
			printf("numInteriors %d\n", lvl->numInteriors);

			printf("\n");
		}

		gbKeyHold = true;
	}
	else if (i && !gbKeyHold)
	{
		upd ^= 1;
		gbKeyHold = true;
	}
	else if (!r && !t && !g && !i) { gbKeyHold = false; }

	if (upd)
	{
		CVuVector p = FindPlayerPedLCS()->CPed.CPhysical.CEntity.CPlaceable.m_pMat.pos;
		printf("%f %f %f\n", p.x, p.y, p.z);
	}

}

void PluginLoop()
{
	if (!WaitElf(true)) { return; }

	if (ver)
	{
		PluginLoopNew();
		return;
	}


	bool enter = (GetAsyncKeyState(VK_RETURN) & 0x8000);
	if (enter) { return; } // f3 load hold fast anticrash (todo pcsx2 load flag)
	bool r = (GetAsyncKeyState('R') & 0x8000);
	bool g = (GetAsyncKeyState('G') & 0x8000);
	bool b = (GetAsyncKeyState('B') & 0x8000);
	bool p = (GetAsyncKeyState('P') & 0x8000);
	bool o = (GetAsyncKeyState('O') & 0x8000);
	bool d = (GetAsyncKeyState('D') & 0x8000);
	bool t = (GetAsyncKeyState('T') & 0x8000);
	bool i = (GetAsyncKeyState('I') & 0x8000);
	//if (i) { quit = true; return; }
	bool shift = (GetAsyncKeyState(VK_SHIFT) & 0x8000);
	//bool f7 = (GetAsyncKeyState(VK_F7) & 0x8000); // f not work
	//bool f8 = (GetAsyncKeyState(VK_F8) & 0x8000); // screenshot
	if (r && !gbKeyHold)
	{
		bool allowhold = OnKey(0); // glass
		if (!allowhold) { gbKeyHold = true; }
	}
	else if (g && !gbKeyHold)
	{
		bool allowhold = OnKey(1);
		if (!allowhold) { gbKeyHold = true; }
	}
	else if (b && !gbKeyHold)
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
		bool allowhold = OnKey(4);
		if (!allowhold) { gbKeyHold = true; }
	}
	else if (o && !gbKeyHold)
	{
		PatchTest();
		gbKeyHold = true;
	}
	else if (!r && !g && !b && !o && !d) { gbKeyHold = false; }

	TeleporterTester();
	if (tmp3) { EmpireTest(1); }
	UpdNonSyncStuff();
	//if(CTheScripts_MainScriptSize) { patch<uint32_t>(SCRVAR(5547), 0); } // PHI_A2 float
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

	if (t /*&& shift*/) // teleport
	{
		CVector pos;
		//printf("Enter tp (x y z): ");
		////std::cin.ignore();
		//scanf("%f %f %f", &pos.x, &pos.y, &pos.z);
		//TeleportPlayer(pos);
		pos = FindPlayerMenuTarget();
		if (pos.x != 0.0f && pos.y != 0.0f) { TeleportPlayer(pos); }
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

DWORD CALLBACK ThreadEntry(LPVOID)
{
	WaitElf();
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