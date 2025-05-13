//+------------------------------------------------------------------+
//|                                           mirrortrade.dll         |
//|              Huda Choirul Anam - Copyright 2025, MetaQuotes Ltd. |
//|                                        https://www.ulfasanda.com |
//+------------------------------------------------------------------+

#include <windows.h>
#include <string>
#include <mutex>
#include <fstream>
#include <iomanip>
#include <ctime>
#include <sstream>
#include <filesystem>

// Struktur untuk menyimpan sinyal order
struct OrderSignal {
    char pair[12];
    int type; // 0 = BUY, 1 = SELL
    double sl;
    double tp;
    double lot;
    bool isValid = false; // Penanda apakah sinyal valid
};

OrderSignal g_signal;
std::mutex g_mutex;

// Fungsi untuk memastikan direktori log ada
void ensureLogDirectory() {
    const std::string logDirectory = "C:\\Users\\Public\\Documents\\MirrorTrade";
    if (!std::filesystem::exists(logDirectory)) {
        std::filesystem::create_directories(logDirectory);
    }
}

// Fungsi untuk mendapatkan tanggal hari ini dalam format YYYY.MM.DD
std::string getCurrentDate() {
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);

    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y.%m.%d");
    return oss.str();
}

// Fungsi untuk menulis log ke file
void writeToLog(const std::string& phase, const std::string& message) {
    ensureLogDirectory();
    std::string logFilePath = "C:\\Users\\Public\\Documents\\MirrorTrade\\" + getCurrentDate() + "-" + phase + ".log";

    std::ofstream logFile(logFilePath, std::ios::app); // Buka file dalam mode append
    if (logFile.is_open()) {
        auto t = std::time(nullptr);
        auto tm = *std::localtime(&t);

        logFile << "[" << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") << "] " << message << std::endl;
        logFile.close();
    }
}

// Fungsi untuk mendaftarkan server
extern "C" __declspec(dllexport) bool __stdcall RegisterServer(const char* kodeserver) {
    std::lock_guard<std::mutex> lock(g_mutex);
    g_signal.isValid = false; // Reset sinyal saat server mendaftar

    std::ostringstream oss;
    oss << "[Phase 1] EA to DLL: Server registered with code: " << kodeserver;
    writeToLog("Phase1-EAtoDLL", oss.str());

    return true;
}

// Fungsi untuk menghapus server
extern "C" __declspec(dllexport) bool __stdcall UnregisterServer(const char* kodeserver) {
    std::lock_guard<std::mutex> lock(g_mutex);
    g_signal.isValid = false; // Reset sinyal saat server dihapus

    std::ostringstream oss;
    oss << "[Phase 4] DLL to EA: Server unregistered with code: " << kodeserver;
    writeToLog("Phase4-DLLtoEA", oss.str());

    return true;
}

// Fungsi untuk mengirim sinyal order ke MirrorTrade.exe
extern "C" __declspec(dllexport) void __stdcall SendOrderSignal(
    const char* kodeserver, const char* pair, int type, double sl, double tp, double lot) {
    if (!kodeserver || !pair) return;

    std::lock_guard<std::mutex> lock(g_mutex);
    strncpy(g_signal.pair, pair, sizeof(g_signal.pair) - 1);
    g_signal.pair[sizeof(g_signal.pair) - 1] = '\0';
    g_signal.type = type;
    g_signal.sl = sl;
    g_signal.tp = tp;
    g_signal.lot = lot;
    g_signal.isValid = true;

    // Log aktivitas EA ke DLL
    std::ostringstream oss1;
    oss1 << "[Phase 1] EA to DLL: Signal sent -> Server=" << kodeserver << ", Pair=" << pair
         << ", Type=" << (type == 0 ? "BUY" : "SELL")
         << ", SL=" << sl << ", TP=" << tp << ", Lot=" << lot;
    writeToLog("Phase1-EAtoDLL", oss1.str());

    // Log aktivitas DLL ke EXE
    std::ostringstream oss2;
    oss2 << "[Phase 2] DLL to EXE: Sending data to EXE -> Server=" << kodeserver << ", Pair=" << pair
         << ", Type=" << (type == 0 ? "BUY" : "SELL")
         << ", SL=" << sl << ", TP=" << tp << ", Lot=" << lot;
    writeToLog("Phase2-DLLtoEXE", oss2.str());
}

// Fungsi untuk memeriksa status server
extern "C" __declspec(dllexport) bool __stdcall CheckServerStatus(const char* kodeserver) {
    std::lock_guard<std::mutex> lock(g_mutex);

    std::ostringstream oss;
    oss << "[Phase 4] DLL to EA: Check server status -> Server=" << kodeserver
        << ", Status=" << (g_signal.isValid ? "Active" : "Inactive");
    writeToLog("Phase4-DLLtoEA", oss.str());

    return g_signal.isValid; // Jika ada sinyal valid, server dianggap aktif
}
