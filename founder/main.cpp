#define NOMINMAX
#include "Windows.h" 
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdlib>    // для system("cls")
#include <clocale>    // для setlocale
#include <algorithm>  // для std::transform
#include <limits>     // для std::numeric_limits
#include <regex>
#include <sstream>
#include <iomanip>
#include <unordered_map>
#include <unordered_set>

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

void U_SetCurrentDirectory()
{
    char currentDir[MAX_PATH];
    GetModuleFileNameA(NULL, currentDir, MAX_PATH);
    std::string::size_type pos = std::string(currentDir).find_last_of("\\/");
    SetCurrentDirectoryA(std::string(currentDir).substr(0, pos).c_str());
}

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

#define CW_R() SetColor(FOREGROUND_RED)                       
#define CW_G() SetColor(FOREGROUND_GREEN)                     
#define CW_B() SetColor(FOREGROUND_BLUE)                      
#define CW_Y() SetColor(FOREGROUND_RED | FOREGROUND_GREEN)    
#define CW_C() SetColor(FOREGROUND_GREEN | FOREGROUND_BLUE)   
#define CW_M() SetColor(FOREGROUND_RED | FOREGROUND_BLUE)     
#define CW_W() SetColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE)
#define CW_K() SetColor(0)       
#define CW_DEF() CW_G()

// orig

//std::string Trim(std::string str)
//{
//    // Find the first non-whitespace character from the beginning.
//    size_t start = str.find_first_not_of(" \t\n\r\f\v");
//
//    if (start == std::string::npos) {
//        // If the string consists only of whitespace, return an empty string.
//        return "";
//    }
//
//    // Find the last non-whitespace character from the end.
//    size_t end = str.find_last_not_of(" \t\n\r\f\v");
//
//    // Calculate the length of the trimmed substring.
//    size_t length = end - start + 1;
//
//    // Extract and return the trimmed substring.
//    return str.substr(start, length);
//}
//
//std::string RemoveDouble32(std::string str, bool is_trim)
//{
//    bool is_find = false;
//
//    // удаление дублирующихся пробелов
//    for (size_t i = 0; i < str.length(); ++i) {
//        if (is_find && str[i] == ' ') {
//            str.erase(i, 1);
//            --i; // смещаем индекс, чтобы не пропустить символ после удаления
//            continue;
//        }
//        is_find = (str[i] == ' ');
//    }
//
//    // удаление пробелов в начале и конце строки, если is_trim установлен в true
//    if (is_trim) {
//        //str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](unsigned char ch) { return !std::isspace(ch); }));
//        //str.erase(std::find_if(str.rbegin(), str.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), str.end());
//        str = Trim(str);
//    }
//
//    return str;
//}

std::string GetBaseName(const std::string& filename) {
    // Убираем путь
    size_t slash = filename.find_last_of("/\\");
    std::string name = (slash != std::string::npos ? filename.substr(slash + 1) : filename);
    // Убираем расширение
    size_t dot = name.find_last_of('.');
    return (dot != std::string::npos ? name.substr(0, dot) : name);
}

std::string Trim(std::string str) {
    const char* ws = " \t\n\r\f\v";
    size_t s = str.find_first_not_of(ws);
    if (s == std::string::npos) return "";
    size_t e = str.find_last_not_of(ws);
    return str.substr(s, e - s + 1);
}

std::string RemoveDoubleSpacesAndTabs(std::string str, bool trim) {
    bool lastSpace = false;
    for (size_t i = 0; i < str.size(); ++i) {
        if (lastSpace && (str[i] == ' ' || str[i] == '\t')) {
            str.erase(i, 1); --i; continue;
        }
        lastSpace = (str[i] == ' ' || str[i] == '\t');
    }
    if (trim) str = Trim(str);
    return str;
}


