//+------------------------------------------------------------------+
//|                                           MirrorTrade.exe         |
//|              Huda Choirul Anam - Copyright 2025, MetaQuotes Ltd. |
//|                                        https://www.ulfasanda.com |
//+------------------------------------------------------------------+

#include <iostream>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <stdexcept>

// Struktur untuk menyimpan sinyal order
struct OrderSignal {
    std::string pair;
    int type; // 0 = BUY, 1 = SELL
    double sl;
    double tp;
    double lot;
};

// Struktur untuk menyimpan status server
struct ServerStatus {
    bool isActive = false;
    std::vector<OrderSignal> signals;
};

std::unordered_map<std::string, ServerStatus> servers;
std::mutex serversMutex;
std::condition_variable activityNotifier;

// Fungsi untuk menangani error
void handleException(const std::string& context) {
    try {
        throw;
    } catch (const std::exception& ex) {
        std::cerr << "[ERROR] Exception in " << context << ": " << ex.what() << std::endl;
    } catch (...) {
        std::cerr << "[ERROR] Unknown exception in " << context << std::endl;
    }
}

// Fungsi untuk menerima sinyal dari server (DLL)
void receiveSignal(const std::string& serverCode, const OrderSignal& signal) {
    try {
        std::lock_guard<std::mutex> lock(serversMutex);

        // Pastikan server terdaftar dan aktif
        if (servers.find(serverCode) == servers.end() || !servers[serverCode].isActive) {
            std::cerr << "[ERROR] Signal received from inactive or unknown server: " << serverCode << std::endl;
            return;
        }

        servers[serverCode].signals.push_back(signal);
        std::cout << "[SERVER] Signal received: " << signal.pair
                  << ", Type: " << (signal.type == 0 ? "BUY" : "SELL")
                  << ", SL: " << signal.sl << ", TP: " << signal.tp
                  << ", Lot: " << signal.lot << std::endl;
        activityNotifier.notify_all();
    } catch (...) {
        handleException("receiveSignal");
    }
}

// Fungsi utama untuk melayani sinyal dan notifikasi
void processSignals() {
    try {
        while (true) {
            std::unique_lock<std::mutex> lock(serversMutex);

            // Tunggu hingga ada aktivitas baru
            activityNotifier.wait(lock, [] {
                for (const auto& server : servers) {
                    if (!server.second.signals.empty()) return true;
                }
                return false;
            });

            // Proses sinyal untuk setiap server
            for (auto& [serverCode, status] : servers) {
                while (!status.signals.empty()) {
                    OrderSignal signal = status.signals.back();
                    status.signals.pop_back();

                    std::cout << "[CLIENT] Processing signal: " << signal.pair
                              << ", Type: " << (signal.type == 0 ? "BUY" : "SELL")
                              << ", SL: " << signal.sl << ", TP: " << signal.tp
                              << ", Lot: " << signal.lot << std::endl;
                }
            }
        }
    } catch (...) {
        handleException("processSignals");
    }
}

int main() {
    try {
        std::cout << "Starting MirrorTrade.exe (Passive Mode)..." << std::endl;

        // MirrorTrade.exe tidak akan mendaftarkan server secara otomatis.
        // Semua aktivitas hanya akan terjadi jika ada input dari DLL.

        // Jalankan thread untuk memproses sinyal secara pasif
        std::thread signalThread(processSignals);

        // Tunggu thread selesai (tidak akan pernah selesai kecuali ada exit signal)
        signalThread.join();

        return 0;
    } catch (...) {
        handleException("main");
        return 1;
    }
}
