#include "MainWindow.h"
#include "FlashcardView.h"
#include "BrowseView.h"
#include "StatsView.h"
#include "SettingsDialog.h"

#include <QApplication>
#include <QEvent>
#include <QCloseEvent>
#include <QResizeEvent>
#include <QScrollArea>
#include <QFrame>
#include <QSpacerItem>
#include <QTimer>

// ════════════════════════════════════════════════════
//  Өнгө + хэмжээ тогтмол
// ════════════════════════════════════════════════════

namespace Style {
    // HTML CSS variable-уудтай тохирно
    const QString BG_DARK     = "#0d0d0d";
    const QString BG_SIDEBAR  = "#111111";
    const QString BG_CARD     = "#1a1a1a";
    const QString BG_HOVER    = "#222222";
    const QString GOLD        = "#c9a96e";
    const QString GOLD_DIM    = "#a07840";
    const QString TEXT_PRI    = "#e8e8e8";
    const QString TEXT_SEC    = "#888888";
    const QString TEXT_MUT    = "#555555";
    const QString BORDER      = "#2a2a2a";
    const QString GREEN       = "#4a7c59";
    const QString RED         = "#8b3a3a";
    const QString BLUE        = "#3a5a8b";

    const int     SIDEBAR_W   = 220;
    const int     TOPBAR_H    = 56;
    const int     RADIUS      = 8;
}

// ════════════════════════════════════════════════════
//  Constructor
// ════════════════════════════════════════════════════

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent)
{
    setWindowTitle("WeWords");
    setMinimumSize(900, 600);
    resize(1200, 750);

    // ── Өгөгдөл ──
    m_db = new WordDatabase(this);
    m_db->loadAll();

    m_progressEN = new UserProgress("en", this);
    m_progressZH = new UserProgress("zh", this);

    // ── UI ──
    setupStyles();
    setupUi();

    // ── Дохио холбох ──
    connect(m_progressEN, &UserProgress::progressChanged,
            this, &MainWindow::onProgressChanged);
    connect(m_progressZH, &UserProgress::progressChanged,
            this, &MainWindow::onProgressChanged);

    // ── Анхны харагдал ──
    navigateTo("home");
    updateSidebarStats();
}

// ════════════════════════════════════════════════════
//  Глобал stylesheet
// ════════════════════════════════════════════════════

void MainWindow::setupStyles()
{
    qApp->setStyleSheet(QString(R"(
        QMainWindow, QWidget {
            background: %1;
            color: %2;
            font-family: 'Segoe UI', Arial, sans-serif;
            font-size: 13px;
        }
        QScrollBar:vertical {
            background: %3; width: 6px; border-radius: 3px;
        }
        QScrollBar::handle:vertical {
            background: %4; border-radius: 3px; min-height: 20px;
        }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }
        QScrollBar:horizontal {
            background: %3; height: 6px; border-radius: 3px;
        }
        QScrollBar::handle:horizontal {
            background: %4; border-radius: 3px;
        }
        QToolTip {
            background: %5; color: %2; border: 1px solid %6;
            padding: 4px 8px; border-radius: 4px;
        }
    )").arg(Style::BG_DARK, Style::TEXT_PRI, Style::BG_DARK,
            Style::TEXT_MUT, Style::BG_CARD, Style::BORDER));
}

// ════════════════════════════════════════════════════
//  UI байгуулалт
// ════════════════════════════════════════════════════

void MainWindow::setupUi()
{
    QWidget* central = new QWidget(this);
    setCentralWidget(central);

    QHBoxLayout* root = new QHBoxLayout(central);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    // Sidebar
    m_sidebar = buildSidebar();
    root->addWidget(m_sidebar);

    // Right: topbar + content
    QWidget* rightPane = new QWidget;
    QVBoxLayout* rightLayout = new QVBoxLayout(rightPane);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(0);

    setupTopbar();
    rightLayout->addWidget(m_topbar);

    setupContentArea();
    rightLayout->addWidget(m_stack, 1);

    root->addWidget(rightPane, 1);
}

// ════════════════════════════════════════════════════
//  Sidebar
// ════════════════════════════════════════════════════

