#include "BrowseView.h"
#include "WordCard.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollBar>
#include <QFrame>
#include <QRandomGenerator>
#include <algorithm>

static const QString C_BG     = "#0d0d0d";
static const QString C_CARD   = "#1a1a1a";
static const QString C_BORDER = "#2a2a2a";
static const QString C_GOLD   = "#c9a96e";
static const QString C_TEXT   = "#e8e8e8";
static const QString C_SEC    = "#888888";
static const QString C_MUT    = "#555555";
static const QString C_HOVER  = "#222222";

BrowseView::BrowseView(WordDatabase*  db,
                       UserProgress*  progressEN,
                       UserProgress*  progressZH,
                       AppSettings*   settings,
                       QWidget*       parent)
    : QWidget(parent)
    , m_db(db), m_progressEN(progressEN)
    , m_progressZH(progressZH), m_settings(settings)
{
    setupUi();
}

void BrowseView::setupUi()
{
    setStyleSheet(QString("background:%1;").arg(C_BG));

    QVBoxLayout* root = new QVBoxLayout(this);
    root->setContentsMargins(24, 16, 24, 16);
    root->setSpacing(12);

    // ── HSK level pills (ZH хэлэнд) ──
    m_hskPills = new QWidget;
    QHBoxLayout* pillLay = new QHBoxLayout(m_hskPills);
    pillLay->setContentsMargins(0, 0, 0, 0);
    pillLay->setSpacing(8);

    for (int i = 1; i <= 6; ++i) {
        QPushButton* pill = new QPushButton(QString("HSK %1").arg(i));
        pill->setFixedHeight(28);
        pill->setCursor(Qt::PointingHandCursor);
        pill->setStyleSheet(QString(
            "QPushButton { background:%1; color:%2; border:1px solid %3; "
            "border-radius:14px; padding:0 14px; font-size:11px; font-weight:600; }"
            "QPushButton:hover { border-color:%4; color:%4; }")
            .arg(C_CARD, C_SEC, C_BORDER, C_GOLD));
        const QString sec = QString("HSK %1").arg(i);
        connect(pill, &QPushButton::clicked, this, [this, sec]{
            m_sideFilter = sec;
            applyFilter();
            emit filterChanged(sec);
        });
        pillLay->addWidget(pill);
    }
    pillLay->addStretch();
    root->addWidget(m_hskPills);
    m_hskPills->hide();

    // ── Controls bar ──
    QHBoxLayout* ctrlRow = new QHBoxLayout;
    ctrlRow->setSpacing(10);

    m_search = new QLineEdit;
    m_search->setPlaceholderText("🔍  Search words...");
    m_search->setFixedHeight(36);
    m_search->setStyleSheet(QString(
        "QLineEdit { background:%1; color:%2; border:1px solid %3; "
        "border-radius:6px; padding:0 12px; font-size:13px; }"
        "QLineEdit:focus { border-color:%4; }")
        .arg(C_CARD, C_TEXT, C_BORDER, C_GOLD));
    connect(m_search, &QLineEdit::textChanged, this, &BrowseView::onSearch);
    ctrlRow->addWidget(m_search, 1);

    auto makeCombo = [&](QComboBox*& cb, const QStringList& items) {
        cb = new QComboBox;
        cb->setFixedHeight(36);
        cb->addItems(items);
        cb->setStyleSheet(QString(
            "QComboBox { background:%1; color:%2; border:1px solid %3; "
            "border-radius:6px; padding:0 10px; font-size:12px; min-width:100px; }"
            "QComboBox::drop-down { border:none; width:20px; }"
            "QComboBox QAbstractItemView { background:%1; color:%2; "
            "border:1px solid %3; selection-background-color:%4; }")
            .arg(C_CARD, C_TEXT, C_BORDER, C_HOVER));
    };

    makeCombo(m_filterCmb, {"All", "New", "Hard", "Learned"});
    connect(m_filterCmb, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &BrowseView::onFilterChange);
    ctrlRow->addWidget(m_filterCmb);

    makeCombo(m_sortCmb, {"Original", "A → Z", "Z → A", "Random"});
    connect(m_sortCmb, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &BrowseView::onSortChange);
    ctrlRow->addWidget(m_sortCmb);

    m_shuffleBtn = new QPushButton("🔀");
    m_shuffleBtn->setFixedSize(36, 36);
    m_shuffleBtn->setCursor(Qt::PointingHandCursor);
    m_shuffleBtn->setToolTip("Shuffle");
    m_shuffleBtn->setStyleSheet(QString(
        "QPushButton { background:%1; color:%2; border:1px solid %3; "
        "border-radius:6px; font-size:16px; }"
        "QPushButton:hover { border-color:%4; }")
        .arg(C_CARD, C_TEXT, C_BORDER, C_GOLD));
    connect(m_shuffleBtn, &QPushButton::clicked, this, &BrowseView::onShuffle);
    ctrlRow->addWidget(m_shuffleBtn);

    root->addLayout(ctrlRow);

    // Count info
    m_countInfo = new QLabel("0 words");
    m_countInfo->setStyleSheet(
        QString("color:%1; font-size:11px;").arg(C_SEC));
    root->addWidget(m_countInfo);

    // ── Grid scroll area ──
    m_scroll = new QScrollArea;
    m_scroll->setWidgetResizable(true);
    m_scroll->setFrameShape(QFrame::NoFrame);
    m_scroll->setStyleSheet("QScrollArea { background: transparent; border: none; }");

    m_grid = new QWidget;
    m_grid->setStyleSheet("background: transparent;");
    m_gridLay = new QGridLayout(m_grid);
    m_gridLay->setContentsMargins(0, 0, 0, 0);
    m_gridLay->setSpacing(10);

    m_scroll->setWidget(m_grid);
    root->addWidget(m_scroll, 1);

    // ── Pager ──
    QWidget* pager = new QWidget;
    QHBoxLayout* pageLay = new QHBoxLayout(pager);
    pageLay->setContentsMargins(0, 4, 0, 4);
    pageLay->setSpacing(10);

    m_prevBtn = new QPushButton("← Prev");
    m_nextBtn = new QPushButton("Next →");
    for (auto* b : {m_prevBtn, m_nextBtn}) {
        b->setFixedHeight(32);
        b->setCursor(Qt::PointingHandCursor);
        b->setStyleSheet(QString(
            "QPushButton { background:%1; color:%2; border:1px solid %3; "
            "border-radius:6px; padding:0 16px; font-size:12px; }"
            "QPushButton:hover { border-color:%4; }"
            "QPushButton:disabled { color:%5; }")
            .arg(C_CARD, C_TEXT, C_BORDER, C_GOLD, C_MUT));
    }
    connect(m_prevBtn, &QPushButton::clicked, this, &BrowseView::onPrevPage);
    connect(m_nextBtn, &QPushButton::clicked, this, &BrowseView::onNextPage);

    m_pageInfo = new QLabel("Page 1");
    m_pageInfo->setAlignment(Qt::AlignCenter);
    m_pageInfo->setStyleSheet(
        QString("color:%1; font-size:12px;").arg(C_SEC));

    pageLay->addWidget(m_prevBtn);
    pageLay->addStretch();
    pageLay->addWidget(m_pageInfo);
    pageLay->addStretch();
    pageLay->addWidget(m_nextBtn);
    root->addWidget(pager);
}