void _oldFoundSubstringRaws()
{
    // 1. Запрос имени входного файла
    std::string input_filename;
    std::cout << "Введите имя входного файла (с расширением или без): ";
    std::getline(std::cin, input_filename);
    if (input_filename.find('.') == std::string::npos) {
        input_filename += ".txt";
    }

    // 2. Открываем файл в бинарном режиме, чтобы получить размер
    std::ifstream fin_bin(input_filename, std::ios::binary | std::ios::ate);
    if (!fin_bin) {
        std::cerr << "Не удалось открыть файл \"" << input_filename << "\" для чтения.\n";
        return;
    }
    std::streampos size = fin_bin.tellg();
    fin_bin.close();

    // 3. Читаем файл построчно и сохраняем строки в вектор
    std::ifstream fin(input_filename);
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(fin, line)) {
        lines.push_back(line);
    }
    fin.close();

    // 4. Выводим размер и количество строк
    std::cout << "Размер файла: " << size << " байт\n";
    std::cout << "Количество строк: " << lines.size() << "\n\n";

    // 5. Запрос подстроки для поиска
    std::string pattern;
    std::cout << "Введите подстроку для поиска: ";
    std::getline(std::cin, pattern);

    //// 6. Запрос учитывать ли регистр
    //std::cout << "Игнорировать регистр? (y/n): ";
    //char case_choice;
    //std::cin >> case_choice;
    //std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    //bool lowercase = (case_choice == 'y' || case_choice == 'Y');
    //// если нужно игнорировать регистр — приводим шаблон к нижнему

    bool lowercase = true; // hardcode

    std::string pattern_low;
    if (lowercase) {
        pattern_low = pattern;
        std::transform(pattern_low.begin(), pattern_low.end(), pattern_low.begin(), ::tolower);
    }

    //system("cls");

    //// 7. Ищем и выводим строки, содержащие подстроку
    //std::vector<std::string> found;
    //int i = 1;
    //for (const auto& l : lines) {
    //    if (l.find(pattern) != std::string::npos) {
    //        printf("%d. %s\n", i++, l.c_str());
    //        found.push_back(l);
    //    }
    //}
    //if (found.empty()) {
    //    std::cout << "Совпадений не найдено.\n";
    //}

    // 7. Ищем совпадения: сохраняем (номер строки, текст)
    std::vector<std::pair<int, std::string>> found;
    for (int idx = 0; idx < (int)lines.size(); ++idx) {
        const auto& l = lines[idx];
        bool match = false;
        if (lowercase) {
            std::string tmp = l;
            std::transform(tmp.begin(), tmp.end(), tmp.begin(), ::tolower);
            if (tmp.find(pattern_low) != std::string::npos) match = true;
        }
        else {
            if (l.find(pattern) != std::string::npos) match = true;
        }
        if (match) {
            found.emplace_back(idx + 1, l);
        }
    }

    // 8. Вывод в консоль
    if (found.empty()) {
        std::cout << "Совпадений не найдено.\n";
    }
    else {
        int cnt = 1;
        for (auto& p : found) {
            // формат: 1 [147555]    строка
            std::cout << cnt << " [" << p.first << "]\t" << p.second << "\n";
            ++cnt;
        }
    }

    // 9. Запрос имени выходного файла
    std::cout << "\nВведите имя выходного файла для сохранения результатов: ";
    std::string output_filename;
    std::getline(std::cin, output_filename);
    // Если пользователь не указал расширение, добавляем .txt по умолчанию
    if (output_filename.find('.') == std::string::npos) {
        output_filename += ".txt";
    }

    // 10. Если файл существует, уведомляем пользователя о перезаписи
    std::ifstream test(output_filename);
    if (test) {
        std::cout << "Файл \"" << output_filename << "\" существует и будет перезаписан.\n";
        test.close();
    }

    // 11. Открываем файл на запись (с перезаписью) и сохраняем найденные строки
    std::ofstream fout(output_filename, std::ios::trunc);
    if (!fout) {
        std::cerr << "Не удалось открыть файл \"" << output_filename << "\" для записи.\n";
        return;
    }
    //for (const auto& l : found) {
    //    fout << l << "\n";
    //}
    for (auto& p : found) {
        fout << p.first << ".\t" << p.second << "\n";
    }
    fout.close();

    std::cout << "Результаты сохранены в файл \"" << output_filename << "\".\n";
}


