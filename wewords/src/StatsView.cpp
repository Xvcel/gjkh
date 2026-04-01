#include "StatsView.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QFrame>
#include <QDate>
#include <QPainterPath>

static const QString C_BG     = "#0d0d0d";
static const QString C_CARD   = "#1a1a1a";
static const QString C_BORDER = "#2a2a2a";
static const QString C_GOLD   = "#c9a96e";
static const QString C_TEXT   = "#e8e8e8";
static const QString C_SEC    = "#888888";
static const QString C_MUT    = "#555555";
static const QString C_HOVER  = "#222222";
static const QString C_GREEN  = "#4a7c59";
static const QString C_RED    = "#8b3a3a";
static const QString C_BLUE   = "#3a5a8b";

// ════════════════════════════════════════════════════
//  DailyHistoryChart
// ════════════════════════════════════════════════════

DailyHistoryChart::DailyHistoryChart(QWidget* parent)
    : QWidget(parent)
{
    setFixedHeight(140);
    setStyleSheet("background: transparent;");
}

void DailyHistoryChart::setData(UserProgress* prog, int dailyGoal)
{
    m_goal = dailyGoal;
    m_bars.clear();
    m_max = 1;

    const QDate today = QDate::currentDate();
    for (int i = 13; i >= 0; --i) {
        const QDate d = today.addDays(-i);
        int val = (i == 0) ? prog->todayCount()
                            : prog->historyFor(d);
        Bar b;
        b.lbl   = d.toString("d/M");
        b.val   = val;
        b.today = (i == 0);
        m_bars.append(b);
        if (val > m_max) m_max = val;
    }
    update();
}

void DailyHistoryChart::paintEvent(QPaintEvent*)
{
    if (m_bars.isEmpty()) return;

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const int W    = width();
    const int H    = height();
    const int N    = m_bars.size();
    const int gap  = 4;
    const int barW = (W - gap * (N + 1)) / N;
    const int maxH = H - 28;  // leave room for labels

    // Goal line
    if (m_max > 0) {
        const int goalY = H - 20 - (int)((float)m_goal / m_max * maxH);
        p.setPen(QPen(QColor(C_GOLD), 1, Qt::DashLine));
        p.drawLine(0, goalY, W, goalY);
    }

    for (int i = 0; i < N; ++i) {
        const Bar& b  = m_bars[i];
        const int x   = gap + i * (barW + gap);
        const int bH  = m_max > 0 ? (int)((float)b.val / m_max * maxH) : 0;
        const int y   = H - 20 - bH;

        // Bar
        QColor fillColor = b.today ? QColor(C_GOLD)
                         : (b.val >= m_goal ? QColor(C_GREEN) : QColor(C_BLUE));
        if (b.val == 0) fillColor = QColor(C_HOVER);

        QPainterPath path;
        path.addRoundedRect(x, y, barW, bH + 1, 3, 3);
        p.fillPath(path, fillColor);

        // Label
        p.setPen(b.today ? QColor(C_GOLD) : QColor(C_MUT));
        p.setFont(QFont("Segoe UI", 7));
        p.drawText(QRect(x, H - 16, barW, 16),
                   Qt::AlignCenter, b.lbl);
    }
}

// ════════════════════════════════════════════════════
//  StatsView
// ════════════════════════════════════════════════════

