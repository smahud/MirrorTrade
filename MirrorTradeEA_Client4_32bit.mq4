//+------------------------------------------------------------------+
//|                                         MirrorTradeEA_Client4_32bit.mq4 |
//|              Huda Choirul Anam - Copyright 2025, MetaQuotes Ltd. |
//|                                        https://www.ulfasanda.com |
//+------------------------------------------------------------------+

#import "mirrortrade32.dll" // 32-bit DLL
    bool CheckServerStatus(string kodeserver);
    bool GetLastSentSignal(string kodeserver, string &pair, int &type, double &sl, double &tp, double &lot);
#import

input string ServerCode = "SRV01"; // Server code
input double LotMultiplier = 1.0;  // Faktor pengali lot

int OnInit() {
    Print("Using 32-bit DLL: mirrortrade32.dll");

    if (CheckServerStatus(ServerCode)) {
        Print("Connected to server: ", ServerCode);
    } else {
        Print("Error: Server not active: ", ServerCode);
    }

    return INIT_SUCCEEDED;
}

void OnTick() {
    string pair;
    int type;
    double sl, tp, lot;

    if (!CheckServerStatus(ServerCode)) {
        Print("Error: Server not active. Waiting for connection...");
        return;
    }

    if (GetLastSentSignal(ServerCode, pair, type, sl, tp, lot)) {
        int digits = MarketInfo(pair, MODE_DIGITS);
        double multiplier = (digits == 4) ? 0.1 : 1.0;

        // Sesuaikan harga
        sl *= multiplier;
        tp *= multiplier;

        // Kirim order
        int ticket = OrderSend(pair, type, lot, MarketInfo(pair, MODE_ASK), 3, sl, tp);
        if (ticket < 0) {
            Print("Order failed: ", GetLastError());
        }
    }
}
