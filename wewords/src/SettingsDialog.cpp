#include "SettingsDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFrame>
#include <QMessageBox>

static const QString C_BG     = "#111111";
static const QString C_CARD   = "#1a1a1a";
static const QString C_BORDER = "#2a2a2a";
static const QString C_GOLD   = "#c9a96e";
static const QString C_TEXT   = "#e8e8e8";
static const QString C_SEC    = "#888888";
static const QString C_HOVER  = "#222222";
static const QString C_RED    = "#8b3a3a";

SettingsDialog::SettingsDialog(AppSettings*   settings,
                               UserProgress*  progressEN,
                               UserProgress*  progressZH,
                               QWidget*       parent)
    : QDialog(parent)
    , m_settings(settings)
    , m_progressEN(progressEN)
    , m_progressZH(progressZH)
{
    setWindowTitle("Settings");
    setFixedWidth(460);
    setModal(true);
    setupUi();
}

QWidget* SettingsDialog::makeRow(const QString& label,
                                   const QString& sub,
                                   QWidget* ctrl)
{
    QWidget* row = new QWidget;
    QHBoxLayout* rl = new QHBoxLayout(row);
    rl->setContentsMargins(0, 6, 0, 6);

    QVBoxLayout* textCol = new QVBoxLayout;
    QLabel* lbl = new QLabel(label);
    lbl->setStyleSheet(
        QString("color:%1; font-size:13px; font-weight:600;").arg(C_TEXT));
    textCol->addWidget(lbl);
    if (!sub.isEmpty()) {
        QLabel* s = new QLabel(sub);
        s->setStyleSheet(
            QString("color:%1; font-size:11px;").arg(C_SEC));
        textCol->addWidget(s);
    }
    rl->addLayout(textCol, 1);
    rl->addWidget(ctrl);
    return row;
}