struct FuncNodeDB { uint32_t ptr; std::string name; };
std::vector<FuncNodeDB> ParseIdaFuncs(std::string path)
{
    std::vector<FuncNodeDB> res;

    std::ifstream fin(path);
    if (!fin.is_open()) {
        std::cerr << "Не удалось открыть файл: " << path << "\n";
        return res;
    }

    std::string line;
    bool headerDetected = false;
    //std::string currentPtr;
    uint32_t currentFuncPtr = 0;

    while (std::getline(fin, line)) {
        std::string trimmed = RemoveDoubleSpacesAndTabs(line, true);

        if (trimmed.rfind("//----- (", 0) == 0) {
            size_t p1 = trimmed.find('(');
            size_t p2 = trimmed.find(')');
            if (p1 != std::string::npos && p2 != std::string::npos && p2 > p1 + 1) {
                //currentPtr = trimmed.substr(p1 + 1, p2 - p1 - 1);
                std::string ptrStr = trimmed.substr(p1 + 1, p2 - p1 - 1);
                std::stringstream ss;
                ss << std::hex << ptrStr;
                ss >> currentFuncPtr;
                headerDetected = true;
            }
            continue;
        }

        //if (headerDetected && !trimmed.empty())
        if (headerDetected && !trimmed.empty() && (trimmed.rfind("//", 0) != 0)) // // positive sp value comms fix
        {
            headerDetected = false;
            std::string currentFuncName;
            size_t posParen = trimmed.find('(');
            if (posParen != std::string::npos) {
                size_t posSpace = trimmed.rfind(' ', posParen);
                currentFuncName = (posSpace != std::string::npos)
                    ? trimmed.substr(posSpace + 1, posParen - posSpace - 1)
                    : trimmed.substr(0, posParen);
                size_t atPos = currentFuncName.find('@');
                if (atPos != std::string::npos) currentFuncName.erase(atPos);
            }
            else currentFuncName.clear();
            res.push_back({ currentFuncPtr, currentFuncName });
        }
    }

    fin.close();
    return res;
}

void SaveFuncNodeDBToFile(std::vector<FuncNodeDB>& table, std::string filename)
{
    std::ofstream fout(filename);
    if (!fout.is_open()) { std::cout << "err: " << filename << "\n"; return; }
    for (const auto& f : table) {
        //fout << std::uppercase << std::hex << std::setw(8) << std::setfill('0') << f.ptr << " " << f.name << "\n";
        fout << std::uppercase << std::hex << std::setw(8) << std::setfill('0') << f.ptr << " " << f.name << "\n";
        fout << std::nouppercase << std::dec;
    }
    fout.close();
}
void DumpFuncNodeDBToFile(std::vector<FuncNodeDB>& table) 
{
    for (const auto& f : table) {
        printf("0x%p %s\n", f.ptr, f.name.c_str());
    }
}

