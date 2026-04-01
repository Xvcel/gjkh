#pragma once
#include <QString>

// ── WeWords үндсэн өгөгдлийн бүтэц ──
// HTML дэх ENG_DATA болон HSK_DATA-г тусгасан

enum class WordStatus {
    New,        // Шинэ үг
    Hard,       // Хүнд үг
    Learned     // Сурсан үг
};

enum class WordLanguage {
    English,    // Oxford 5000 (EN → MN)
    Chinese     // HSK 1-6    (ZH → MN)
};

struct Word {
    int         id          = 0;
    QString     english     = "";   // EN: English үг | ZH: Ханз
    QString     mongolian   = "";   // Монгол орчуулга (хоёр хэлэнд адил)
    QString     pinyin      = "";   // Зөвхөн HSK үгэнд
    QString     section     = "";   // Зөвхөн HSK: "HSK 1" .. "HSK 6"
    WordLanguage language   = WordLanguage::English;

    // Тооцоолсон талбар (UserProgress-аас ирнэ)
    WordStatus  status      = WordStatus::New;

    // Helper методууд
    bool isNew()     const { return status == WordStatus::New;     }
    bool isHard()    const { return status == WordStatus::Hard;    }
    bool isLearned() const { return status == WordStatus::Learned; }
    bool isChinese() const { return language == WordLanguage::Chinese; }
};
