#pragma once
#include <QObject>
#include <QHash>
#include <QVector>
#include <QString>
#include <QDate>
#include "Word.h"

// ── Хэрэглэгчийн явц ──
// HTML дэх db объекттой тохирно.
// QSettings (Windows Registry) эсвэл JSON файлд хадгалагдана.

class UserProgress : public QObject
{
    Q_OBJECT

public:
    explicit UserProgress(const QString& lang, QObject* parent = nullptr);

    // Үгийн төлөв тохируулах
    void    markLearned(int wordId);
    void    markHard   (int wordId);
    void    markNew    (int wordId);   // hard/learned-ыг арилгана

    // Үгийн төлөв унших
    bool    isLearned  (int wordId) const;
    bool    isHard     (int wordId) const;
    bool    isNew      (int wordId) const;
    WordStatus statusOf(int wordId) const;

    // Өнөөдрийн статистик
    int     todayCount ()           const;
    bool    isTodayLearned(int id)  const;
    void    addTodayLearned(int id);

    // Өдөр тутмын түүх
    QHash<QString, int>  dailyHistory()     const { return m_dailyHistory; }
    int                  historyFor(const QDate& d) const;

    // Streak тооцоолол (HTML computeStreak()-тай адил)
    int     streakDays()  const;

    // Нийт тоо
    int     learnedCount() const;
    int     hardCount()    const;

    // Хадгалах / ачаалах
    bool    save()   const;
    bool    load();

    // Хэлийг шилжүүлэхэд progress reset хийхгүй,
    // зүгээр өөр хэлний объект үүсгэнэ
    QString language() const { return m_lang; }

    // Бүх явцыг устгах (settings-ийн Reset товч)
    void    resetAll();
    void    resetLearned();

signals:
    void progressChanged();

private:
    void    checkDailyReset();
    QString settingsKey()  const;
    QString todayString()  const;
    QString yesterdayString() const;

    QString             m_lang;
    QHash<int, qint64>  m_learned;         // id → timestamp
    QHash<int, qint64>  m_hard;            // id → timestamp
    QVector<int>        m_todayLearned;    // өнөөдрийн session
    QString             m_lastDate;        // "yyyy-MM-dd"
    QHash<QString, int> m_dailyHistory;    // "yyyy-MM-dd" → count
};
