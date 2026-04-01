#pragma once
#include <QWidget>
#include <QVector>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QScrollArea>
#include <QLabel>
#include <QGridLayout>

#include "Word.h"
#include "WordDatabase.h"
#include "UserProgress.h"
#include "AppSettings.h"

class WordCardWidget;

class BrowseView : public QWidget
{
    Q_OBJECT
public:
    explicit BrowseView(WordDatabase*  db,
                        UserProgress*  progressEN,
                        UserProgress*  progressZH,
                        AppSettings*   settings,
                        QWidget*       parent = nullptr);

    void refresh(const QString& lang, const QString& filter);

signals:
    void filterChanged(const QString& filter);

private slots:
    void onSearch(const QString& q);
    void onFilterChange(int idx);
    void onSortChange(int idx);
    void onShuffle();
    void onPrevPage();
    void onNextPage();

private:
    void setupUi();
    void applyFilter();
    void renderGrid();
    void renderPager();

    // Data
    WordDatabase*  m_db         = nullptr;
    UserProgress*  m_progressEN = nullptr;
    UserProgress*  m_progressZH = nullptr;
    AppSettings*   m_settings   = nullptr;
    QString        m_lang       = "en";
    QString        m_sideFilter = "all";
    QString        m_searchQ    = "";
    QString        m_sortVal    = "original";
    int            m_page       = 1;
    static const int PAGE_SIZE  = 60;

    QVector<Word>  m_all;
    QVector<Word>  m_filtered;

    UserProgress* progress() const {
        return m_lang == "en" ? m_progressEN : m_progressZH;
    }

    // UI
    QWidget*       m_hskPills   = nullptr;
    QLineEdit*     m_search     = nullptr;
    QComboBox*     m_filterCmb  = nullptr;
    QComboBox*     m_sortCmb    = nullptr;
    QPushButton*   m_shuffleBtn = nullptr;
    QScrollArea*   m_scroll     = nullptr;
    QWidget*       m_grid       = nullptr;
    QGridLayout*   m_gridLay    = nullptr;
    QPushButton*   m_prevBtn    = nullptr;
    QPushButton*   m_nextBtn    = nullptr;
    QLabel*        m_pageInfo   = nullptr;
    QLabel*        m_countInfo  = nullptr;
};
