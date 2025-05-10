#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdint>
#include <algorithm>
#include <iomanip>
#include <unordered_map>
#include <windows.h>
#include <shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")

#define Clamp(v, low, high) ((v) < (low) ? (low) : (v) > (high) ? (high) : (v))

static uint32_t read_be32(const uint8_t* p) {
    return (uint32_t(p[0]) << 24) | (uint32_t(p[1]) << 16) | (uint32_t(p[2]) << 8) | uint32_t(p[3]);
}

#define VAG_SAMPLES_PER_BLOCK 28 // Samples per VAG ADPCM block
#define VAG_BLOCK_SIZE      16 // ADPCM block size in bytes

#pragma pack(push,1)
struct tSamplePS2
{
    uint32_t nOffset;
    uint32_t nSize;
    uint32_t nFrequency;
};
static_assert(sizeof(tSamplePS2) == 12, "tSamplePS2 must be 12 bytes");

struct tSamplePC
{
    uint32_t nOffset;
    uint32_t nSize;
    uint32_t nFrequency;
    uint32_t nLoopStart;
    int32_t nLoopEnd;
};
static_assert(sizeof(tSamplePC) == 20, "tSamplePC must be 20 bytes");

struct VAGheader {
    uint32_t magic;         /* always 'VAGp' for identifying*/
    uint32_t ver;           /* format version (2) */
    uint32_t ssa;           /* Source Start Address, always 0 (reserved for VAB format) */
    uint32_t dataSize;      /* Sound Data Size in byte */
    uint32_t sampleRate;    /* sampling frequency, 44100(>pt1000), 32000(>pt), 22000(>pt0800)... */
    uint16_t volL;          /* base volume for Left channel */
    uint16_t volR;          /* base volume for Right channel */
    uint16_t pitch;         /* base pitch (includes fs modulation)*/
    uint16_t ADSR1;         /* base ADSR1 (see SPU manual) */
    uint16_t ADSR2;         /* base ADSR2 (see SPU manual) */
    uint16_t reserved;      /* not in use */
    char     name[16];      
};
static_assert(sizeof(VAGheader) == 48, "VAGheader must be 48 bytes");
#pragma pack(pop)

// ADPCM koef
static const int16_t coefs[5][2] = {
    {    0,   0 },
    {   60,   0 },
    {  115, -52 },
    {   98, -55 },
    {  122, -60 }
};

void decodeVAG(const uint8_t* adpcmData, uint32_t dataSize, std::vector<int16_t>& pcm,
    int& loopStartSample, int& loopEndSample)
{
    int16_t hist1 = 0, hist2 = 0;
    loopStartSample = -1;
    loopEndSample = -1;
    size_t blocks = dataSize / VAG_BLOCK_SIZE;
    int samplePos = 0;

    for (size_t b = 0; b < blocks; ++b) {
        const uint8_t* blk = adpcmData + b * VAG_BLOCK_SIZE;
        uint8_t shift = blk[0] & 0x0F;
        uint8_t filt = (blk[0] & 0xF0) >> 4;
        uint8_t flag = blk[1];

        if (flag == 0x06) {
            //loopStartSample = samplePos;
            int32_t ls = samplePos - VAG_SAMPLES_PER_BLOCK;
            loopStartSample = (ls > 0 ? ls : 0);
        }
        if (flag == 0x03) {
            // по умолчанию — не включаем текущий блок в зону повторения
            loopEndSample = samplePos;
            // если же нужно, чтобы этот блок проигрывался в конце петли,
            //loopEndSample = samplePos + VAG_SAMPLES_PER_BLOCK;
            break;
        }
        if (flag == 0x07) {
            // EOF
            break;
        }

        // 14 bytes 28 samples
        int16_t c1 = coefs[filt][0], c2 = coefs[filt][1];
        for (int i = 0; i < 14; ++i) {
            uint8_t byte = blk[2 + i];
            for (int half = 0; half < 2; ++half) {
                int32_t s = (half == 0 ? (byte >> 4) : (byte & 0x0F));
                if (s >= 8) s -= 16;
                s = (s << shift) + ((c1 * hist1 + c2 * hist2 + 32) >> 6);
                s = Clamp(s, -32768, 32767);
                pcm.push_back(int16_t(s));
                hist2 = hist1; hist1 = int16_t(s);
                ++samplePos;
            }
        }
    }
}

