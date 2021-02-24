// Copyright (c) 2019-present Anonymous275.
// BeamMP Server code is not in the public domain and is not free software.
// One must be granted explicit permission by the copyright holder in order to modify or distribute any part of the source or binaries.
// Anything else is prohibited. Modified works may not be published and have be upstreamed to the official repository.
///
/// Created by Anonymous275 on 10/29/2020
///

#include "Lua/LuaSystem.hpp"
#ifdef WIN32
#include <conio.h>
#include <windows.h>
#else // *nix
typedef unsigned long DWORD, *PDWORD, *LPDWORD;
#include <termios.h>
#include <unistd.h>
#endif // WIN32
#include "Logger.h"
#include <array>
#include <cstring>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>

std::vector<std::string> QConsoleOut;
std::string CInputBuff;
std::mutex MLock;
std::unique_ptr<Lua> LuaConsole;
void HandleInput(const std::string& cmd) {
    std::cout << std::endl;
    if (cmd == ("exit")) {
        _Exit(0);
    } else if (cmd == ("clear") || cmd == ("cls")) {
        // 2J is clearscreen, H is reset position to top-left
        ConsoleOut(("\x1b[2J\x1b[H"));
    } else {
        LuaConsole->Execute(cmd);
    }
}

void ProcessOut() {
    printf("%c[2K\r", 27);
    for (const std::string& msg : QConsoleOut)
        if (!msg.empty())
            std::cout << msg;
    MLock.lock();
    QConsoleOut.clear();
    MLock.unlock();
    std::cout << "> " << CInputBuff << std::flush;
}

void ConsoleOut(const std::string& msg) {
    MLock.lock();
    QConsoleOut.emplace_back(msg);
    MLock.unlock();
}

[[noreturn]] void OutputRefresh() {
    DebugPrintTID();
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        ProcessOut();
    }
}

#ifndef WIN32

static struct termios old, current;

void initTermios(int echo) {
    tcgetattr(0, &old); /* grab old terminal i/o settings */
    current = old; /* make new settings same as old settings */
    current.c_lflag &= ~ICANON; /* disable buffered i/o */
    if (echo) {
        current.c_lflag |= ECHO; /* set echo mode */
    } else {
        current.c_lflag &= ~ECHO; /* set no echo mode */
    }
    tcsetattr(0, TCSANOW, &current); /* use these new terminal i/o settings now */
}

void resetTermios(void) {
    tcsetattr(0, TCSANOW, &old);
}

char getch_(int echo) {
    char ch;
    initTermios(echo);
    read(STDIN_FILENO, &ch, 1);
    resetTermios();
    return ch;
}

char _getch(void) {
    return getch_(0);
}

#endif // WIN32

void SetupConsole() {
#if defined(WIN32)
    DWORD outMode = 0;
    HANDLE stdoutHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    if (stdoutHandle == INVALID_HANDLE_VALUE) {
        error("Invalid console handle! Inputs will not work properly");
        return;
    }
    if (!GetConsoleMode(stdoutHandle, &outMode)) {
        error("Invalid console mode! Inputs will not work properly");
        return;
    }
    // Enable ANSI escape codes
    outMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    if (!SetConsoleMode(stdoutHandle, outMode)) {
        error("failed to set console mode! Inputs will not work properly");
        return;
    }
#else
#endif // WIN32
}

static std::vector<std::string> ConsoleHistory {};
// buffer used to revert back to what we were writing when we go all the way forward in history
static std::string LastInputBuffer {};
static size_t ConsoleHistoryReadIndex { 0 };

static inline void ConsoleHistoryAdd(const std::string& cmd) {
    LastInputBuffer.clear();
    ConsoleHistory.push_back(cmd);
    ConsoleHistoryReadIndex = ConsoleHistory.size();
}

static inline void ConsoleHistoryEnsureBounds() {
    if (ConsoleHistoryReadIndex >= ConsoleHistory.size()) {
        ConsoleHistoryReadIndex = ConsoleHistory.size() - 1;
    }
}

static inline void ConsoleHistoryGoBack() {
    if (ConsoleHistoryReadIndex > 0) {
        --ConsoleHistoryReadIndex;
        ConsoleHistoryEnsureBounds();
    }
}