void SettingsDialog::setupUi()
{
    setStyleSheet(QString(
        "QDialog { background:%1; color:%2; }"
        "QCheckBox { color:%2; font-size:13px; }"
        "QCheckBox::indicator { width:18px; height:18px; border:1px solid %3; "
        "border-radius:4px; background:%4; }"
        "QCheckBox::indicator:checked { background:%5; border-color:%5; }"
        "QSpinBox { background:%4; color:%2; border:1px solid %3; "
        "border-radius:6px; padding:4px 8px; font-size:13px; min-width:70px; }"
        "QPushButton { border-radius:6px; font-size:13px; }")
        .arg(C_BG, C_TEXT, C_BORDER, C_CARD, C_GOLD));

    QVBoxLayout* root = new QVBoxLayout(this);
    root->setContentsMargins(24, 20, 24, 20);
    root->setSpacing(4);

    // Title
    QLabel* title = new QLabel("⚙  Settings");
    title->setStyleSheet(
        QString("color:%1; font-size:18px; font-weight:700;").arg(C_TEXT));
    root->addWidget(title);
    root->addSpacing(8);

    // ── Section: Cards ──
    QLabel* cardSec = new QLabel("CARDS");
    cardSec->setStyleSheet(
        QString("color:%1; font-size:10px; font-weight:700; "
                "letter-spacing:1px; margin-top:8px;").arg(C_SEC));
    root->addWidget(cardSec);

    m_reverse = new QCheckBox;
    m_reverse->setChecked(m_settings->reverseCards);
    root->addWidget(makeRow("Reverse cards",
        "Show Mongolian on front", m_reverse));

    m_autoAdv = new QCheckBox;
    m_autoAdv->setChecked(m_settings->autoAdvance);
    root->addWidget(makeRow("Auto advance",
        "Move to next after marking", m_autoAdv));

    m_pinyin = new QCheckBox;
    m_pinyin->setChecked(m_settings->showPinyin);
    root->addWidget(makeRow("Show Pinyin",
        "Display pinyin on Chinese cards", m_pinyin));

    QFrame* sep1 = new QFrame;
    sep1->setFrameShape(QFrame::HLine);
    sep1->setStyleSheet(QString("color:%1; margin:8px 0;").arg(C_BORDER));
    root->addWidget(sep1);

    // ── Section: Session ──
    QLabel* sessSec = new QLabel("SESSION");
    sessSec->setStyleSheet(
        QString("color:%1; font-size:10px; font-weight:700; letter-spacing:1px;")
        .arg(C_SEC));
    root->addWidget(sessSec);

    m_sessSize = new QSpinBox;
    m_sessSize->setRange(5, 100);
    m_sessSize->setValue(m_settings->sessionSize);
    m_sessSize->setSuffix(" cards");
    root->addWidget(makeRow("Session size",
        "Cards per flashcard session", m_sessSize));

    m_dailyGoal = new QSpinBox;
    m_dailyGoal->setRange(1, 200);
    m_dailyGoal->setValue(m_settings->dailyGoal);
    m_dailyGoal->setSuffix(" words");
    root->addWidget(makeRow("Daily goal",
        "Words to learn per day", m_dailyGoal));

    QFrame* sep2 = new QFrame;
    sep2->setFrameShape(QFrame::HLine);
    sep2->setStyleSheet(QString("color:%1; margin:8px 0;").arg(C_BORDER));
    root->addWidget(sep2);

    // ── Section: Reset ──
    QLabel* resetSec = new QLabel("PROGRESS");
    resetSec->setStyleSheet(
        QString("color:%1; font-size:10px; font-weight:700; letter-spacing:1px;")
        .arg(C_SEC));
    root->addWidget(resetSec);

    QPushButton* btnResetLearned = new QPushButton("Reset Learned");
    btnResetLearned->setFixedHeight(32);
    btnResetLearned->setCursor(Qt::PointingHandCursor);
    btnResetLearned->setStyleSheet(
        QString("QPushButton { background:transparent; color:%1; "
                "border:1px solid %1; padding:0 14px; }"
                "QPushButton:hover { background:%2; }")
        .arg(C_SEC, C_HOVER));
    connect(btnResetLearned, &QPushButton::clicked,
            this, &SettingsDialog::onResetLearned);
    root->addWidget(makeRow("Reset learned",
        "Move learned words back to new", btnResetLearned));

    QPushButton* btnResetAll = new QPushButton("Reset All");
    btnResetAll->setFixedHeight(32);
    btnResetAll->setCursor(Qt::PointingHandCursor);
    btnResetAll->setStyleSheet(
        QString("QPushButton { background:transparent; color:%1; "
                "border:1px solid %1; padding:0 14px; }"
                "QPushButton:hover { background:%2; }")
        .arg(C_RED, "#3a1a1a"));
    connect(btnResetAll, &QPushButton::clicked,
            this, &SettingsDialog::onResetAll);
    root->addWidget(makeRow("Reset all progress",
        "Clear all learned and hard words", btnResetAll));

    root->addStretch();

    // ── Buttons ──
    QHBoxLayout* btnRow = new QHBoxLayout;
    btnRow->setSpacing(10);

    QPushButton* cancel = new QPushButton("Cancel");
    cancel->setFixedHeight(36);
    cancel->setCursor(Qt::PointingHandCursor);
    cancel->setStyleSheet(
        QString("QPushButton { background:%1; color:%2; "
                "border:1px solid %3; padding:0 20px; }"
                "QPushButton:hover { background:%4; }")
        .arg(C_CARD, C_TEXT, C_BORDER, C_HOVER));
    connect(cancel, &QPushButton::clicked, this, &QDialog::reject);
    btnRow->addWidget(cancel);

    m_okBtn = new QPushButton("Save");
    m_okBtn->setFixedHeight(36);
    m_okBtn->setCursor(Qt::PointingHandCursor);
    m_okBtn->setStyleSheet(
        QString("QPushButton { background:%1; color:#1a1a1a; "
                "border:none; padding:0 24px; font-weight:700; }"
                "QPushButton:hover { background:%2; }")
        .arg(C_GOLD, "#e0b87e"));
    connect(m_okBtn, &QPushButton::clicked, this, &QDialog::accept);
    btnRow->addWidget(m_okBtn);

    root->addLayout(btnRow);
}

AppSettings SettingsDialog::getSettings() const
{
    AppSettings s = *m_settings;
    s.reverseCards  = m_reverse->isChecked();
    s.autoAdvance   = m_autoAdv->isChecked();
    s.showPinyin    = m_pinyin->isChecked();
    s.sessionSize   = m_sessSize->value();
    s.dailyGoal     = m_dailyGoal->value();
    return s;
}

void SettingsDialog::onResetLearned()
{
    if (QMessageBox::question(this, "Reset Learned",
            "Move all learned words back to New?\n"
            "This applies to both English and Chinese.",
            QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
    {
        m_progressEN->resetLearned();
        m_progressZH->resetLearned();
    }
}

void SettingsDialog::onResetAll()
{
    if (QMessageBox::question(this, "Reset All Progress",
            "Clear ALL progress (learned, hard, streaks)?\n"
            "This cannot be undone.",
            QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
    {
        m_progressEN->resetAll();
        m_progressZH->resetAll();
    }
}