StatsView::StatsView(WordDatabase*  db,
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

static QWidget* makeCard(const QString& title, QVBoxLayout*& bodyLayout)
{
    QWidget* card = new QWidget;
    card->setStyleSheet(QString(
        "QWidget { background:%1; border-radius:12px; border:1px solid %2; }")
        .arg(C_CARD, C_BORDER));
    QVBoxLayout* cl = new QVBoxLayout(card);
    cl->setContentsMargins(24, 20, 24, 20);
    cl->setSpacing(12);

    QLabel* t = new QLabel(title);
    t->setStyleSheet(
        QString("color:%1; font-size:14px; font-weight:700;").arg(C_TEXT));
    cl->addWidget(t);

    QFrame* sep = new QFrame;
    sep->setFrameShape(QFrame::HLine);
    sep->setStyleSheet(QString("color:%1;").arg(C_BORDER));
    cl->addWidget(sep);

    bodyLayout = cl;
    return card;
}

void StatsView::setupUi()
{
    setStyleSheet(QString("background:%1;").arg(C_BG));

    QScrollArea* scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setStyleSheet("QScrollArea { border:none; background:transparent; }");

    QWidget* content = new QWidget;
    content->setStyleSheet("background:transparent;");
    QVBoxLayout* lay = new QVBoxLayout(content);
    lay->setContentsMargins(32, 24, 32, 24);
    lay->setSpacing(20);

    // ── Карт 1: Overall ──
    QVBoxLayout* overallBody = nullptr;
    QWidget* overallCard = makeCard("Overall Progress", overallBody);

    QHBoxLayout* topRow = new QHBoxLayout;

    // Том хувь
    QVBoxLayout* pctCol = new QVBoxLayout;
    m_pctLbl = new QLabel("0%");
    m_pctLbl->setStyleSheet(
        QString("color:%1; font-size:48px; font-weight:700;").arg(C_GOLD));
    pctCol->addWidget(m_pctLbl);
    m_learnSub = new QLabel("0 of 0 words learned");
    m_learnSub->setStyleSheet(
        QString("color:%1; font-size:12px;").arg(C_SEC));
    pctCol->addWidget(m_learnSub);
    topRow->addLayout(pctCol, 1);

    // Detail stats
    QVBoxLayout* detCol = new QVBoxLayout;
    detCol->setSpacing(6);
    auto makeDet = [&](QLabel*& lbl, const QString& pre) {
        QWidget* row = new QWidget;
        QHBoxLayout* rl = new QHBoxLayout(row);
        rl->setContentsMargins(0,0,0,0);
        QLabel* pl = new QLabel(pre);
        pl->setStyleSheet(
            QString("color:%1; font-size:12px;").arg(C_SEC));
        rl->addWidget(pl);
        lbl = new QLabel("0");
        lbl->setStyleSheet(
            QString("color:%1; font-size:12px; font-weight:600;").arg(C_TEXT));
        rl->addWidget(lbl);
        rl->addStretch();
        detCol->addWidget(row);
    };
    makeDet(m_detLearn, "🟢 Learned:");
    makeDet(m_detHard,  "🔴 Hard:");
    makeDet(m_detNew,   "⬜ New:");
    makeDet(m_detToday, "⭐ Today:");
    topRow->addLayout(detCol);
    overallBody->addLayout(topRow);

    m_bar = new QProgressBar;
    m_bar->setFixedHeight(6);
    m_bar->setTextVisible(false);
    m_bar->setStyleSheet(QString(
        "QProgressBar { background:%1; border-radius:3px; border:none; }"
        "QProgressBar::chunk { background:%2; border-radius:3px; }")
        .arg(C_HOVER, C_GOLD));
    overallBody->addWidget(m_bar);

    m_streakLbl = new QLabel("🔥 0 day streak");
    m_streakLbl->setStyleSheet(
        QString("color:%1; font-size:13px; font-weight:600;").arg(C_TEXT));
    overallBody->addWidget(m_streakLbl);

    lay->addWidget(overallCard);

    // ── Карт 2: Daily history ──
    QVBoxLayout* histBody = nullptr;
    QWidget* histCard = makeCard("14-Day History", histBody);
    m_chart = new DailyHistoryChart;
    histBody->addWidget(m_chart);
    lay->addWidget(histCard);

    // ── Карт 3: HSK level (ZH) ──
    QVBoxLayout* lvlBody = nullptr;
    m_levelCard = makeCard("HSK Level Progress", lvlBody);

    m_levelRows = new QWidget;
    QVBoxLayout* lvlRowLay = new QVBoxLayout(m_levelRows);
    lvlRowLay->setContentsMargins(0,0,0,0);
    lvlRowLay->setSpacing(10);

    const QStringList lvls = {"HSK 1","HSK 2","HSK 3","HSK 4","HSK 5","HSK 6"};
    const QStringList colors = {C_GREEN, C_BLUE, C_GOLD,
                                  "#7c5a4a", "#4a5a7c", "#7c4a6a"};
    for (int i = 0; i < 6; ++i) {
        QWidget* row = new QWidget;
        row->setFixedHeight(44);
        QHBoxLayout* rl = new QHBoxLayout(row);
        rl->setContentsMargins(0,0,0,0);

        QLabel* lbl = new QLabel(lvls[i]);
        lbl->setFixedWidth(52);
        lbl->setStyleSheet(
            QString("color:%1; font-size:12px; font-weight:700;")
            .arg(colors[i]));
        rl->addWidget(lbl);

        QProgressBar* bar = new QProgressBar;
        bar->setFixedHeight(8);
        bar->setTextVisible(false);
        bar->setStyleSheet(QString(
            "QProgressBar { background:%1; border-radius:4px; border:none; }"
            "QProgressBar::chunk { background:%2; border-radius:4px; }")
            .arg(C_HOVER, colors[i]));
        rl->addWidget(bar, 1);

        QLabel* cnt = new QLabel("0 / 0");
        cnt->setFixedWidth(70);
        cnt->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        cnt->setStyleSheet(
            QString("color:%1; font-size:11px;").arg(C_SEC));
        rl->addWidget(cnt);

        lvlRowLay->addWidget(row);
    }

    lvlBody->addWidget(m_levelRows);
    lay->addWidget(m_levelCard);

    lay->addStretch();
    scroll->setWidget(content);

    QVBoxLayout* rootLay = new QVBoxLayout(this);
    rootLay->setContentsMargins(0,0,0,0);
    rootLay->addWidget(scroll);
}

void StatsView::refresh(const QString& lang)
{
    m_lang = lang;
    UserProgress* prog = progress();
    const int total   = m_db->totalCount(lang);
    const int learned = prog->learnedCount();
    const int hard    = prog->hardCount();
    const int newW    = total - learned - hard;
    const int today   = prog->todayCount();
    const int pct     = total > 0 ? learned * 100 / total : 0;
    const int goal    = m_settings ? m_settings->dailyGoal : 28;

    m_pctLbl->setText(QString("%1%").arg(pct));
    m_learnSub->setText(
        QString("%1 of %2 words learned").arg(learned).arg(total));
    m_bar->setRange(0, qMax(total, 1));
    m_bar->setValue(learned);
    m_detLearn->setText(QString::number(learned));
    m_detHard->setText(QString::number(hard));
    m_detNew->setText(QString::number(qMax(0, newW)));
    m_detToday->setText(
        QString("%1 / %2").arg(today).arg(goal));
    m_streakLbl->setText(
        QString("🔥 %1 day streak").arg(prog->streakDays()));

    m_chart->setData(prog, goal);

    // HSK level bars
    m_levelCard->setVisible(lang == "zh");
    if (lang == "zh") {
        QLayout* lay = m_levelRows->layout();
        for (int i = 0; i < 6; ++i) {
            QWidget* row = lay->itemAt(i)->widget();
            if (!row) continue;
            QProgressBar* bar = row->findChild<QProgressBar*>();
            QLabel* cnt = row->findChildren<QLabel*>().last();
            if (!bar || !cnt) continue;

            const QVector<Word>& ws = m_db->hskLevel(i + 1);
            int lvlTotal = ws.size();
            int lvlLearn = 0;
            for (const Word& w : ws)
                if (prog->isLearned(w.id)) lvlLearn++;

            bar->setRange(0, qMax(lvlTotal, 1));
            bar->setValue(lvlLearn);
            cnt->setText(
                QString("%1 / %2").arg(lvlLearn).arg(lvlTotal));
        }
    }
}