// ════════════════════════════════════════════════════
//  Refresh (MainWindow-оос дуудагдана)
// ════════════════════════════════════════════════════

void BrowseView::refresh(const QString& lang, const QString& filter)
{
    m_lang       = lang;
    m_sideFilter = filter;
    m_page       = 1;

    // HSK pills зөвхөн ZH хэлэнд
    m_hskPills->setVisible(lang == "zh");

    // Filter combo-г sidebar filter-тэй синк хийх
    if      (filter == "new")     m_filterCmb->setCurrentIndex(1);
    else if (filter == "hard")    m_filterCmb->setCurrentIndex(2);
    else if (filter == "learned") m_filterCmb->setCurrentIndex(3);
    else                          m_filterCmb->setCurrentIndex(0);

    m_all = m_db->words(lang);
    applyFilter();
}

// ════════════════════════════════════════════════════
//  Шүүлт + эрэмбэлэлт
// ════════════════════════════════════════════════════

void BrowseView::applyFilter()
{
    UserProgress* prog = progress();
    m_filtered.clear();

    for (const Word& w : m_all) {
        // Search
        if (!m_searchQ.isEmpty()) {
            const QString q = m_searchQ.toLower();
            if (!w.english.toLower().contains(q) &&
                !w.mongolian.toLower().contains(q) &&
                !w.pinyin.toLower().contains(q)) continue;
        }

        // Status filter
        const int fi = m_filterCmb->currentIndex();
        if (fi == 1 && !prog->isNew(w.id))     continue;
        if (fi == 2 && !prog->isHard(w.id))    continue;
        if (fi == 3 && !prog->isLearned(w.id)) continue;

        // Section filter (HSK level sidebar)
        if (!m_sideFilter.isEmpty() && m_sideFilter != "all" &&
            m_sideFilter != "new" && m_sideFilter != "hard" &&
            m_sideFilter != "learned") {
            if (w.section != m_sideFilter) continue;
        }

        // Status from progress
        Word mw = w;
        if      (prog->isLearned(w.id)) mw.status = WordStatus::Learned;
        else if (prog->isHard(w.id))    mw.status = WordStatus::Hard;
        else                             mw.status = WordStatus::New;

        m_filtered.append(mw);
    }

    // Эрэмбэлэлт
    const int si = m_sortCmb->currentIndex();
    if (si == 1) std::sort(m_filtered.begin(), m_filtered.end(),
        [](const Word& a, const Word& b){ return a.english < b.english; });
    else if (si == 2) std::sort(m_filtered.begin(), m_filtered.end(),
        [](const Word& a, const Word& b){ return a.english > b.english; });
    else if (si == 3) {
        for (int i = m_filtered.size() - 1; i > 0; --i) {
            int j = QRandomGenerator::global()->bounded(i + 1);
            std::swap(m_filtered[i], m_filtered[j]);
        }
    }

    m_countInfo->setText(
        QString("%1 words").arg(m_filtered.size()));
    m_page = 1;
    renderGrid();
    renderPager();
}

