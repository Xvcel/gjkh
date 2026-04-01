#pragma once
#include <QString>

// ── Апп-ын тохиргоо ──
// HTML дэх settings объекттой тохирно

struct AppSettings {
    bool    reverseCards    = false;    // Карт урвуулах (MN→EN)
    bool    autoAdvance     = true;     // Дараагийн карт автоматаар орох
    int     sessionSize     = 20;       // Нэг session-д хэдэн үг
    int     dailyGoal       = 28;       // Өдрийн зорилт

    // Нэмэлт тохиргоо (HTML-д байгаагүй, C++ хувилбарт нэмэв)
    bool    showPinyin      = true;     // HSK: pinyin харуулах
    int     autoFlipDelay   = 3000;     // Auto-flip хугацаа (ms)
    QString currentLanguage = "en";     // "en" | "zh"
};