QWidget* MainWindow::buildSidebar()
{
    QWidget* sb = new QWidget;
    sb->setFixedWidth(Style::SIDEBAR_W);
    sb->setStyleSheet(QString("background:%1; border-right:1px solid %2;")
                      .arg(Style::BG_SIDEBAR, Style::BORDER));

    QVBoxLayout* lay = new QVBoxLayout(sb);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(0);

    // ── Logo / Хэл солих ──
    lay->addWidget(buildSidebarLogo());

    // ── Навигаци ──
    lay->addWidget(buildSidebarNav());

    // ── Шүүлт товчнууд ──
    lay->addWidget(buildSidebarFilters());

    lay->addStretch();

    // ── Явц харуулах ──
    lay->addWidget(buildSidebarStats());

    return sb;
}

QWidget* MainWindow::buildSidebarLogo()
{
    QWidget* w = new QWidget;
    w->setFixedHeight(64);
    w->setStyleSheet(QString("border-bottom:1px solid %1;").arg(Style::BORDER));

    QHBoxLayout* lay = new QHBoxLayout(w);
    lay->setContentsMargins(16, 0, 16, 0);

    // Лого текст
    QLabel* logo = new QLabel("📚 WeWords");
    logo->setStyleSheet(QString("color:%1; font-size:16px; font-weight:700;")
                        .arg(Style::GOLD));
    lay->addWidget(logo, 1);

    // EN / ZH товч
    m_btnLangEN = new QPushButton("EN");
    m_btnLangZH = new QPushButton("ZH");
    for (auto* btn : {m_btnLangEN, m_btnLangZH}) {
        btn->setFixedSize(32, 24);
        btn->setCursor(Qt::PointingHandCursor);
    }
    auto langBtnStyle = [](bool active) {
        return QString("QPushButton { background:%1; color:%2; "
                        "border:1px solid %3; border-radius:4px; font-size:11px; font-weight:600; }"
                        "QPushButton:hover { background:%4; }")
            .arg(active ? Style::GOLD : "transparent",
                 active ? Style::BG_DARK : Style::TEXT_SEC,
                 active ? Style::GOLD : Style::BORDER,
                 active ? Style::GOLD : Style::BG_HOVER);
    };
    m_btnLangEN->setStyleSheet(langBtnStyle(true));
    m_btnLangZH->setStyleSheet(langBtnStyle(false));

    connect(m_btnLangEN, &QPushButton::clicked, this, [this]{ onLanguageChanged("en"); });
    connect(m_btnLangZH, &QPushButton::clicked, this, [this]{ onLanguageChanged("zh"); });

    lay->addWidget(m_btnLangEN);
    lay->addSpacing(4);
    lay->addWidget(m_btnLangZH);

    return w;
}

QWidget* MainWindow::buildSidebarNav()
{
    QWidget* w = new QWidget;
    QVBoxLayout* lay = new QVBoxLayout(w);
    lay->setContentsMargins(8, 12, 8, 4);
    lay->setSpacing(2);

    QLabel* sec = new QLabel("NAVIGATE");
    sec->setStyleSheet(QString("color:%1; font-size:10px; font-weight:700; "
                                "letter-spacing:1px; padding:0 8px 6px 8px;")
                        .arg(Style::TEXT_MUT));
    lay->addWidget(sec);

    m_navHome   = makeSidebarNavBtn("🏠", "Home",       "home");
    m_navFlash  = makeSidebarNavBtn("⚡", "Flashcards", "flash");
    m_navBrowse = makeSidebarNavBtn("📖", "Browse",     "browse");
    m_navReview = makeSidebarNavBtn("🔁", "Review Hard","review");
    m_navStats  = makeSidebarNavBtn("📊", "Statistics", "stats");

    for (auto* btn : {m_navHome, m_navFlash, m_navBrowse,
                       m_navReview, m_navStats})
        lay->addWidget(btn);

    return w;
}

QPushButton* MainWindow::makeSidebarNavBtn(const QString& icon,
                                            const QString& label,
                                            const QString& viewName)
{
    auto* btn = new QPushButton(icon + "  " + label);
    btn->setFixedHeight(36);
    btn->setCheckable(true);
    btn->setCursor(Qt::PointingHandCursor);
    btn->setStyleSheet(sidebarBtnStyle(false));
    connect(btn, &QPushButton::clicked, this, [this, viewName]{
        navigateTo(viewName);
    });
    return btn;
}

