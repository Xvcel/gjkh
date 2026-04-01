#include "UserProgress.h"
#include <QSettings>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDate>
#include <QDebug>

// ════════════════════════════════════════════════════
//  UserProgress — Constructor
// ════════════════════════════════════════════════════

UserProgress::UserProgress(const QString& lang, QObject* parent)
    : QObject(parent)
    , m_lang(lang)
{
    load();
    checkDailyReset();
}

// ════════════════════════════════════════════════════
//  Тохиргоо түлхүүр
//  Windows Registry: HKCU\Software\WeWords\progress_en
// ════════════════════════════════════════════════════

QString UserProgress::settingsKey() const
{
    return "progress_" + m_lang;
}

// ════════════════════════════════════════════════════
//  Өдрийн тэмдэглэл
// ════════════════════════════════════════════════════

QString UserProgress::todayString() const
{
    return QDate::currentDate().toString(Qt::ISODate);
}

QString UserProgress::yesterdayString() const
{
    return QDate::currentDate().addDays(-1).toString(Qt::ISODate);
}

// ════════════════════════════════════════════════════
//  Өдрийн reset шалгалт
//  HTML-ийн checkDailyReset()-тай тохирно
// ════════════════════════════════════════════════════

void UserProgress::checkDailyReset()
{
    const QString today = todayString();
    if (m_lastDate == today) return;

    // Өмнөх өдрийн today-ийг learned руу шилжүүлнэ
    if (!m_lastDate.isEmpty() && !m_todayLearned.isEmpty()) {
        m_dailyHistory[m_lastDate] = m_todayLearned.size();
        for (int id : m_todayLearned) {
            m_learned[id] = QDateTime::currentMSecsSinceEpoch();
        }
    }
    m_todayLearned.clear();
    m_lastDate = today;
    save();
}

// ════════════════════════════════════════════════════
//  Үгийн төлөв тохируулах
// ════════════════════════════════════════════════════

void UserProgress::markLearned(int wordId)
{
    m_hard.remove(wordId);
    if (!m_todayLearned.contains(wordId))
        m_todayLearned.append(wordId);
    save();
    emit progressChanged();
}

void UserProgress::markHard(int wordId)
{
    m_hard[wordId] = QDateTime::currentMSecsSinceEpoch();
    m_todayLearned.removeAll(wordId);
    m_learned.remove(wordId);
    save();
    emit progressChanged();
}

void UserProgress::markNew(int wordId)
{
    m_hard.remove(wordId);
    m_learned.remove(wordId);
    m_todayLearned.removeAll(wordId);
    save();
    emit progressChanged();
}

// ════════════════════════════════════════════════════
//  Үгийн төлөв унших
// ════════════════════════════════════════════════════

bool UserProgress::isLearned(int wordId) const
{
    return m_learned.contains(wordId) || m_todayLearned.contains(wordId);
}

bool UserProgress::isHard(int wordId) const
{
    return m_hard.contains(wordId);
}

bool UserProgress::isNew(int wordId) const
{
    return !isLearned(wordId) && !isHard(wordId);
}

WordStatus UserProgress::statusOf(int wordId) const
{
    if (isLearned(wordId)) return WordStatus::Learned;
    if (isHard(wordId))    return WordStatus::Hard;
    return WordStatus::New;
}

// ════════════════════════════════════════════════════
//  Өнөөдрийн статистик
// ════════════════════════════════════════════════════

int UserProgress::todayCount() const
{
    return m_todayLearned.size();
}

bool UserProgress::isTodayLearned(int id) const
{
    return m_todayLearned.contains(id);
}

void UserProgress::addTodayLearned(int id)
{
    if (!m_todayLearned.contains(id))
        m_todayLearned.append(id);
    save();
}

// ════════════════════════════════════════════════════
//  Нийт тоо
// ════════════════════════════════════════════════════

int UserProgress::learnedCount() const
{
    // today-learned-с зөвхөн learned-д ороогүй нь тоологдоно
    int todayOnly = 0;
    for (int id : m_todayLearned)
        if (!m_learned.contains(id)) todayOnly++;
    return m_learned.size() + todayOnly;
}

int UserProgress::hardCount() const
{
    return m_hard.size();
}

// ════════════════════════════════════════════════════
//  Өдрийн түүх
// ════════════════════════════════════════════════════

int UserProgress::historyFor(const QDate& d) const
{
    const QString key = d.toString(Qt::ISODate);
    return m_dailyHistory.value(key, 0);
}

// ════════════════════════════════════════════════════
//  Streak тооцоолол
//  HTML-ийн computeStreak()-тай тохирно
// ════════════════════════════════════════════════════

