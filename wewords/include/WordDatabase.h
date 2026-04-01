#pragma once
#include <QObject>
#include <QVector>
#include <QString>
#include <QStringList>
#include <QJsonObject>
#include "Word.h"

class WordDatabase : public QObject
{
    Q_OBJECT
public:
    explicit WordDatabase(QObject* parent = nullptr);

    void loadAll();
    bool loadEnglishWords(const QString& filePath);
    bool loadChineseWords(const QString& filePath);
    bool loadHskLevel(int level, const QString& filePath);

    const QVector<Word>& englishWords()  const { return m_englishWords; }
    const QVector<Word>& chineseWords()  const { return m_chineseWords; }
    const QVector<Word>& hskLevel(int n) const;
    const QVector<Word>& words(const QString& lang) const;

    int totalCount(const QString& lang) const;
    int hskLevelCount(int level) const;

    static QStringList hskSections() {
        return {"HSK 1","HSK 2","HSK 3","HSK 4","HSK 5","HSK 6"};
    }

    QVector<Word> search(const QString& lang, const QString& query) const;
    QVector<Word> filterBySection(const QVector<Word>& pool,
                                  const QString& section) const;

    void loadBuiltinEnglishWords();
    void loadBuiltinChineseWords();

private:
    QVector<Word>  m_englishWords;
    QVector<Word>  m_chineseWords;
    QVector<Word>  m_hsk[6];
    QVector<Word>  m_empty;

    Word parseEnglishEntry(const QJsonObject& obj) const;
    Word parseChineseEntry(const QJsonObject& obj) const;
    bool loadFromFile(const QString& path, QVector<Word>& target, bool isChinese);
};