QWidget* MainWindow::buildSidebarFilters()
{
    QWidget* w = new QWidget;
    QVBoxLayout* lay = new QVBoxLayout(w);
    lay->setContentsMargins(8, 8, 8, 4);
    lay->setSpacing(2);

    // ── Нэгдсэн шүүлт ──
    QLabel* sec = new QLabel("FILTER");
    sec->setStyleSheet(QString("color:%1; font-size:10px; font-weight:700; "
                                "letter-spacing:1px; padding:0 8px 6px 8px;")
                        .arg(Style::TEXT_MUT));
    lay->addWidget(sec);

    // helper: шүүлт товч + тоо pill
    auto makeFilterRow = [&](QPushButton*& btn, QLabel*& pill,
                              const QString& icon, const QString& label,
                              const QString& filterName) {
        QWidget* row = new QWidget;
        QHBoxLayout* rl = new QHBoxLayout(row);
        rl->setContentsMargins(0, 0, 0, 0);
        rl->setSpacing(0);

        btn = new QPushButton(icon + "  " + label);
        btn->setFixedHeight(34);
        btn->setCheckable(true);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setStyleSheet(filterBtnStyle(false));
        rl->addWidget(btn, 1);

        pill = new QLabel("0");
        pill->setFixedSize(32, 20);
        pill->setAlignment(Qt::AlignCenter);
        pill->setStyleSheet(QString("background:%1; color:%2; "
                                     "border-radius:10px; font-size:10px; font-weight:600;")
                             .arg(Style::BG_HOVER, Style::TEXT_SEC));
        rl->addWidget(pill);

        connect(btn, &QPushButton::clicked, this, [this, filterName]{
            onSideFilterChanged(filterName);
        });
        lay->addWidget(row);
    };

    makeFilterRow(m_filterAll,     m_pillAll,     "🔵", "All Words",  "all");
    makeFilterRow(m_filterNew,     m_pillNew,     "⬜", "New",        "new");
    makeFilterRow(m_filterHard,    m_pillHard,    "🔴", "Hard",       "hard");
    makeFilterRow(m_filterLearned, m_pillLearned, "🟢", "Learned",    "learned");

    // ── HSK түвшин шүүлт (зөвхөн ZH хэлэнд) ──
    m_hskSection = new QWidget;
    QVBoxLayout* hskLay = new QVBoxLayout(m_hskSection);
    hskLay->setContentsMargins(0, 8, 0, 0);
    hskLay->setSpacing(2);

    QLabel* hskSec = new QLabel("HSK LEVEL");
    hskSec->setStyleSheet(QString("color:%1; font-size:10px; font-weight:700; "
                                   "letter-spacing:1px; padding:0 8px 6px 8px;")
                           .arg(Style::TEXT_MUT));
    hskLay->addWidget(hskSec);

    const QStringList hskLabels = {"HSK 1","HSK 2","HSK 3",
                                    "HSK 4","HSK 5","HSK 6"};
    for (int i = 0; i < 6; ++i) {
        QWidget* row = new QWidget;
        QHBoxLayout* rl = new QHBoxLayout(row);
        rl->setContentsMargins(0, 0, 0, 0);
        rl->setSpacing(0);

        m_filterHSK[i] = new QPushButton("📘  " + hskLabels[i]);
        m_filterHSK[i]->setFixedHeight(32);
        m_filterHSK[i]->setCheckable(true);
        m_filterHSK[i]->setCursor(Qt::PointingHandCursor);
        m_filterHSK[i]->setStyleSheet(filterBtnStyle(false));
        rl->addWidget(m_filterHSK[i], 1);

        m_pillHSK[i] = new QLabel(
            QString::number(m_db->hskLevelCount(i + 1)));
        m_pillHSK[i]->setFixedSize(32, 20);
        m_pillHSK[i]->setAlignment(Qt::AlignCenter);
        m_pillHSK[i]->setStyleSheet(
            QString("background:%1; color:%2; border-radius:10px; "
                    "font-size:10px; font-weight:600;")
            .arg(Style::BG_HOVER, Style::TEXT_SEC));
        rl->addWidget(m_pillHSK[i]);

        const QString sec = hskLabels[i];
        connect(m_filterHSK[i], &QPushButton::clicked, this, [this, sec]{
            onSideFilterChanged(sec);
        });
        hskLay->addWidget(row);
    }

    m_hskSection->setVisible(false);   // EN хэлд нуух
    lay->addWidget(m_hskSection);

    return w;
}