#define TRY_DETECT_FROM_FUNC_NAME 
#define ADD_LOCAL_INDEX
void FoundSubstringRaws()
{
    bool headerDetected = false;
    std::string currentFuncPtr;
    uint32_t currentFuncPtrParsed;
    std::string currentFuncName;
    int currentFuncStartLine = -1;
    std::vector<FuncNodeDB> funcList;

    // 1. Ввод имени входного файла
    std::string input_filename;
    std::cout << "Введите имя входного файла: "; // clear target 4 search
    std::getline(std::cin, input_filename);
    if (input_filename.find('.') == std::string::npos) input_filename += ".txt";

    // Ввод имени ida файла с функциями (разделено для чистого поиска первого) search in clear, translate to named
    std::string map_filename;
    std::cout << "Введите имя файла со всеми именами (или - для пропуска): "; // full db 4 cross
    std::getline(std::cin, map_filename);
    if (input_filename.find('.') == std::string::npos) input_filename += ".txt";
    if (map_filename != "-" && !map_filename.empty()) {
       funcList = ParseIdaFuncs(map_filename);
       std::cout << "PARSED FULL DB SIZE: " << funcList.size() << "\n";
       //DumpFuncNodeDBToFile(funcList);
    }

    // 2. Получаем размер
    std::ifstream fin_bin(input_filename, std::ios::binary | std::ios::ate);
    if (!fin_bin) { std::cerr << "Не удалось открыть файл " << input_filename << "\n"; return; }
    auto size = fin_bin.tellg();
    fin_bin.close();

    // 3. Чтение всех строк
    std::ifstream fin(input_filename);
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(fin, line)) lines.push_back(line);
    fin.close();

    std::cout << "Размер файла: " << size << " байт\n";
    std::cout << "Строк: " << lines.size() << "\n\n";

    // 4. Запрос шаблона
    std::string pattern;
    std::cout << "Введите подстроку для поиска: ";
    std::getline(std::cin, pattern);
    std::cout << "\n";

    bool lowercase = true;
    std::string pat_low = pattern;
    if (lowercase) std::transform(pat_low.begin(), pat_low.end(), pat_low.begin(), ::tolower);

    // Заголовок для вывода
    std::string header = "Line\t\tFuncPtr\t\tFuncName\tSubstring\t\t(Query: " + pat_low + ")\n";
    std::cout << header;

    struct Rec { int line, localine; std::string ptr, name, substr; };
    std::vector<Rec> results;

    for (int i = 0; i < (int)lines.size(); ++i) {
        const auto& rawLn = lines[i];
        // Очищенная для вывода строка (пробелы/табуляции)
        std::string displayLn = RemoveDoubleSpacesAndTabs(rawLn, true);

#ifdef TRY_DETECT_FROM_FUNC_NAME
        if (displayLn.rfind("//----- (", 0) == 0) {
            size_t p1 = displayLn.find('(');
            size_t p2 = displayLn.find(')');
            if (p1 != std::string::npos && p2 != std::string::npos && p2 > p1 + 1) {
                currentFuncPtr = displayLn.substr(p1 + 1, p2 - p1 - 1);
                std::stringstream ss;
                ss << std::hex << currentFuncPtr;
                ss >> currentFuncPtrParsed;
                headerDetected = true;
                currentFuncStartLine = -1;
                //currentFuncName = ""; // erase
            }
            continue;
        }
        //if (headerDetected && !displayLn.empty())
        if (headerDetected && !displayLn.empty() && (displayLn.rfind("//", 0) != 0)) // positive sp value comms fix
        {
            headerDetected = false;
            currentFuncStartLine = i + 2; // строка со скобкой { сразу после шапки (могут быть доп комменты между)
            size_t posParen = displayLn.find('(');
            if (posParen != std::string::npos) {
                size_t posSpace = displayLn.rfind(' ', posParen);
                currentFuncName = (posSpace != std::string::npos)
                    ? displayLn.substr(posSpace + 1, posParen - posSpace - 1)
                    : displayLn.substr(0, posParen);
                size_t atPos = currentFuncName.find('@');
                if (atPos != std::string::npos) currentFuncName.erase(atPos);
            }
            else currentFuncName.clear();
        }
#endif
        // Поиск в rawLn, без удаления двойных пробелов
        bool match = false;
        if (lowercase) {
            std::string low = rawLn;
            std::transform(low.begin(), low.end(), low.begin(), ::tolower);
            match = (low.find(pat_low) != std::string::npos);
        }
        else {
            match = (rawLn.find(pattern) != std::string::npos);
        }

        if (match) {
            std::string dispPtr = currentFuncPtr.empty() ? "-----------" : currentFuncPtr;
            std::string dispName = currentFuncName.empty() ? "-----------" : currentFuncName;
            // try found name from full db and rewrite
            if (!currentFuncPtr.empty()) { for (auto& f : funcList) { if (f.ptr == currentFuncPtrParsed) { dispName = f.name; break; } } }
            int localindex = (currentFuncStartLine > 0 && i + 1 >= currentFuncStartLine) ? ((i + 1) - currentFuncStartLine + 1) : 0;
            //std::cout << (i + 1) << '\t' << dispPtr << '\t' << dispName << '\t' << displayLn << "\n"; // orig
            CW_M();
            std::cout << (i + 1);
#if defined(ADD_LOCAL_INDEX) && defined(TRY_DETECT_FROM_FUNC_NAME)
            if (currentFuncStartLine > 0 && i + 1 >= currentFuncStartLine) { std::cout << " [" << localindex << "]"; }
            else { std::cout << " [ 0 ]"; }
#endif
            //CW_Y();
            CW_C();
            std::cout << '\t' << dispPtr;
            //CW_G();
            CW_Y();
            std::cout << '\t' << dispName;
            CW_C();
            //CW_W();
            std::cout << '\t' << displayLn << "\n";
            CW_DEF();
            results.push_back({ i + 1, localindex, dispPtr, dispName, displayLn });
        }
    }

    if (results.empty()) { std::cout << "Совпадений нет.\n"; return; }

    // 6. Автоматическое имя выходного файла
    std::string base = GetBaseName(input_filename);
    std::string safePat = pattern;
    for (auto& c : safePat) if (c == ' ') c = '_';
    std::string out_fn = safePat + "_" + base + ".txt";

    std::ofstream fout(out_fn);
    fout << header;
    for (auto& r : results) {
#if defined(ADD_LOCAL_INDEX) && defined(TRY_DETECT_FROM_FUNC_NAME)
        fout << r.line << ((r.localine > 0) ? (" [" + std::to_string(r.localine) + "]") : " [ 0 ]")
            << '\t' << r.ptr << '\t' << r.name << '\t' << r.substr << "\n";
#else
        fout << r.line << '\t' << r.ptr << '\t' << r.name << '\t' << r.substr << "\n";
#endif
    }
    fout.close();
    std::cout << "Результаты сохранены в " << out_fn << "\n";
}



