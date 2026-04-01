#include "WordCard.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QPainterPath>
#include <QEnterEvent>

static const QString C_CARD   = "#1a1a1a";
static const QString C_BORDER = "#2a2a2a";
static const QString C_HOVER  = "#222222";
static const QString C_TEXT   = "#e8e8e8";
static const QString C_SEC    = "#888888";
static const QString C_GOLD   = "#c9a96e";
static const QString C_GREEN  = "#4a7c59";
static const QString C_RED    = "#8b3a3a";
static const QString C_MUT    = "#555555";

WordCardWidget::WordCardWidget(const Word& word, QWidget* parent)
    : QWidget(parent), m_word(word)
{
    setupUi();
}

void WordCardWidget::setupUi()
{
    setFixedHeight(110);
    setCursor(Qt::PointingHandCursor);
    setMouseTracking(true);

    QVBoxLayout* lay = new QVBoxLayout(this);
    lay->setContentsMargins(14, 10, 14, 10);
    lay->setSpacing(4);

    // Top row: word + badge
    QHBoxLayout* top = new QHBoxLayout;
    top->setSpacing(6);

    m_wordLbl = new QLabel(m_word.english);
    m_wordLbl->setStyleSheet(
        QString("color:%1; font-size:15px; font-weight:600;").arg(C_TEXT));
    m_wordLbl->setWordWrap(false);
    top->addWidget(m_wordLbl, 1);

    m_badge = new QLabel("New");
    m_badge->setFixedHeight(18);
    m_badge->setAlignment(Qt::AlignCenter);
    m_badge->setStyleSheet(
        QString("background:%1; color:%2; border-radius:9px; "
                "padding:0 8px; font-size:9px; font-weight:700;")
        .arg(C_MUT, C_SEC));
    top->addWidget(m_badge);

    lay->addLayout(top);

    // Pinyin (ZH only)
    m_pinyinLbl = new QLabel(m_word.pinyin);
    m_pinyinLbl->setStyleSheet(
        QString("color:%1; font-size:11px; font-style:italic;").arg(C_GOLD));
    m_pinyinLbl->setVisible(!m_word.pinyin.isEmpty());
    lay->addWidget(m_pinyinLbl);

    // Mongolian
    m_mnLbl = new QLabel(m_word.mongolian);
    m_mnLbl->setStyleSheet(
        QString("color:%1; font-size:12px;").arg(C_SEC));
    m_mnLbl->setWordWrap(false);
    QFontMetrics fm(m_mnLbl->font());
    m_mnLbl->setText(
        fm.elidedText(m_word.mongolian, Qt::ElideRight, 180));
    lay->addWidget(m_mnLbl);

    // Section badge row (HSK)
    if (!m_word.section.isEmpty()) {
        m_secBadge = new QLabel(m_word.section);
        m_secBadge->setFixedHeight(16);
        m_secBadge->setStyleSheet(
            QString("background:%1; color:%2; border-radius:8px; "
                    "padding:0 6px; font-size:9px; font-weight:600;")
            .arg(C_GOLD + "22", C_GOLD));
        lay->addWidget(m_secBadge);
    }

    // Status badge update
    switch (m_word.status) {
    case WordStatus::Learned:
        m_badge->setText("Learned");
        m_badge->setStyleSheet(
            QString("background:%1; color:#d4edda; border-radius:9px; "
                    "padding:0 8px; font-size:9px; font-weight:700;")
            .arg(C_GREEN));
        break;
    case WordStatus::Hard:
        m_badge->setText("Hard");
        m_badge->setStyleSheet(
            QString("background:%1; color:#f8d7da; border-radius:9px; "
                    "padding:0 8px; font-size:9px; font-weight:700;")
            .arg(C_RED));
        break;
    default:
        break;
    }
}

void WordCardWidget::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    // Left border color (status indicator)
    QString borderColor = C_BORDER;
    if (m_word.status == WordStatus::Learned) borderColor = C_GREEN;
    else if (m_word.status == WordStatus::Hard) borderColor = C_RED;

    // Background
    QPainterPath path;
    path.addRoundedRect(rect(), 8, 8);
    p.fillPath(path, QColor(m_hovered ? C_HOVER : C_CARD));

    // Border
    p.setPen(QPen(QColor(borderColor), 1));
    p.drawPath(path);

    // Left accent bar
    p.fillRect(0, 8, 3, height() - 16, QColor(borderColor));
}

void WordCardWidget::mousePressEvent(QMouseEvent*)
{
    emit clicked(m_word);
}

void WordCardWidget::enterEvent(QEnterEvent*)
{
    m_hovered = true;
    update();
}

void WordCardWidget::leaveEvent(QEvent*)
{
    m_hovered = false;
    update();
}