QWidget* MainWindow::buildSidebarStats()
{
    QWidget* w = new QWidget;
    w->setStyleSheet(QString("border-top:1px solid %1;").arg(Style::BORDER));

    QVBoxLayout* lay = new QVBoxLayout(w);
    lay->setContentsMargins(16, 12, 16, 16);
    lay->setSpacing(4);

    // Хувь + progress bar
    QHBoxLayout* pctRow = new QHBoxLayout;
    m_sbPct = new QLabel("0%");
    m_sbPct->setStyleSheet(
        QString("color:%1; font-size:12px; font-weight:700;").arg(Style::GOLD));
    pctRow->addWidget(m_sbPct);
    pctRow->addStretch();

    m_sbGoal = new QLabel("Goal: 28");
    m_sbGoal->setStyleSheet(
        QString("color:%1; font-size:10px;").arg(Style::TEXT_SEC));
    pctRow->addWidget(m_sbGoal);
    lay->addLayout(pctRow);

    m_sbBar = new QProgressBar;
    m_sbBar->setFixedHeight(4);
    m_sbBar->setRange(0, 100);
    m_sbBar->setValue(0);
    m_sbBar->setTextVisible(false);
    m_sbBar->setStyleSheet(QString(
        "QProgressBar { background:%1; border-radius:2px; border:none; }"
        "QProgressBar::chunk { background:%2; border-radius:2px; }")
        .arg(Style::BG_HOVER, Style::GOLD));
    lay->addWidget(m_sbBar);

    // Өнөөдөр / Streak
    m_sbToday = new QLabel("Today: 0");
    m_sbToday->setStyleSheet(
        QString("color:%1; font-size:11px;").arg(Style::TEXT_SEC));
    lay->addWidget(m_sbToday);

    m_sbStreak = new QLabel("🔥 0 day streak");
    m_sbStreak->setStyleSheet(
        QString("color:%1; font-size:11px;").arg(Style::TEXT_SEC));
    lay->addWidget(m_sbStreak);

    return w;
}

// ════════════════════════════════════════════════════
//  Topbar
// ════════════════════════════════════════════════════

void MainWindow::setupTopbar()
{
    m_topbar = new QWidget;
    m_topbar->setFixedHeight(Style::TOPBAR_H);
    m_topbar->setStyleSheet(QString(
        "background:%1; border-bottom:1px solid %2;")
        .arg(Style::BG_SIDEBAR, Style::BORDER));

    QHBoxLayout* lay = new QHBoxLayout(m_topbar);
    lay->setContentsMargins(24, 0, 16, 0);

    QVBoxLayout* titleLay = new QVBoxLayout;
    titleLay->setSpacing(0);

    m_topHeading = new QLabel("Home");
    m_topHeading->setStyleSheet(
        QString("color:%1; font-size:16px; font-weight:700;").arg(Style::TEXT_PRI));
    titleLay->addWidget(m_topHeading);

    m_topSub = new QLabel("");
    m_topSub->setStyleSheet(
        QString("color:%1; font-size:11px;").arg(Style::TEXT_SEC));
    titleLay->addWidget(m_topSub);

    lay->addLayout(titleLay, 1);

    // Settings товч
    m_btnSettings = new QPushButton("⚙ Settings");
    m_btnSettings->setFixedHeight(32);
    m_btnSettings->setCursor(Qt::PointingHandCursor);
    m_btnSettings->setStyleSheet(QString(
        "QPushButton { background:transparent; color:%1; "
        "border:1px solid %2; border-radius:6px; padding:0 12px; font-size:12px; }"
        "QPushButton:hover { background:%3; }")
        .arg(Style::TEXT_SEC, Style::BORDER, Style::BG_HOVER));
    connect(m_btnSettings, &QPushButton::clicked,
            this, &MainWindow::onOpenSettings);
    lay->addWidget(m_btnSettings);
}

// ════════════════════════════════════════════════════
//  Content area — QStackedWidget
// ════════════════════════════════════════════════════

void MainWindow::setupContentArea()
{
    m_stack = new QStackedWidget;
    m_stack->setStyleSheet(QString("background:%1;").arg(Style::BG_DARK));

    m_homeView  = buildHomeView();
    m_flashView = new FlashcardView(m_db, m_progressEN, m_progressZH,
                                    &m_settings, this);
    m_browseView = new BrowseView(m_db, m_progressEN, m_progressZH,
                                  &m_settings, this);
    m_statsView  = new StatsView(m_db, m_progressEN, m_progressZH,
                                 &m_settings, this);

    m_stack->addWidget(m_homeView);    // index 0
    m_stack->addWidget(m_flashView);   // index 1
    m_stack->addWidget(m_browseView);  // index 2
    m_stack->addWidget(m_statsView);   // index 3

    // Browse шүүлт → sidebar filter sync
    connect(m_browseView, &BrowseView::filterChanged,
            this, [this](const QString& f){
        m_currentFilter = f;
        setSidebarActiveFilter(f);
        updateSidebarStats();
    });
}