int UserProgress::streakDays() const
{
    int streak = 0;
    QDate d = QDate::currentDate();

    // Өнөөдөр ямар нэг үг сурсан бол streak = 1
    if (todayCount() > 0) {
        streak = 1;
        d = d.addDays(-1);
    }

    // Өмнөх өдрүүдийг шалгана
    for (int i = 0; i < 366; i++) {
        const QString key = d.toString(Qt::ISODate);
        if (m_dailyHistory.value(key, 0) > 0) {
            streak++;
            d = d.addDays(-1);
        } else {
            break;
        }
    }
    return streak;
}

// ════════════════════════════════════════════════════
//  Reset
// ════════════════════════════════════════════════════

void UserProgress::resetAll()
{
    m_learned.clear();
    m_hard.clear();
    m_todayLearned.clear();
    m_lastDate = todayString();
    m_dailyHistory.clear();
    save();
    emit progressChanged();
}

void UserProgress::resetLearned()
{
    m_learned.clear();
    m_todayLearned.clear();
    save();
    emit progressChanged();
}

// ════════════════════════════════════════════════════
//  Хадгалах — QSettings (Windows Registry)
// ════════════════════════════════════════════════════

bool UserProgress::save() const
{
    QSettings settings("WeWords", "WeWords");
    settings.beginGroup(settingsKey());

    // learned: "id:timestamp,id:timestamp,..."
    QStringList learnedList;
    for (auto it = m_learned.begin(); it != m_learned.end(); ++it)
        learnedList << QString("%1:%2").arg(it.key()).arg(it.value());
    settings.setValue("learned", learnedList.join(','));

    // hard
    QStringList hardList;
    for (auto it = m_hard.begin(); it != m_hard.end(); ++it)
        hardList << QString("%1:%2").arg(it.key()).arg(it.value());
    settings.setValue("hard", hardList.join(','));

    // todayLearned
    QStringList todayList;
    for (int id : m_todayLearned) todayList << QString::number(id);
    settings.setValue("todayLearned", todayList.join(','));

    settings.setValue("lastDate", m_lastDate);

    // dailyHistory: JSON object
    QJsonObject histObj;
    for (auto it = m_dailyHistory.begin(); it != m_dailyHistory.end(); ++it)
        histObj[it.key()] = it.value();
    settings.setValue("dailyHistory",
                       QString::fromUtf8(
                           QJsonDocument(histObj).toJson(QJsonDocument::Compact)));

    settings.endGroup();
    return true;
}

// ════════════════════════════════════════════════════
//  Ачаалах — QSettings
// ════════════════════════════════════════════════════

bool UserProgress::load()
{
    QSettings settings("WeWords", "WeWords");
    settings.beginGroup(settingsKey());

    // learned
    m_learned.clear();
    const QString learnedStr = settings.value("learned").toString();
    if (!learnedStr.isEmpty()) {
        for (const QString& part : learnedStr.split(',')) {
            const QStringList kv = part.split(':');
            if (kv.size() == 2)
                m_learned[kv[0].toInt()] = kv[1].toLongLong();
        }
    }

    // hard
    m_hard.clear();
    const QString hardStr = settings.value("hard").toString();
    if (!hardStr.isEmpty()) {
        for (const QString& part : hardStr.split(',')) {
            const QStringList kv = part.split(':');
            if (kv.size() == 2)
                m_hard[kv[0].toInt()] = kv[1].toLongLong();
        }
    }

    // todayLearned
    m_todayLearned.clear();
    const QString todayStr = settings.value("todayLearned").toString();
    if (!todayStr.isEmpty()) {
        for (const QString& part : todayStr.split(',')) {
            const int id = part.toInt();
            if (id > 0) m_todayLearned.append(id);
        }
    }

    m_lastDate = settings.value("lastDate").toString();

    // dailyHistory
    m_dailyHistory.clear();
    const QString histStr = settings.value("dailyHistory").toString();
    if (!histStr.isEmpty()) {
        const QJsonDocument doc =
            QJsonDocument::fromJson(histStr.toUtf8());
        if (doc.isObject()) {
            const QJsonObject obj = doc.object();
            for (auto it = obj.begin(); it != obj.end(); ++it)
                m_dailyHistory[it.key()] = it.value().toInt();
        }
    }

    settings.endGroup();
    qDebug() << "Progress loaded for" << m_lang
             << "| learned:" << m_learned.size()
             << "| hard:"    << m_hard.size()
             << "| today:"   << m_todayLearned.size();
    return true;
}