void convbase()
{
    std::string input_path, output_path;
    int choice;

    // 1. Запрос входного файла
    std::cout << "Введите путь к входному файлу: ";
    std::getline(std::cin, input_path);

    // 2. Запрос направления конвертации
    std::cout << "Выберите конвертацию (1: hex->dec, 2: dec->hex): ";
    std::cin >> choice;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    // 3. Запрос выходного файла
    std::cout << "Введите путь к выходному файлу: ";
    std::getline(std::cin, output_path);

    std::ifstream fin(input_path);
    if (!fin) {
        std::cerr << "Не удалось открыть входной файл: " << input_path << "\n";
        return;
    }
    std::ofstream fout(output_path);
    if (!fout) {
        std::cerr << "Не удалось открыть выходной файл: " << output_path << "\n";
        return;
    }

    // Регулярки для поиска литералов
    std::regex re_line_prefix(R"(^\s*\d+\.\s*)");
    std::regex re_hex(R"(0x([0-9A-Fa-f]+)([uUlL]*))");
    std::regex re_dec(R"(\b(\d+)([uUlL]*)\b)");

    std::string line;
    while (std::getline(fin, line)) {
        std::smatch m;
        std::string prefix, rest;

        // Отделяем номер строки в начале (если есть)
        if (std::regex_search(line, m, re_line_prefix)) {
            prefix = m.str(0);
            rest = line.substr(m.length());
        }
        else {
            rest = line;
        }

        // Выбираем нужную конвертацию
        std::string result;
        std::size_t last_pos = 0;
        if (choice == 1) {
            // hex -> dec
            for (auto it = std::sregex_iterator(rest.begin(), rest.end(), re_hex);
                it != std::sregex_iterator();
                ++it)
            {
                auto match = *it;
                // добавляем текст до совпадения
                result += rest.substr(last_pos, match.position() - last_pos);
                // конвертация
                unsigned long val = std::stoul(match[1].str(), nullptr, 16);
                result += std::to_string(val) + match[2].str();
                last_pos = match.position() + match.length();
            }
        }
        else {
            // dec -> hex
            for (auto it = std::sregex_iterator(rest.begin(), rest.end(), re_dec);
                it != std::sregex_iterator();
                ++it)
            {
                auto match = *it;
                // добавляем текст до совпадения
                result += rest.substr(last_pos, match.position() - last_pos);
                // конвертация
                unsigned long val = std::stoul(match[1].str());
                std::ostringstream oss;
                oss << "0x" << std::uppercase << std::hex << val << match[2].str();
                result += oss.str();
                last_pos = match.position() + match.length();
            }
        }
        // добавляем остаток строки
        result += rest.substr(last_pos);

        // Пишем в выходной файл
        fout << prefix << result << "\n";
    }

    std::cout << "Готово! Результат записан в: " << output_path << "\n";
}