// ════════════════════════════════════════════════════
//  Home view
// ════════════════════════════════════════════════════

QWidget* MainWindow::buildHomeView()
{
    QScrollArea* scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setStyleSheet("QScrollArea { background: transparent; border: none; }");

    QWidget* content = new QWidget;
    content->setStyleSheet("background: transparent;");
    QVBoxLayout* lay = new QVBoxLayout(content);
    lay->setContentsMargins(40, 40, 40, 40);
    lay->setSpacing(24);

    // Мэндчилгээ
    m_homeGreeting = new QLabel("Good morning! 👋");
    m_homeGreeting->setStyleSheet(
        QString("color:%1; font-size:26px; font-weight:700;").arg(Style::TEXT_PRI));
    lay->addWidget(m_homeGreeting);

    m_homeSub = new QLabel("Ready to learn some words today?");
    m_homeSub->setStyleSheet(
        QString("color:%1; font-size:14px;").arg(Style::TEXT_SEC));
    lay->addWidget(m_homeSub);

    // Статистик карт мөр
    QHBoxLayout* statsRow = new QHBoxLayout;
    statsRow->setSpacing(16);

    auto makeStatCard = [&](QLabel*& lbl, const QString& title,
                             const QString& val, const QString& color) {
        QWidget* card = new QWidget;
        card->setFixedHeight(100);
        card->setStyleSheet(QString(
            "background:%1; border-radius:12px; border:1px solid %2;")
            .arg(Style::BG_CARD, Style::BORDER));
        QVBoxLayout* cl = new QVBoxLayout(card);
        cl->setContentsMargins(20, 16, 20, 16);

        lbl = new QLabel(val);
        lbl->setStyleSheet(
            QString("color:%1; font-size:28px; font-weight:700;").arg(color));
        cl->addWidget(lbl);

        QLabel* t = new QLabel(title);
        t->setStyleSheet(
            QString("color:%1; font-size:11px; font-weight:600; letter-spacing:0.5px;")
            .arg(Style::TEXT_SEC));
        cl->addWidget(t);
        statsRow->addWidget(card, 1);
    };

    makeStatCard(m_homeTotal,   "TOTAL WORDS", "0",   Style::TEXT_PRI);
    makeStatCard(m_homeLearned, "LEARNED",     "0",   Style::GREEN);
    makeStatCard(m_homeHard,    "HARD",        "0",   Style::RED);
    makeStatCard(m_homeToday,   "TODAY",       "0",   Style::GOLD);

    lay->addLayout(statsRow);

    // Quick action товчнууд
    QLabel* qaSec = new QLabel("Quick Start");
    qaSec->setStyleSheet(
        QString("color:%1; font-size:16px; font-weight:700;").arg(Style::TEXT_PRI));
    lay->addWidget(qaSec);

    QHBoxLayout* qaRow = new QHBoxLayout;
    qaRow->setSpacing(12);

    auto makeQABtn = [&](const QString& icon, const QString& label,
                          const QString& sub, const QString& view) {
        QWidget* card = new QWidget;
        card->setCursor(Qt::PointingHandCursor);
        card->setFixedHeight(80);
        card->setStyleSheet(QString(
            "QWidget { background:%1; border-radius:10px; border:1px solid %2; }"
            "QWidget:hover { background:%3; border-color:%4; }")
            .arg(Style::BG_CARD, Style::BORDER, Style::BG_HOVER, Style::GOLD_DIM));
        QHBoxLayout* cl = new QHBoxLayout(card);
        cl->setContentsMargins(16, 0, 16, 0);

        QLabel* ic = new QLabel(icon);
        ic->setStyleSheet("font-size:24px;");
        cl->addWidget(ic);
        cl->addSpacing(8);

        QVBoxLayout* tl = new QVBoxLayout;
        QLabel* lt = new QLabel(label);
        lt->setStyleSheet(
            QString("color:%1; font-size:13px; font-weight:600;").arg(Style::TEXT_PRI));
        tl->addWidget(lt);
        QLabel* ls = new QLabel(sub);
        ls->setStyleSheet(
            QString("color:%1; font-size:11px;").arg(Style::TEXT_SEC));
        tl->addWidget(ls);
        cl->addLayout(tl, 1);

        card->installEventFilter(this);
        card->setProperty("navTarget", view);
        qaRow->addWidget(card, 1);
    };

    makeQABtn("⚡","Flashcards","Start a session",   "flash");
    makeQABtn("🔁","Review Hard","Practice hard words","review");
    makeQABtn("📖","Browse","Explore all words",    "browse");

    lay->addLayout(qaRow);

    // HSK level progress (ZH хэлэнд)
    m_levelSection = new QWidget;
    QVBoxLayout* lvlLay = new QVBoxLayout(m_levelSection);
    lvlLay->setContentsMargins(0, 0, 0, 0);
    lvlLay->setSpacing(8);

    QLabel* lvlSec = new QLabel("HSK Level Progress");
    lvlSec->setStyleSheet(
        QString("color:%1; font-size:16px; font-weight:700;").arg(Style::TEXT_PRI));
    lvlLay->addWidget(lvlSec);

    buildLevelBars(lvlLay);
    lay->addWidget(m_levelSection);
    m_levelSection->setVisible(false);

    lay->addStretch();

    scroll->setWidget(content);
    return scroll;
}