static inline void ConsoleHistoryGoForward() {
    ++ConsoleHistoryReadIndex;
    ConsoleHistoryEnsureBounds();
}

static std::string CompositeInput;
static bool CompositeInputExpected { false };

static void ProcessCompositeInput() {
#ifdef WIN32
    if (CompositeInput.size() == 1 && memcmp(CompositeInput.data(), std::array<char, 1> { 72 }.data(), 1) == 0) {
#else // unix
    if (CompositeInput.size() == 2 && memcmp(CompositeInput.data(), std::array<char, 2> { 91, 65 }.data(), 2) == 0) {
#endif // WIN32

        // UP ARROW
        if (!ConsoleHistory.empty()) {
            ConsoleHistoryGoBack();
            CInputBuff = ConsoleHistory.at(ConsoleHistoryReadIndex);
        }
#ifdef WIN32
    } else if (CompositeInput.size() == 1 && memcmp(CompositeInput.data(), std::array<char, 1> { 80 }.data(), 1) == 0) {
#else // unix
    } else if (CompositeInput.size() == 2 && memcmp(CompositeInput.data(), std::array<char, 2> { 91, 66 }.data(), 2) == 0) {
#endif // WIN32

        // DOWN ARROW
        if (!ConsoleHistory.empty()) {
            if (ConsoleHistoryReadIndex == ConsoleHistory.size() - 1) {
                CInputBuff = LastInputBuffer;
            } else {
                ConsoleHistoryGoForward();
                CInputBuff = ConsoleHistory.at(ConsoleHistoryReadIndex);
            }
        }
    } else {
        // not composite input, we made a mistake, so lets just add it to the buffer like usual
        CInputBuff += CompositeInput;
    }
    // ensure history doesnt grow too far beyond a max
    static constexpr size_t MaxHistory = 10;
    if (ConsoleHistory.size() > 2 * MaxHistory) {
        decltype(ConsoleHistory) NewHistory(ConsoleHistory.begin() + ConsoleHistory.size() - MaxHistory, ConsoleHistory.end());
        ConsoleHistory = std::move(NewHistory);
        ConsoleHistoryReadIndex = ConsoleHistory.size();
    }
}

void ReadCin() {
    DebugPrintTID();
    size_t null_byte_counter = 0;
    while (true) {
        int In = _getch();
        //info(std::to_string(In));
        if (In == 0) {
            ++null_byte_counter;
            if (null_byte_counter > 50) {
                info(("too many null bytes in input, this is now assumed to be a background thread - console input is now disabled"));
                break;
            }
        }
        if (CompositeInputExpected) {
            CompositeInput += char(In);
#ifdef WIN32
            if (CompositeInput.size() == 1) {
#else // unix
            if (CompositeInput.size() == 2) {
#endif // WIN32
                CompositeInputExpected = false;
                ProcessCompositeInput();
            }
            continue;
        }
        if (In == 13 || In == '\n') {
            if (!CInputBuff.empty()) {
                HandleInput(CInputBuff);
                ConsoleHistoryAdd(CInputBuff);
                CInputBuff.clear();
            }
        } else if (In == 8 || In == 127) {
            if (!CInputBuff.empty())
                CInputBuff.pop_back();
        } else if (In == 4) {
            CInputBuff = "exit";
            HandleInput(CInputBuff);
            CInputBuff.clear();
        } else if (In == 12) {
            CInputBuff = "clear";
            HandleInput(CInputBuff);
            CInputBuff.clear();
#ifdef WIN32
        } else if (In == 224) {
#else // unix
        } else if (In == 27) {
#endif // WIN32

            // escape char, assume stuff follows
            CompositeInputExpected = true;
            CompositeInput.clear();
        } else if (!isprint(In)) {
            // ignore
        } else {
            CInputBuff += char(In);
            LastInputBuffer = CInputBuff;
        }
    }
}
void ConsoleInit() {
    SetupConsole();
    LuaConsole = std::make_unique<Lua>(true);
    LuaConsole->Init();
    printf("> ");
    std::thread In(ReadCin);
    In.detach();
    std::thread Out(OutputRefresh);
    Out.detach();
}