//#define PS2_SFX_IDS
#define BANK_NAME

int main()
{
    char exePath[MAX_PATH] = {};
    if (GetModuleFileNameA(NULL, exePath, MAX_PATH))
    {
        PathRemoveFileSpecA(exePath);
        SetCurrentDirectoryA(exePath);
    }

    std::vector<std::string> ps2Banks = 
    { 
        // SET0 (0-9)
        "audio\\SET0\\SFX0_PS",
        "audio\\SET0\\SFX1_PS",
#ifdef PS2_SFX_IDS
        "audio\\SET0\\SFX2_PS", // PED
#endif
        "audio\\SET0\\SFX3_PS", // FE
        "audio\\SET0\\SFX4_PS",
        "audio\\SET0\\SFX5_PS",
        "audio\\SET0\\SFX6_PS",
        "audio\\SET0\\SFX7_PS",
        "audio\\SET0\\SFX8_PS",
        "audio\\SET0\\SFX9_PS",

        // SET1 (10-19)
        "audio\\SET1\\SFX10_PS",
        "audio\\SET1\\SFX11_PS",
        "audio\\SET1\\SFX12_PS",
        "audio\\SET1\\SFX13_PS",
        "audio\\SET1\\SFX14_PS",
        "audio\\SET1\\SFX15_PS",
        "audio\\SET1\\SFX16_PS",
        "audio\\SET1\\SFX17_PS",
        "audio\\SET1\\SFX18_PS",
        "audio\\SET1\\SFX19_PS",

        // SET2 (20-29)
        "audio\\SET2\\SFX20_PS",
        "audio\\SET2\\SFX21_PS",
        "audio\\SET2\\SFX22_PS",
        "audio\\SET2\\SFX23_PS",
        "audio\\SET2\\SFX24_PS",
        "audio\\SET2\\SFX25_PS",
        "audio\\SET2\\SFX26_PS",
        "audio\\SET2\\SFX27_PS",
        "audio\\SET2\\SFX28_PS",
        "audio\\SET2\\SFX29_PS",

        // SET3 (30-39)
        "audio\\SET3\\SFX30_PS",
        "audio\\SET3\\SFX31_PS",
        "audio\\SET3\\SFX32_PS",
        "audio\\SET3\\SFX33_PS",
        "audio\\SET3\\SFX34_PS",
        "audio\\SET3\\SFX35_PS",
        "audio\\SET3\\SFX36_PS",
        "audio\\SET3\\SFX37_PS",
        "audio\\SET3\\SFX38_PS",
        "audio\\SET3\\SFX39_PS",

        // SET4 (40-49)
        "audio\\SET4\\SFX40_PS",
        "audio\\SET4\\SFX41_PS",
        "audio\\SET4\\SFX42_PS",
        "audio\\SET4\\SFX43_PS",
        "audio\\SET4\\SFX44_PS",
        "audio\\SET4\\SFX45_PS",
        "audio\\SET4\\SFX46_PS",
        "audio\\SET4\\SFX47_PS",
        "audio\\SET4\\SFX48_PS",
        "audio\\SET4\\SFX49_PS",

        // SET5 (50-59)
        "audio\\SET5\\SFX50_PS",
        "audio\\SET5\\SFX51_PS",
        "audio\\SET5\\SFX52_PS",
        "audio\\SET5\\SFX53_PS",
        "audio\\SET5\\SFX54_PS",
        "audio\\SET5\\SFX55_PS",
        "audio\\SET5\\SFX56_PS",
        "audio\\SET5\\SFX57_PS",
        "audio\\SET5\\SFX58_PS",
        "audio\\SET5\\SFX59_PS",

        // SET6 (60-69)
        "audio\\SET6\\SFX60_PS",
        "audio\\SET6\\SFX61_PS",
        "audio\\SET6\\SFX62_PS",
        "audio\\SET6\\SFX63_PS",
        "audio\\SET6\\SFX64_PS",
        "audio\\SET6\\SFX65_PS",
        "audio\\SET6\\SFX66_PS",
#ifndef PS2_SFX_IDS
        "audio\\SET0\\SFX2_PS", // PED
#endif
        "audio\\SET6\\SFX67_PS", // MISSION
    };
    std::string pcSdtFileName = "sfx";


    std::vector<tSamplePS2> combinedTable;
    std::vector<std::string> combinedBankNames;
    std::vector<uint8_t>    combinedRawData;
    uint32_t rawBaseOffset = 0;

    for (const auto& bank : ps2Banks)
    {
        auto banknamepos = bank.find_last_of("\\/");
        std::string bankName = (banknamepos != std::string::npos) ? bank.substr(banknamepos + 1) : bank;

        std::ifstream ifsSdt(bank + ".sdt", std::ios::binary);
        if (!ifsSdt) { std::cerr << "Cannot open " << bank << ".sdt\n"; return 1; }
        size_t cnt = ifsSdt.seekg(0, std::ios::end).tellg() / sizeof(tSamplePS2);
        ifsSdt.seekg(0, std::ios::beg);

        std::vector<tSamplePS2> tbl(cnt);
        ifsSdt.read(reinterpret_cast<char*>(tbl.data()), cnt * sizeof(tSamplePS2));

        for (auto& e : tbl) {
            e.nOffset += rawBaseOffset;
            combinedTable.push_back(e);
            combinedBankNames.push_back(bankName);
        }

        std::ifstream ifsRaw(bank + ".raw", std::ios::binary);
        if (!ifsRaw) { std::cerr << "Cannot open " << bank << ".raw\n"; return 1; }
        auto pos = ifsRaw.seekg(0, std::ios::end).tellg();
        ifsRaw.seekg(0, std::ios::beg);
        size_t sz = size_t(pos);

        size_t prevSize = combinedRawData.size();
        combinedRawData.resize(prevSize + sz);
        ifsRaw.read(reinterpret_cast<char*>(combinedRawData.data() + prevSize), sz);

        rawBaseOffset += uint32_t(sz);
    }

    // calc loops
#ifdef BANK_NAME
    std::cout << "Idx  Bank              Name             Length   Freq    LoopStart   LoopEnd\n";
    std::cout << "---------------------------------------------------------------------------\n";
#else
    std::cout << "Idx  Name             Length   Freq   LoopStart   LoopEnd\n";
    std::cout << "----------------------------------------------------------\n";
#endif
    std::vector<std::pair<uint32_t, int32_t>> loopPoints;
    loopPoints.reserve(combinedTable.size());

    for (size_t i = 0; i < combinedTable.size(); ++i) {
        const auto& e = combinedTable[i];
        const auto& bankName = combinedBankNames[i];
        if (e.nOffset + sizeof(VAGheader) > combinedRawData.size()) {
            std::cerr << "Entry " << i << ": out of range\n";
            loopPoints.emplace_back(0, 0);
            continue;
        }
        const uint8_t* ptr = combinedRawData.data() + e.nOffset;
        if (read_be32(ptr) != 'VAGp') { // 0x56414770
            std::cerr << "Entry " << i << ": not VAGp\n";
            loopPoints.emplace_back(0, 0);
            continue;
        }
        uint32_t adpcmSize = read_be32(ptr + 0x0C);
        uint32_t sampleRate = read_be32(ptr + 0x10);
        const VAGheader* vh = reinterpret_cast<const VAGheader*>(ptr);
        std::string name(vh->name, vh->name + 16);
        name.erase(std::find(name.begin(), name.end(), '\0'), name.end());

        std::vector<int16_t> pcm;
        int ls = -1, le = -1;
        decodeVAG(ptr + sizeof(VAGheader), adpcmSize, pcm, ls, le);

        if (ls >= 0 && le < 0) le = int(pcm.size());
        uint32_t bytesTotal = uint32_t(pcm.size() * sizeof(int16_t));
        uint32_t bytesLS = ls >= 0 ? uint32_t(ls * sizeof(int16_t)) : 0;
        int32_t  bytesLE = le >= 0 ? int32_t(le * sizeof(int16_t)) : -1;

        loopPoints.emplace_back(bytesLS, bytesLE);

#ifdef BANK_NAME
        std::cout << std::setw(3) << i << "  "
            << std::left << std::setw(16) << bankName << " "
            << std::setw(16) << name << " "
            << std::right << std::setw(7) << bytesTotal << " "
            << std::setw(7) << sampleRate << " "
            << std::setw(10) << bytesLS << " "
            << std::setw(8) << bytesLE << "\n";
#else
        std::cout << std::setw(3) << i << "  "
            << std::left << std::setw(16) << name << " "
            << std::right << std::setw(7) << bytesTotal << " "
            << std::setw(7) << sampleRate << " "
            << std::setw(10) << bytesLS << " "
            << std::setw(8) << bytesLE << "\n";
#endif
    }
    //return 0;

    // upd pc sdt stuff
    std::ifstream ifsPc(pcSdtFileName + ".sdt", std::ios::binary);
    if (!ifsPc) { std::cerr << "Cannot open " << pcSdtFileName << ".sdt\n"; return 1; }
    size_t pcCount = ifsPc.seekg(0, std::ios::end).tellg() / sizeof(tSamplePC);
    ifsPc.seekg(0, std::ios::beg);
    std::vector<tSamplePC> pcTable(pcCount);
    ifsPc.read(reinterpret_cast<char*>(pcTable.data()), pcCount * sizeof(tSamplePC));
    ifsPc.close();

    size_t n = min(pcTable.size(), loopPoints.size());
    for (size_t i = 0; i < n; ++i) {
        pcTable[i].nLoopStart = loopPoints[i].first;
        pcTable[i].nLoopEnd = loopPoints[i].second;
    }

    std::ofstream ofsPc(pcSdtFileName + ".sdt", std::ios::binary | std::ios::trunc);
    ofsPc.write(reinterpret_cast<const char*>(pcTable.data()), pcTable.size() * sizeof(tSamplePC));
    ofsPc.close();

    std::cout << "PC .sdt updated: " << pcSdtFileName << ".sdt (" << n << " entries updated)\n";
    return 0;
}

// https://www.psxdev.net/forum/viewtopic.php?t=841
//Detection of loop points in VAGs that continuously plays(aka sampler loop)
//Using the code from bitMaster floating around on the web,
//watch for flag no. 6, right before where it watches for no. 7 (EOF)
//then
//(value - 56) / 2 // value is the current position in bytes of the output stream being generated
//will be the sample value where the loop begins !
//Basically we can now extract VAGs with their loop points preserved !!!
//Note 1 : one should early exit the function by watching input stream position, instead of relying on flag no.
//7 (many times this don't seem to be sufficient).
//Note 2 : the end point of a loop region will be always at EOF, i.e.everything beyond is discarded, i.e.
//if your input WAV was 5 sec longand loop was between 1sec and 2sec, the output VAG will be 2 sec.
//(this is for people converting WAV / AIFF to VAG, wondering why they suddenly end being shorter than the source)
//Btw it also seems to work for lower - level format.VH / .VB combo.