void MainWindow::buildLevelBars(QVBoxLayout* layout)
{
    const QStringList labels = {"HSK 1","HSK 2","HSK 3","HSK 4","HSK 5","HSK 6"};
    for (int i = 0; i < 6; ++i) {
        int total = m_db->hskLevelCount(i + 1);

        QWidget* row = new QWidget;
        row->setStyleSheet(QString(
            "background:%1; border-radius:8px; border:1px solid %2;")
            .arg(Style::BG_CARD, Style::BORDER));
        QHBoxLayout* rl = new QHBoxLayout(row);
        rl->setContentsMargins(16, 10, 16, 10);

        QLabel* lbl = new QLabel(labels[i]);
        lbl->setFixedWidth(48);
        lbl->setStyleSheet(
            QString("color:%1; font-size:12px; font-weight:600;").arg(Style::GOLD));
        rl->addWidget(lbl);

        QProgressBar* bar = new QProgressBar;
        bar->setFixedHeight(6);
        bar->setRange(0, qMax(total, 1));
        bar->setValue(0);
        bar->setTextVisible(false);
        bar->setStyleSheet(QString(
            "QProgressBar { background:%1; border-radius:3px; border:none; }"
            "QProgressBar::chunk { background:%2; border-radius:3px; }")
            .arg(Style::BG_HOVER, Style::GREEN));
        rl->addWidget(bar, 1);

        QLabel* cnt = new QLabel("0 / " + QString::number(total));
        cnt->setFixedWidth(80);
        cnt->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        cnt->setStyleSheet(
            QString("color:%1; font-size:11px;").arg(Style::TEXT_SEC));
        rl->addWidget(cnt);

        layout->addWidget(row);
    }
}

void MainWindow::updateHomeView()
{
    UserProgress* prog = progress();
    int total   = m_db->totalCount(m_currentLang);
    int learned = prog->learnedCount();
    int hard    = prog->hardCount();
    int today   = prog->todayCount();

    m_homeTotal  ->setText(QString::number(total));
    m_homeLearned->setText(QString::number(learned));
    m_homeHard   ->setText(QString::number(hard));
    m_homeToday  ->setText(QString::number(today));

    // Мэндчилгээ
    const QTime t = QTime::currentTime();
    QString greet = t.hour() < 12 ? "Good morning! 🌅"
                  : t.hour() < 17 ? "Good afternoon! ☀️"
                  : "Good evening! 🌙";
    m_homeGreeting->setText(greet);

    // HSK level section
    bool isZH = (m_currentLang == "zh");
    m_levelSection->setVisible(isZH);

    if (isZH) {
        // Level bar-уудыг шинэчлэх
        QLayout* lvlLay = m_levelSection->layout();
        for (int i = 1; i < lvlLay->count(); ++i) {
            QWidget* row = lvlLay->itemAt(i)->widget();
            if (!row) continue;
            QProgressBar* bar = row->findChild<QProgressBar*>();
            QLabel*       cnt = row->findChildren<QLabel*>().last();
            if (!bar || !cnt) continue;

            const QVector<Word>& lvlWords = m_db->hskLevel(i);
            int lvlTotal   = lvlWords.size();
            int lvlLearned = 0;
            for (const Word& w : lvlWords)
                if (prog->isLearned(w.id)) lvlLearned++;
            bar->setRange(0, qMax(lvlTotal, 1));
            bar->setValue(lvlLearned);
            cnt->setText(QString("%1 / %2").arg(lvlLearned).arg(lvlTotal));
        }
    }
}

