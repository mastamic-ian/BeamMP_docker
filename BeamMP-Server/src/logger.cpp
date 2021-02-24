// Copyright (c) 2019-present Anonymous275.
// BeamMP Server code is not in the public domain and is not free software.
// One must be granted explicit permission by the copyright holder in order to modify or distribute any part of the source or binaries.
// Anything else is prohibited. Modified works may not be published and have be upstreamed to the official repository.
///
/// Created by Anonymous275 on 7/17/2020
///
#include "Logger.h"
#include "RWMutex.h"
#include "Security/Enc.h"
#include "Settings.h"
#include <chrono>
#include <fstream>
#include <mutex>
#include <sstream>
#include <thread>
#include <unordered_map>

static RWMutex ThreadNameMapMutex;
static std::unordered_map<std::thread::id, std::string> ThreadNameMap;

std::string ThreadName() {
    ReadLock lock(ThreadNameMapMutex);
    std::string Name;
    if (ThreadNameMap.find(std::this_thread::get_id()) != ThreadNameMap.end()) {
        Name = ThreadNameMap.at(std::this_thread::get_id());
    } else {
        std::stringstream ss;
        ss << std::this_thread::get_id();
        Name = ss.str();
    }
    return Name;
}

void SetThreadName(const std::string& Name, bool overwrite) {
    WriteLock lock(ThreadNameMapMutex);
    if (overwrite || ThreadNameMap.find(std::this_thread::get_id()) == ThreadNameMap.end()) {
        ThreadNameMap[std::this_thread::get_id()] = Name;
    }
}

std::string getDate() {
    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    time_t tt = std::chrono::system_clock::to_time_t(now);
    tm local_tm {};
#ifdef WIN32
    localtime_s(&local_tm, &tt);
#else // unix
    localtime_r(&tt, &local_tm);
#endif // WIN32
    std::stringstream date;
    int S = local_tm.tm_sec;
    int M = local_tm.tm_min;
    int H = local_tm.tm_hour;
    std::string Secs = (S > 9 ? std::to_string(S) : "0" + std::to_string(S));
    std::string Min = (M > 9 ? std::to_string(M) : "0" + std::to_string(M));
    std::string Hour = (H > 9 ? std::to_string(H) : "0" + std::to_string(H));
    date
        << "["
        << local_tm.tm_mday << "/"
        << local_tm.tm_mon + 1 << "/"
        << local_tm.tm_year + 1900 << " "
        << Hour << ":"
        << Min << ":"
        << Secs
        << "] ";
    if (Debug) {
        date << ThreadName()
             << " ";
    }
    return date.str();
}

void InitLog() {
    std::ofstream LFS;
    LFS.open(("Server.log"));
    if (!LFS.is_open()) {
        error(("logger file init failed!"));
    } else
        LFS.close();
}
std::mutex LogLock;

void DebugPrintTIDInternal(const std::string& func, bool overwrite) {
    // we need to print to cout here as we might crash before all console output is handled,
    // due to segfaults or asserts.
    SetThreadName(func, overwrite);
#ifdef DEBUG
    std::scoped_lock Guard(LogLock);
    std::stringstream Print;
    Print << "(debug build) Thread '" << std::this_thread::get_id() << "' is " << func << "\n";
    ConsoleOut(Print.str());
#endif // DEBUG
}

void addToLog(const std::string& Line) {
    std::ofstream LFS("Server.log", std::ios_base::app);
    LFS << Line.c_str();
}
void info(const std::string& toPrint) {
    std::scoped_lock Guard(LogLock);
    std::string Print = getDate() + ("[INFO] ") + toPrint + "\n";
    ConsoleOut(Print);
    addToLog(Print);
}
void debug(const std::string& toPrint) {
    if (!Debug)
        return;
    std::scoped_lock Guard(LogLock);
    std::string Print = getDate() + ("[DEBUG] ") + toPrint + "\n";
    ConsoleOut(Print);
    addToLog(Print);
}
void warn(const std::string& toPrint) {
    std::scoped_lock Guard(LogLock);
    std::string Print = getDate() + ("[WARN] ") + toPrint + "\n";
    ConsoleOut(Print);
    addToLog(Print);
}
void error(const std::string& toPrint) {
    std::scoped_lock Guard(LogLock);
    std::string Print = getDate() + ("[ERROR] ") + toPrint + "\n";
    ConsoleOut(Print);
    addToLog(Print);
}
void except(const std::string& toPrint) {
    std::scoped_lock Guard(LogLock);
    std::string Print = getDate() + ("[EXCEP] ") + toPrint + "\n";
    ConsoleOut(Print);
    addToLog(Print);
}
