#define NOMINMAX
#include "Windows.h" 
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdlib>    // ��� system("cls")
#include <clocale>    // ��� setlocale
#include <algorithm>  // ��� std::transform
#include <limits>     // ��� std::numeric_limits
#include <regex>
#include <sstream>

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


void FoundSubstringRaws()
{
    // 1. ������ ����� �������� �����
    std::string input_filename;
    std::cout << "������� ��� �������� ����� (� ����������� ��� ���): ";
    std::getline(std::cin, input_filename);
    if (input_filename.find('.') == std::string::npos) {
        input_filename += ".txt";
    }

    // 2. ��������� ���� � �������� ������, ����� �������� ������
    std::ifstream fin_bin(input_filename, std::ios::binary | std::ios::ate);
    if (!fin_bin) {
        std::cerr << "�� ������� ������� ���� \"" << input_filename << "\" ��� ������.\n";
        return;
    }
    std::streampos size = fin_bin.tellg();
    fin_bin.close();

    // 3. ������ ���� ��������� � ��������� ������ � ������
    std::ifstream fin(input_filename);
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(fin, line)) {
        lines.push_back(line);
    }
    fin.close();

    // 4. ������� ������ � ���������� �����
    std::cout << "������ �����: " << size << " ����\n";
    std::cout << "���������� �����: " << lines.size() << "\n\n";

    // 5. ������ ��������� ��� ������
    std::string pattern;
    std::cout << "������� ��������� ��� ������: ";
    std::getline(std::cin, pattern);

    // 6. ������ ��������� �� �������
    std::cout << "������������ �������? (y/n): ";
    char case_choice;
    std::cin >> case_choice;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    bool lowercase = (case_choice == 'y' || case_choice == 'Y');
    // ���� ����� ������������ ������� � �������� ������ � �������
    std::string pattern_low;
    if (lowercase) {
        pattern_low = pattern;
        std::transform(pattern_low.begin(), pattern_low.end(), pattern_low.begin(), ::tolower);
    }

    //system("cls");

    //// 7. ���� � ������� ������, ���������� ���������
    //std::vector<std::string> found;
    //int i = 1;
    //for (const auto& l : lines) {
    //    if (l.find(pattern) != std::string::npos) {
    //        printf("%d. %s\n", i++, l.c_str());
    //        found.push_back(l);
    //    }
    //}
    //if (found.empty()) {
    //    std::cout << "���������� �� �������.\n";
    //}

    // 7. ���� ����������: ��������� (����� ������, �����)
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

    // 8. ����� � �������
    if (found.empty()) {
        std::cout << "���������� �� �������.\n";
    }
    else {
        int cnt = 1;
        for (auto& p : found) {
            // ������: 1 [147555]    ������
            std::cout << cnt << " [" << p.first << "]\t" << p.second << "\n";
            ++cnt;
        }
    }

    // 9. ������ ����� ��������� �����
    std::cout << "\n������� ��� ��������� ����� ��� ���������� �����������: ";
    std::string output_filename;
    std::getline(std::cin, output_filename);
    // ���� ������������ �� ������ ����������, ��������� .txt �� ���������
    if (output_filename.find('.') == std::string::npos) {
        output_filename += ".txt";
    }

    // 10. ���� ���� ����������, ���������� ������������ � ����������
    std::ifstream test(output_filename);
    if (test) {
        std::cout << "���� \"" << output_filename << "\" ���������� � ����� �����������.\n";
        test.close();
    }

    // 11. ��������� ���� �� ������ (� �����������) � ��������� ��������� ������
    std::ofstream fout(output_filename, std::ios::trunc);
    if (!fout) {
        std::cerr << "�� ������� ������� ���� \"" << output_filename << "\" ��� ������.\n";
        return;
    }
    //for (const auto& l : found) {
    //    fout << l << "\n";
    //}
    for (auto& p : found) {
        fout << p.first << ".\t" << p.second << "\n";
    }
    fout.close();

    std::cout << "���������� ��������� � ���� \"" << output_filename << "\".\n";
}

void convbase()
{
    std::string input_path, output_path;
    int choice;

    // 1. ������ �������� �����
    std::cout << "������� ���� � �������� �����: ";
    std::getline(std::cin, input_path);

    // 2. ������ ����������� �����������
    std::cout << "�������� ����������� (1: hex->dec, 2: dec->hex): ";
    std::cin >> choice;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    // 3. ������ ��������� �����
    std::cout << "������� ���� � ��������� �����: ";
    std::getline(std::cin, output_path);

    std::ifstream fin(input_path);
    if (!fin) {
        std::cerr << "�� ������� ������� ������� ����: " << input_path << "\n";
        return;
    }
    std::ofstream fout(output_path);
    if (!fout) {
        std::cerr << "�� ������� ������� �������� ����: " << output_path << "\n";
        return;
    }

    // ��������� ��� ������ ���������
    std::regex re_line_prefix(R"(^\s*\d+\.\s*)");
    std::regex re_hex(R"(0x([0-9A-Fa-f]+)([uUlL]*))");
    std::regex re_dec(R"(\b(\d+)([uUlL]*)\b)");

    std::string line;
    while (std::getline(fin, line)) {
        std::smatch m;
        std::string prefix, rest;

        // �������� ����� ������ � ������ (���� ����)
        if (std::regex_search(line, m, re_line_prefix)) {
            prefix = m.str(0);
            rest = line.substr(m.length());
        }
        else {
            rest = line;
        }

        // �������� ������ �����������
        std::string result;
        std::size_t last_pos = 0;
        if (choice == 1) {
            // hex -> dec
            for (auto it = std::sregex_iterator(rest.begin(), rest.end(), re_hex);
                it != std::sregex_iterator();
                ++it)
            {
                auto match = *it;
                // ��������� ����� �� ����������
                result += rest.substr(last_pos, match.position() - last_pos);
                // �����������
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
                // ��������� ����� �� ����������
                result += rest.substr(last_pos, match.position() - last_pos);
                // �����������
                unsigned long val = std::stoul(match[1].str());
                std::ostringstream oss;
                oss << "0x" << std::uppercase << std::hex << val << match[2].str();
                result += oss.str();
                last_pos = match.position() + match.length();
            }
        }
        // ��������� ������� ������
        result += rest.substr(last_pos);

        // ����� � �������� ����
        fout << prefix << result << "\n";
    }

    std::cout << "������! ��������� ������� �: " << output_path << "\n";
}

int main()
{
    //std::setlocale(LC_ALL, "");
    U_SetCurrentDirectory();
    InitConsole();

    FoundSubstringRaws();
    //convbase();

    return 0;
}