// ════════════════════════════════════════════════════
//  Навигаци
// ════════════════════════════════════════════════════

void MainWindow::navigateTo(const QString& view)
{
    m_currentView = view;

    // Nav товчны идэвхтэй байдал
    setSidebarActiveNav(view);

    // Stack солих
    if (view == "home") {
        m_stack->setCurrentWidget(m_homeView);
        updateHomeView();
        m_topHeading->setText("Home");
        m_topSub->setText(QString("WeWords — %1 words")
                          .arg(m_db->totalCount(m_currentLang)));
    } else if (view == "flash") {
        m_flashView->startSession(m_currentLang, m_currentFilter,
                                  SessionMode::Flash);
        m_stack->setCurrentWidget(m_flashView);
        m_topHeading->setText("Flashcards");
        m_topSub->setText("Press Space to flip, K = known, H = hard");
    } else if (view == "review") {
        m_flashView->startSession(m_currentLang, "hard",
                                  SessionMode::Review);
        m_stack->setCurrentWidget(m_flashView);
        m_topHeading->setText("Review Hard Words");
        m_topSub->setText("Practice the words you marked hard");
    } else if (view == "browse") {
        m_browseView->refresh(m_currentLang, m_currentFilter);
        m_stack->setCurrentWidget(m_browseView);
        m_topHeading->setText("Browse");
        m_topSub->setText(QString("%1 words")
                          .arg(m_db->totalCount(m_currentLang)));
    } else if (view == "stats") {
        m_statsView->refresh(m_currentLang);
        m_stack->setCurrentWidget(m_statsView);
        m_topHeading->setText("Statistics");
        m_topSub->setText("Your learning progress");
    }
}

void MainWindow::onNavigate(const QString& view)
{
    navigateTo(view);
}

// ════════════════════════════════════════════════════
//  Хэл солих
// ════════════════════════════════════════════════════

void MainWindow::onLanguageChanged(const QString& lang)
{
    if (m_currentLang == lang) return;
    m_currentLang = lang;
    m_settings.currentLanguage = lang;
    m_currentFilter = "all";

    // Хэл товч стиль
    bool enActive = (lang == "en");
    auto langBtnStyle = [](bool active) {
        return QString("QPushButton { background:%1; color:%2; "
                        "border:1px solid %3; border-radius:4px; "
                        "font-size:11px; font-weight:600; }"
                        "QPushButton:hover { background:%4; }")
            .arg(active ? Style::GOLD : "transparent",
                 active ? Style::BG_DARK : Style::TEXT_SEC,
                 active ? Style::GOLD : Style::BORDER,
                 active ? Style::GOLD : Style::BG_HOVER);
    };
    m_btnLangEN->setStyleSheet(langBtnStyle(enActive));
    m_btnLangZH->setStyleSheet(langBtnStyle(!enActive));

    // HSK section харагдах байдал
    m_hskSection->setVisible(!enActive);

    // Filter reset → "all"
    setSidebarActiveFilter("all");
    updateSidebarStats();

    // Одоогийн view-г refresh хийнэ
    navigateTo(m_currentView);
}

// ════════════════════════════════════════════════════
//  Шүүлт
// ════════════════════════════════════════════════════

void MainWindow::onSideFilterChanged(const QString& filter)
{
    m_currentFilter = filter;
    setSidebarActiveFilter(filter);

    // Flashcard эсвэл Browse-д шилжих
    if (m_currentView == "flash" || m_currentView == "review") {
        m_flashView->startSession(m_currentLang, filter, SessionMode::Flash);
    } else {
        navigateTo("browse");
    }
}

void MainWindow::setSidebarActiveNav(const QString& view)
{
    struct { QPushButton** btn; const char* name; } navs[] = {
        {&m_navHome,   "home"},
        {&m_navFlash,  "flash"},
        {&m_navBrowse, "browse"},
        {&m_navReview, "review"},
        {&m_navStats,  "stats"},
    };
    for (auto& n : navs) {
        bool active = (view == n.name) ||
                      (view == "review" && QString(n.name) == "review");
        (*n.btn)->setChecked(active);
        (*n.btn)->setStyleSheet(sidebarBtnStyle(active));
    }
}