int main()
{
    //std::setlocale(LC_ALL, "");
    U_SetCurrentDirectory();
    InitConsole();

    //{ // vcs tab
    //    std::vector<FuncNodeDB> fL = ParseIdaFuncs("vcs.c");
    //    //for (auto& f : fL) { printf("0x%p %s\n", f.ptr, f.name.c_str()); }
    //    DumpFuncNodeDBToFile(fL);
    //    SaveFuncNodeDBToFile(fL, "VCSTAB.TXT");
    //    return 0;
    //}
    //{ // lcs tab
    //    std::vector<FuncNodeDB> fL = ParseIdaFuncs("lcs.c");
    //    //for (auto& f : fL) { printf("0x%p %s\n", f.ptr, f.name.c_str()); }
    //    DumpFuncNodeDBToFile(fL);
    //    SaveFuncNodeDBToFile(fL, "LCSTAB.TXT");
    //    return 0;
    //}

#if 0 // 2 source files, dump diff
    { // cmp test
        // Parse input
        std::vector<FuncNodeDB> vcsfL = ParseIdaFuncs("vcs.c");
        printf("vcs: %d\n", vcsfL.size());
        std::vector<FuncNodeDB> lcsfL = ParseIdaFuncs("lcs.c");
        printf("lcs: %d\n", lcsfL.size());

        // Prepare lowercase name lists
        auto buildNameLists = [](const std::vector<FuncNodeDB>& funcs,
            std::vector<std::string>& namesOrig,
            std::vector<std::string>& namesLower) {
                for (const auto& func : funcs) {
                    std::string name = RemoveDoubleSpacesAndTabs(func.name, true);
                    if (!name.empty() && name[0] == '*') name.erase(0, 1);
                    namesOrig.push_back(name);
                    std::string low = name;
                    std::transform(low.begin(), low.end(), low.begin(), ::tolower);
                    namesLower.push_back(low);
                }
        };

        std::vector<std::string> vcsNamesOrig, vcsNamesLower;
        std::vector<std::string> lcsNamesOrig, lcsNamesLower;
        buildNameLists(vcsfL, vcsNamesOrig, vcsNamesLower);
        buildNameLists(lcsfL, lcsNamesOrig, lcsNamesLower);

        // Define substrings to skip
        std::vector<std::string> skipSubs =
        { 
            "sub_",
            "google",
            "std::",
            "hal::",
            "social",
            //"base::",
        };

        // Build lowercase skip list
        std::vector<std::string> skipSubsLower;
        for (const auto& sub : skipSubs) {
            std::string lowerSub = sub;
            std::transform(lowerSub.begin(), lowerSub.end(), lowerSub.begin(), ::tolower);
            skipSubsLower.push_back(lowerSub);
        }

        // Collect missing functions
        std::vector<std::string> missingFuncs;
        for (size_t i = 0; i < lcsNamesLower.size(); ++i) {
            const auto& nameLower = lcsNamesLower[i];
            if (std::find(vcsNamesLower.begin(), vcsNamesLower.end(), lcsNamesLower[i]) != vcsNamesLower.end())
                continue;

            // Skip by substrings
            bool skip = false;
            //for (auto& sub : skipSubs) {
            //    if (lcsNamesLower[i].rfind(sub, 0) == 0) {
            //        skip = true;
            //        break;
            //    }
            //}
            for (const auto& subLower : skipSubsLower) {
                if (nameLower.rfind(subLower, 0) == 0) {
                    skip = true;
                    break;
                }
            }
            if (skip) continue;
            // Skip names without alphanumeric or underscore
            if (lcsNamesLower[i].find_first_of("abcdefghijklmnopqrstuvwxyz0123456789_") == std::string::npos)
                continue;
            // Add original name
            missingFuncs.push_back(lcsNamesOrig[i]);
        }

        // 1) Output unique class names
        std::unordered_set<std::string> classNames;
        for (const auto& fname : missingFuncs) {
            auto pos = fname.rfind("::");
            if (pos != std::string::npos) {
                classNames.insert(fname.substr(0, pos));
            }
        }

        // Filter classes that contain at least one letter
        std::vector<std::string> filteredClasses;
        for (const auto& cls : classNames) {
            if (cls.find_first_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ") != std::string::npos) {
                filteredClasses.push_back(cls);
            }
        }

        // Save class names to file
        std::ofstream classesFile("classes.txt");
        if (classesFile.is_open()) {
            for (const auto& cls : filteredClasses) {
                classesFile << cls << '\n';
            }
            classesFile.close();
        }
        else {
            std::cerr << "Failed to open classes.txt for writing\n";
        }

        // Build map for grouping
        std::unordered_map<std::string, std::vector<std::string>> classMap;
        for (const auto& fname : missingFuncs) {
            auto pos = fname.rfind("::");
            if (pos != std::string::npos) {
                std::string cls = fname.substr(0, pos);
                classMap[cls].push_back(fname);
            }
            else {
                classMap["<global>"].push_back(fname);
            }
        }

        // Save functions grouped by class to file in same format as console
        std::ofstream funcsFile("funcs.txt");
        if (funcsFile.is_open()) {
            for (const auto& cls : filteredClasses) {
                funcsFile << "\r\n\r\nClass " << cls << "\n";
                auto it = classMap.find(cls);
                if (it != classMap.end()) {
                    for (const auto& fn : it->second) {
                        funcsFile << '\t' << fn << '\n';
                    }
                }
            }
            // Handle global functions
            auto globIt = classMap.find("<global>");
            if (globIt != classMap.end()) {
                funcsFile << "\r\n\r\nClass <global>\n";
                for (const auto& fn : globIt->second) {
                    funcsFile << '\t' << fn << '\n';
                }
            }
            funcsFile.close();
        }
        else {
            std::cerr << "Failed to open funcs.txt for writing\n";
        }



        std::cout << "Unique class names\n";
        for (const auto& cls : classNames) {
            std::cout << cls << std::endl;
        }

        //// 2) Group and output functions by class
        //std::unordered_map<std::string, std::vector<std::string>> classMap;
        //for (const auto& fname : missingFuncs) {
        //    auto pos = fname.rfind("::");
        //    if (pos != std::string::npos) {
        //        std::string cls = fname.substr(0, pos);
        //        classMap[cls].push_back(fname);
        //    }
        //    else {
        //        classMap["<global>"].push_back(fname);
        //    }
        //}

        std::cout << "\nFunctions grouped by class\n";
        for (const auto& kv : classMap) {
            std::cout << "\n\nClass " << kv.first << ":\n";
            for (const auto& fn : kv.second) {
                std::cout << "  " << fn << std::endl;
            }
        }
        //copyToClipboard(clipboardText.c_str());
        return 0;
    }
#endif

    bool sh = (GetAsyncKeyState(VK_SHIFT) & 0x8000);
    if (sh) { convbase(); } // if call with nums, no enums
    else { FoundSubstringRaws(); }
    char i;
    std::cin >> i;
    return 0;
}