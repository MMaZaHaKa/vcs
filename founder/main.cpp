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


int main()
{
    //std::setlocale(LC_ALL, "");
    U_SetCurrentDirectory();
    InitConsole();

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
        return 1;
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
        return 1;
    }
    //for (const auto& l : found) {
    //    fout << l << "\n";
    //}
    for (auto& p : found) {
        fout << p.first << ".\t" << p.second << "\n";
    }
    fout.close();

    std::cout << "���������� ��������� � ���� \"" << output_filename << "\".\n";
    return 0;
}