// ════════════════════════════════════════════════════
//  Grid дүрслэх
// ════════════════════════════════════════════════════

void BrowseView::renderGrid()
{
    // Хуучин widget-уудыг устгах
    while (QLayoutItem* item = m_gridLay->takeAt(0)) {
        delete item->widget();
        delete item;
    }

    const int start = (m_page - 1) * PAGE_SIZE;
    const int end   = qMin(start + PAGE_SIZE, m_filtered.size());
    const int COLS  = 3;

    for (int i = start; i < end; ++i) {
        WordCardWidget* card = new WordCardWidget(m_filtered[i]);
        connect(card, &WordCardWidget::clicked, this, [](const Word&){});
        int row = (i - start) / COLS;
        int col = (i - start) % COLS;
        m_gridLay->addWidget(card, row, col);
    }

    // Grid column stretch
    for (int c = 0; c < COLS; ++c)
        m_gridLay->setColumnStretch(c, 1);

    m_scroll->verticalScrollBar()->setValue(0);
}

void BrowseView::renderPager()
{
    const int total = (m_filtered.size() + PAGE_SIZE - 1) / PAGE_SIZE;
    m_pageInfo->setText(
        QString("Page %1 / %2").arg(m_page).arg(qMax(1, total)));
    m_prevBtn->setEnabled(m_page > 1);
    m_nextBtn->setEnabled(m_page < total);
}

// ════════════════════════════════════════════════════
//  Slots
// ════════════════════════════════════════════════════

void BrowseView::onSearch(const QString& q)
{
    m_searchQ = q;
    m_page = 1;
    applyFilter();
}

void BrowseView::onFilterChange(int)
{
    m_page = 1;
    applyFilter();
}

void BrowseView::onSortChange(int)
{
    m_page = 1;
    applyFilter();
}

void BrowseView::onShuffle()
{
    m_sortCmb->setCurrentIndex(3);   // "Random"
    applyFilter();
}

void BrowseView::onPrevPage()
{
    if (m_page > 1) { m_page--; renderGrid(); renderPager(); }
}

void BrowseView::onNextPage()
{
    const int total = (m_filtered.size() + PAGE_SIZE - 1) / PAGE_SIZE;
    if (m_page < total) { m_page++; renderGrid(); renderPager(); }
}
