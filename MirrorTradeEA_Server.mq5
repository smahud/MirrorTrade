//+------------------------------------------------------------------+
//|                                         MirrorTradeEA_Server.mq5 |
//|              Huda Choirul Anam - Copyright 2025, MetaQuotes Ltd. |
//|                                        https://www.ulfasanda.com |
//+------------------------------------------------------------------+

#import "mirrortrade.dll"
    bool RegisterServer(string kodeserver);
    bool UnregisterServer(string kodeserver);
    void SendOrderSignal(string kodeserver, string pair, int type, double sl, double tp, double lot);
#import

input string ServerCode = "SRV01"; // Server identifier

// Variable untuk melacak posisi terakhir yang diproses
datetime lastProcessedTime = 0;

int OnInit() {
    // Daftarkan server
    if (RegisterServer(ServerCode)) {
        Print("Server registered: ", ServerCode);
        Comment("Server registered: ", ServerCode);

        // Inisialisasi lastProcessedTime dengan waktu order terbaru
        int totalPositions = PositionsTotal();
        for (int i = 0; i < totalPositions; i++) {
            if (PositionSelect(PositionGetSymbol(i))) { // Pilih posisi berdasarkan simbol
                datetime openTime = (datetime)PositionGetInteger(POSITION_TIME);
                if (openTime > lastProcessedTime) {
                    lastProcessedTime = openTime; // Update waktu proses terakhir
                }
            } else {
                Print("Failed to select position for index: ", i); // Debugging jika ada kegagalan
            }
        }
        Print("Initialization complete. Last processed time set to: ", TimeToString(lastProcessedTime, TIME_DATE | TIME_MINUTES | TIME_SECONDS));
    } else {
        Print("Failed to register server: ", ServerCode);
        Comment("Failed to register server: ", ServerCode);
    }
    return INIT_SUCCEEDED;
}

void OnDeinit(const int reason) {
    // Hapus server saat EA dihentikan
    UnregisterServer(ServerCode);
    Comment(""); // Hapus komentar dari layar saat EA dihentikan
}

// Fungsi untuk memantau posisi baru
void CheckNewPositions() {
    int totalPositions = PositionsTotal(); // Total posisi terbuka

    // Tampilkan status di layar terminal trading
    Comment("Server Code: ", ServerCode, "\n",
            "Total open positions: ", totalPositions);

    for (int i = 0; i < totalPositions; i++) {
        string symbol = PositionGetSymbol(i); // Ambil simbol posisi
        if (PositionSelect(symbol)) { // Pilih posisi berdasarkan simbol
            datetime openTime = (datetime)PositionGetInteger(POSITION_TIME); // Ambil waktu posisi dibuka

            // Jika posisi baru ditemukan
            if (openTime > lastProcessedTime) {
                lastProcessedTime = openTime; // Update waktu proses terakhir

                // Ambil detail posisi
                string pair = PositionGetString(POSITION_SYMBOL);
                int type = (int)PositionGetInteger(POSITION_TYPE); // 0 = BUY, 1 = SELL
                double sl = PositionGetDouble(POSITION_SL);
                double tp = PositionGetDouble(POSITION_TP);
                double lot = PositionGetDouble(POSITION_VOLUME);

                // Kirim sinyal ke client
                SendOrderSignal(ServerCode, pair, type, sl, tp, lot);
                PrintFormat("New position sent: %s, Type: %d, SL: %.5f, TP: %.5f, Lot: %.2f", pair, type, sl, tp, lot);
                Comment("Server Code: ", ServerCode, "\n",
                        "Total open positions: ", totalPositions, "\n",
                        "Last position sent: \n",
                        "Pair: ", pair, ", Type: ", (type == 0 ? "BUY" : "SELL"), ", Lot: ", lot, ", SL: ", sl, ", TP: ", tp);
            }
        } else {
            Print("Failed to select position for index: ", i); // Debugging jika ada kegagalan
        }
    }
}

void OnTick() {
    CheckNewPositions(); // Periksa posisi baru setiap tick
}
