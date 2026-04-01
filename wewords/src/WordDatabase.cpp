#include "WordDatabase.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>

WordDatabase::WordDatabase(QObject* parent) : QObject(parent) {}

// ════════════════════════════════════════════════════
//  loadAll — QRC resource-аас бүгдийг ачаалах
// ════════════════════════════════════════════════════

void WordDatabase::loadAll()
{
    // English
    if (!loadFromFile(":/data/words_en.json", m_englishWords, false))
        loadBuiltinEnglishWords();

    // Chinese — тус тусын HSK файлаас ачаалж нэгдсэн жагсаалт үүсгэнэ
    m_chineseWords.clear();
    for (int lvl = 1; lvl <= 6; ++lvl) {
        const QString path = QString(":/data/words_hsk%1.json").arg(lvl);
        m_hsk[lvl - 1].clear();
        if (loadFromFile(path, m_hsk[lvl - 1], true)) {
            m_chineseWords << m_hsk[lvl - 1];
        }
    }
    if (m_chineseWords.isEmpty())
        loadBuiltinChineseWords();

    qDebug() << "DB loaded — EN:" << m_englishWords.size()
             << "ZH:" << m_chineseWords.size();
}

// ════════════════════════════════════════════════════
//  Generic file loader
// ════════════════════════════════════════════════════

bool WordDatabase::loadFromFile(const QString& path,
                                QVector<Word>& target,
                                bool isChinese)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Cannot open:" << path;
        return false;
    }
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isArray()) {
        qWarning() << "Not a JSON array:" << path;
        return false;
    }
    target.clear();
    for (const QJsonValue& v : doc.array()) {
        if (v.isObject())
            target.append(isChinese ? parseChineseEntry(v.toObject())
                                    : parseEnglishEntry(v.toObject()));
    }
    return true;
}

bool WordDatabase::loadEnglishWords(const QString& filePath)
{
    return loadFromFile(filePath, m_englishWords, false);
}

bool WordDatabase::loadChineseWords(const QString& filePath)
{
    bool ok = loadFromFile(filePath, m_chineseWords, true);
    // section-аар m_hsk[] -г дахин үүсгэнэ
    for (int i = 0; i < 6; ++i) m_hsk[i].clear();
    for (const Word& w : m_chineseWords) {
        int idx = w.section.mid(4).toInt() - 1;   // "HSK 3" → 2
        if (idx >= 0 && idx < 6) m_hsk[idx].append(w);
    }
    return ok;
}

bool WordDatabase::loadHskLevel(int level, const QString& filePath)
{
    if (level < 1 || level > 6) return false;
    return loadFromFile(filePath, m_hsk[level - 1], true);
}

// ════════════════════════════════════════════════════
//  Parse helpers
// ════════════════════════════════════════════════════

Word WordDatabase::parseEnglishEntry(const QJsonObject& obj) const
{
    Word w;
    w.id        = obj["id"].toInt();
    w.english   = obj["en"].toString();
    w.mongolian = obj["mn"].toString();
    w.language  = WordLanguage::English;
    return w;
}

Word WordDatabase::parseChineseEntry(const QJsonObject& obj) const
{
    Word w;
    w.id        = obj["id"].toInt();                          // global id
    w.english   = obj["Hanzi"].toString();
    w.pinyin    = obj["Pinyin"].toString();
    w.mongolian = obj["Mongolian Translation"].toString();
    w.section   = obj["section"].toString();
    w.language  = WordLanguage::Chinese;
    return w;
}

// ════════════════════════════════════════════════════
//  Хандалт
// ════════════════════════════════════════════════════

const QVector<Word>& WordDatabase::words(const QString& lang) const
{
    if (lang == "en") return m_englishWords;
    if (lang == "zh") return m_chineseWords;
    return m_empty;
}

const QVector<Word>& WordDatabase::hskLevel(int n) const
{
    if (n >= 1 && n <= 6) return m_hsk[n - 1];
    return m_empty;
}

int WordDatabase::totalCount(const QString& lang) const
{
    return words(lang).size();
}

int WordDatabase::hskLevelCount(int level) const
{
    return hskLevel(level).size();
}

// ════════════════════════════════════════════════════
//  Хайлт
// ════════════════════════════════════════════════════

QVector<Word> WordDatabase::search(const QString& lang,
                                   const QString& query) const
{
    if (query.isEmpty()) return words(lang);
    const QString q = query.toLower();
    QVector<Word> result;
    for (const Word& w : words(lang)) {
        if (w.english.toLower().contains(q)  ||
            w.mongolian.toLower().contains(q) ||
            w.pinyin.toLower().contains(q))
            result.append(w);
    }
    return result;
}

QVector<Word> WordDatabase::filterBySection(const QVector<Word>& pool,
                                             const QString& section) const
{
    if (section.isEmpty() || section == "all") return pool;
    QVector<Word> result;
    for (const Word& w : pool)
        if (w.section == section) result.append(w);
    return result;
}

// ════════════════════════════════════════════════════
//  Fallback (QRC байхгүй үед)
// ════════════════════════════════════════════════════

void WordDatabase::loadBuiltinEnglishWords()
{
    struct { int id; const char* en; const char* mn; } S[] = {
        {1,"trillion","их наяд"},{2,"anymore","одоо биш"},
        {3,"settle","суурьших"},{4,"debt","өр"},
        {5,"calculation","тооцоолол"},{6,"widespread","өргөн тархсан"},
        {7,"substance","бодис"},{8,"terror","аймшиг"},
        {9,"merely","зөвхөн"},{10,"peculiar","онцгой"},
    };
    m_englishWords.clear();
    for (auto& s : S) {
        Word w; w.id=s.id; w.english=s.en; w.mongolian=s.mn;
        w.language=WordLanguage::English;
        m_englishWords.append(w);
    }
    qWarning() << "Using fallback EN data (" << m_englishWords.size() << "words)";
}

void WordDatabase::loadBuiltinChineseWords()
{
    struct { int id; const char* hz; const char* py;
             const char* mn; const char* sec; } S[] = {
        {1,"开","kāi","нээх","HSK 1"},{2,"十","shí","арав","HSK 1"},
        {3,"菜","cài","хоол","HSK 1"},{4,"好","hǎo","сайн","HSK 1"},
        {5,"家","jiā","гэр","HSK 1"},
    };
    m_chineseWords.clear();
    for (int i=0;i<6;i++) m_hsk[i].clear();
    for (auto& s : S) {
        Word w; w.id=s.id; w.english=s.hz; w.pinyin=s.py;
        w.mongolian=s.mn; w.section=s.sec;
        w.language=WordLanguage::Chinese;
        m_chineseWords.append(w);
        int idx = QString(s.sec).mid(4).toInt()-1;
        if (idx>=0&&idx<6) m_hsk[idx].append(w);
    }
    qWarning() << "Using fallback ZH data (" << m_chineseWords.size() << "words)";
}
