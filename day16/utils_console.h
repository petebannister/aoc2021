#pragma once

#include "utils.h"
#define WIN32_LEAN_AND_MEAN
#define nominmax
#include <Windows.h>
#endif

struct Console
{
    HANDLE                     hStdOut = 0;
    CONSOLE_SCREEN_BUFFER_INFO csbi = {};
    DWORD                      cellCount = 0;
    COORD                      homeCoords = { 0, 0 };
    DWORD                      count = 0;

    Console() {
        hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
        if (hStdOut == INVALID_HANDLE_VALUE) raise("console");
        if (!GetConsoleScreenBufferInfo(hStdOut, &csbi)) raise("console2");
        cellCount = csbi.dwSize.X * csbi.dwSize.Y;

        clear();
        showCursor(false);
    }

    void clear() {
        /* Fill the entire buffer with spaces */
        if (!FillConsoleOutputCharacter(
            hStdOut,
            (TCHAR)' ',
            cellCount,
            homeCoords,
            &count
        )) return;

        /* Fill the entire buffer with the current colors and attributes */
        if (!FillConsoleOutputAttribute(
            hStdOut,
            csbi.wAttributes,
            cellCount,
            homeCoords,
            &count
        )) return;
    }

    void home() {
        SetConsoleCursorPosition(hStdOut, homeCoords);
    }

    void move(point p) {
        COORD xy;
        xy.X = p.x;
        xy.Y = p.y;
        SetConsoleCursorPosition(hStdOut, xy);
    }

    void putch(point p, char ch) {
        move(p);
        WriteConsoleA(hStdOut, &ch, 1, nullptr, nullptr);
    }
    void put(point p, char ch) {
        putch(p, ch);
    }

    template <typename...Args>
    void put(point p, Args&&... args) {
        puts(p, args_to_string(std::forward<Args>(args)...));
    }
    template <typename...Args>
    void write(point p, Args&&... args) {
        puts(p, args_to_string(std::forward<Args>(args)...));
    }

    template <typename...Args>
    void writeln(point p, Args&&... args) {
        puts(p, args_to_string(std::forward<Args>(args)...));
        putch('\n');
    }
    void puts(point p, char const* s) {
        move(p);
        WriteConsoleA(hStdOut, s, (DWORD)strlen(s), nullptr, nullptr);
    }
    void puts(point p, std::string const& s) {
        move(p);
        WriteConsoleA(hStdOut, s.c_str(), (DWORD)s.size(), nullptr, nullptr);
    }

    void title(std::string const& s) {
        SetConsoleTitleA(s.c_str());
    }

    void showCursor(bool showFlag)
    {
        CONSOLE_CURSOR_INFO     cursorInfo;

        GetConsoleCursorInfo(hStdOut, &cursorInfo);
        cursorInfo.bVisible = showFlag; // set the cursor visibility
        SetConsoleCursorInfo(hStdOut, &cursorInfo);
    }
    static bool key(int kc) {
        return (GetAsyncKeyState(kc) & 0x8000);
    }

};