void MainWindow::setSidebarActiveFilter(const QString& filter)
{
    auto setF = [&](QPushButton* btn, const QString& name) {
        bool a = (filter == name);
        btn->setChecked(a);
        btn->setStyleSheet(filterBtnStyle(a));
    };
    setF(m_filterAll,     "all");
    setF(m_filterNew,     "new");
    setF(m_filterHard,    "hard");
    setF(m_filterLearned, "learned");

    for (int i = 0; i < 6; ++i) {
        const QString sec = QString("HSK %1").arg(i + 1);
        bool a = (filter == sec);
        m_filterHSK[i]->setChecked(a);
        m_filterHSK[i]->setStyleSheet(filterBtnStyle(a));
    }
}

// ════════════════════════════════════════════════════
//  Sidebar статистик шинэчлэх
// ════════════════════════════════════════════════════

void MainWindow::onProgressChanged()
{
    updateSidebarStats();
    if (m_currentView == "home") updateHomeView();
    if (m_currentView == "browse") m_browseView->refresh(m_currentLang, m_currentFilter);
    if (m_currentView == "stats")  m_statsView->refresh(m_currentLang);
}

void MainWindow::updateSidebarStats()
{
    UserProgress* prog = progress();
    int total   = m_db->totalCount(m_currentLang);
    int learned = prog->learnedCount();
    int hard    = prog->hardCount();
    int newW    = total - learned - hard;
    int today   = prog->todayCount();
    int goal    = m_settings.dailyGoal;
    int streak  = prog->streakDays();
    int pct     = total > 0 ? (learned * 100 / total) : 0;

    m_sbPct->setText(QString("%1%").arg(pct));
    m_sbBar->setValue(pct);
    m_sbGoal->setText(QString("Goal: %1").arg(goal));
    m_sbToday->setText(QString("Today: %1 / %2").arg(today).arg(goal));
    m_sbStreak->setText(QString("🔥 %1 day streak").arg(streak));

    // Pill тоо шинэчлэх
    m_pillAll->setText(QString::number(total));
    m_pillNew->setText(QString::number(qMax(0, newW)));
    m_pillHard->setText(QString::number(hard));
    m_pillLearned->setText(QString::number(learned));

    // HSK pill
    for (int i = 0; i < 6; ++i)
        m_pillHSK[i]->setText(
            QString::number(m_db->hskLevelCount(i + 1)));
}

// ════════════════════════════════════════════════════
//  Settings
// ════════════════════════════════════════════════════

void MainWindow::onOpenSettings()
{
    if (!m_settingsDialog)
        m_settingsDialog = new SettingsDialog(&m_settings,
                                              m_progressEN,
                                              m_progressZH, this);
    if (m_settingsDialog->exec() == QDialog::Accepted) {
        m_settings = m_settingsDialog->getSettings();
        // Тохиргоо хэрэгжүүлэх
        onProgressChanged();
    }
}

// ════════════════════════════════════════════════════
//  EventFilter — Home view quick-action карт
// ════════════════════════════════════════════════════

bool MainWindow::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() == QEvent::MouseButtonRelease) {
        QWidget* w = qobject_cast<QWidget*>(obj);
        if (w) {
            const QString target = w->property("navTarget").toString();
            if (!target.isEmpty()) { navigateTo(target); return true; }
        }
    }
    return QMainWindow::eventFilter(obj, event);
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    m_progressEN->save();
    m_progressZH->save();
    event->accept();
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
    QMainWindow::resizeEvent(event);
}

// ════════════════════════════════════════════════════
//  Style helpers
// ════════════════════════════════════════════════════

QString MainWindow::sidebarBtnStyle(bool active, bool /*gold*/)
{
    return QString(
        "QPushButton { background:%1; color:%2; border:none; "
        "border-radius:6px; text-align:left; padding:0 12px; "
        "font-size:13px; font-weight:%3; }"
        "QPushButton:hover { background:%4; color:%5; }")
        .arg(active ? Style::GOLD + "22" : "transparent",
             active ? Style::GOLD : Style::TEXT_SEC,
             active ? "600" : "400",
             Style::BG_HOVER,
             active ? Style::GOLD : Style::TEXT_PRI);
}

QString MainWindow::filterBtnStyle(bool active, bool /*gold*/)
{
    return QString(
        "QPushButton { background:%1; color:%2; border:none; "
        "border-radius:6px; text-align:left; padding:0 8px; "
        "font-size:12px; font-weight:%3; }"
        "QPushButton:hover { background:%4; color:%5; }")
        .arg(active ? Style::GOLD + "22" : "transparent",
             active ? Style::GOLD : Style::TEXT_SEC,
             active ? "600" : "400",
             Style::BG_HOVER,
             active ? Style::GOLD : Style::TEXT_PRI);
}
