#include "FlashcardView.h"
#include <QVBoxLayout>
#include <QEvent>
#include <QHBoxLayout>
#include <QFrame>
#include <QRandomGenerator>
#include <QDate>
#include <QApplication>

// ── Өнгө тогтмол ──
static const QString C_BG      = "#0d0d0d";
static const QString C_CARD    = "#1a1a1a";
static const QString C_BORDER  = "#2a2a2a";
static const QString C_GOLD    = "#c9a96e";
static const QString C_GOLD2   = "#a07840";
static const QString C_TEXT    = "#e8e8e8";
static const QString C_SEC     = "#888888";
static const QString C_MUT     = "#555555";
static const QString C_GREEN   = "#4a7c59";
static const QString C_RED     = "#8b3a3a";
static const QString C_HOVER   = "#222222";

// ════════════════════════════════════════════════════
//  Constructor
// ════════════════════════════════════════════════════

FlashcardView::FlashcardView(WordDatabase*  db,
                             UserProgress*  progressEN,
                             UserProgress*  progressZH,
                             AppSettings*   settings,
                             QWidget*       parent)
    : QWidget(parent)
    , m_db(db), m_progressEN(progressEN)
    , m_progressZH(progressZH), m_settings(settings)
{
    m_autoTimer = new QTimer(this);
    m_autoTimer->setSingleShot(false);
    connect(m_autoTimer, &QTimer::timeout, this, &FlashcardView::onNext);

    setupUi();
}

// ════════════════════════════════════════════════════
//  UI байгуулалт
// ════════════════════════════════════════════════════

void FlashcardView::setupUi()
{
    setStyleSheet(QString("background:%1;").arg(C_BG));
    setFocusPolicy(Qt::StrongFocus);

    m_mainStack = new QStackedWidget(this);
    QVBoxLayout* root = new QVBoxLayout(this);
    root->setContentsMargins(0,0,0,0);
    root->addWidget(m_mainStack);

    // ── Карт хуудас ──
    m_cardPage = new QWidget;
    m_cardPage->setStyleSheet(QString("background:%1;").arg(C_BG));
    QVBoxLayout* cl = new QVBoxLayout(m_cardPage);
    cl->setContentsMargins(32, 20, 32, 20);
    cl->setSpacing(16);

    // Header: progress bar + counter + exit
    QHBoxLayout* hdr = new QHBoxLayout;
    m_progressBar = new QProgressBar;
    m_progressBar->setFixedHeight(4);
    m_progressBar->setTextVisible(false);
    m_progressBar->setStyleSheet(QString(
        "QProgressBar { background:%1; border-radius:2px; border:none; }"
        "QProgressBar::chunk { background:%2; border-radius:2px; }")
        .arg(C_HOVER, C_GOLD));
    hdr->addWidget(m_progressBar, 1);

    m_counterLbl = new QLabel("1 / 20");
    m_counterLbl->setStyleSheet(
        QString("color:%1; font-size:12px; margin-left:12px;").arg(C_SEC));
    hdr->addWidget(m_counterLbl);

    m_btnExit = new QPushButton("✕");
    m_btnExit->setFixedSize(28, 28);
    m_btnExit->setCursor(Qt::PointingHandCursor);
    m_btnExit->setStyleSheet(QString(
        "QPushButton { background:transparent; color:%1; border:1px solid %2; "
        "border-radius:6px; font-size:14px; }"
        "QPushButton:hover { background:%3; color:%4; }")
        .arg(C_MUT, C_BORDER, C_HOVER, C_TEXT));
    connect(m_btnExit, &QPushButton::clicked, this, &FlashcardView::onExit);
    hdr->addWidget(m_btnExit);
    cl->addLayout(hdr);

    // ── Card widget ──
    m_cardContainer = new QWidget;
    m_cardContainer->setMinimumHeight(320);
    m_cardContainer->setCursor(Qt::PointingHandCursor);
    m_cardContainer->setStyleSheet(QString(
        "QWidget { background:%1; border-radius:16px; border:1px solid %2; }")
        .arg(C_CARD, C_BORDER));
    m_cardContainer->installEventFilter(this);

    QVBoxLayout* cardLay = new QVBoxLayout(m_cardContainer);
    cardLay->setContentsMargins(40, 40, 40, 40);
    cardLay->setSpacing(12);
    cardLay->setAlignment(Qt::AlignCenter);

    m_frontTag = new QLabel("ENGLISH");
    m_frontTag->setAlignment(Qt::AlignCenter);
    m_frontTag->setStyleSheet(
        QString("color:%1; font-size:10px; font-weight:700; letter-spacing:2px;")
        .arg(C_MUT));
    cardLay->addWidget(m_frontTag);

    m_frontWord = new QLabel("...");
    m_frontWord->setAlignment(Qt::AlignCenter);
    m_frontWord->setWordWrap(true);
    m_frontWord->setStyleSheet(
        QString("color:%1; font-size:36px; font-weight:700;").arg(C_TEXT));
    cardLay->addWidget(m_frontWord);

    m_frontHint = new QLabel("click to reveal · Space");
    m_frontHint->setAlignment(Qt::AlignCenter);
    m_frontHint->setStyleSheet(
        QString("color:%1; font-size:11px;").arg(C_MUT));
    cardLay->addWidget(m_frontHint);

    // Back side (hidden by default)
    m_backTag = new QLabel("MONGOLIAN");
    m_backTag->setAlignment(Qt::AlignCenter);
    m_backTag->setStyleSheet(
        QString("color:%1; font-size:10px; font-weight:700; letter-spacing:2px;")
        .arg(C_GOLD2));
    m_backTag->hide();
    cardLay->addWidget(m_backTag);

    m_backPinyin = new QLabel("");
    m_backPinyin->setAlignment(Qt::AlignCenter);
    m_backPinyin->setStyleSheet(
        QString("color:%1; font-size:16px; font-style:italic;").arg(C_GOLD));
    m_backPinyin->hide();
    cardLay->addWidget(m_backPinyin);

    m_backTrans = new QLabel("...");
    m_backTrans->setAlignment(Qt::AlignCenter);
    m_backTrans->setWordWrap(true);
    m_backTrans->setStyleSheet(
        QString("color:%1; font-size:28px; font-weight:600;").arg(C_TEXT));
    m_backTrans->hide();
    cardLay->addWidget(m_backTrans);

    m_backSection = new QLabel("");
    m_backSection->setAlignment(Qt::AlignCenter);
    m_backSection->setStyleSheet(
        QString("background:%1; color:%2; border-radius:10px; "
                "padding:2px 10px; font-size:10px; font-weight:600;")
        .arg(C_GOLD + "33", C_GOLD));
    m_backSection->hide();
    cardLay->addWidget(m_backSection);

    cl->addWidget(m_cardContainer, 1);

    // ── Action buttons ──
    QHBoxLayout* acts = new QHBoxLayout;
    acts->setSpacing(12);

    auto makeBtn = [&](const QString& label, const QString& bg,
                        const QString& fg) -> QPushButton* {
        QPushButton* btn = new QPushButton(label);
        btn->setFixedHeight(44);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setStyleSheet(QString(
            "QPushButton { background:%1; color:%2; border:none; "
            "border-radius:8px; font-size:13px; font-weight:600; }"
            "QPushButton:hover { opacity:0.85; }")
            .arg(bg, fg));
        return btn;
    };

    m_btnPrev  = makeBtn("← Prev",    C_HOVER,  C_SEC);
    m_btnPrev->setFixedWidth(80);
    m_btnHard  = makeBtn("✕  Hard",   C_RED,    C_TEXT);
    m_btnKnown = makeBtn("✓  Known",  C_GREEN,  C_TEXT);

    connect(m_btnPrev,  &QPushButton::clicked, this, &FlashcardView::onPrev);
    connect(m_btnHard,  &QPushButton::clicked, this, &FlashcardView::onMarkHard);
    connect(m_btnKnown, &QPushButton::clicked, this, &FlashcardView::onMarkKnown);

    acts->addWidget(m_btnPrev);
    acts->addWidget(m_btnHard, 1);
    acts->addWidget(m_btnKnown, 1);
    cl->addLayout(acts);

    // Keyboard hints
    QLabel* hints = new QLabel(
        "Space = flip  ·  K = known  ·  H = hard  ·  ← → navigate");
    hints->setAlignment(Qt::AlignCenter);
    hints->setStyleSheet(
        QString("color:%1; font-size:10px;").arg(C_MUT));
    cl->addWidget(hints);

    m_mainStack->addWidget(m_cardPage);   // index 0

    // ── Completion page ──
    m_donePage = new QWidget;
    m_donePage->setStyleSheet(QString("background:%1;").arg(C_BG));
    QVBoxLayout* dl = new QVBoxLayout(m_donePage);
    dl->setAlignment(Qt::AlignCenter);
    dl->setSpacing(20);
    dl->setContentsMargins(60, 60, 60, 60);

    QLabel* doneIcon = new QLabel("🎉");
    doneIcon->setAlignment(Qt::AlignCenter);
    doneIcon->setStyleSheet("font-size:48px;");
    dl->addWidget(doneIcon);

    QLabel* doneTit = new QLabel("Session Complete!");
    doneTit->setAlignment(Qt::AlignCenter);
    doneTit->setStyleSheet(
        QString("color:%1; font-size:24px; font-weight:700;").arg(C_TEXT));
    dl->addWidget(doneTit);

    QHBoxLayout* dRow = new QHBoxLayout;
    dRow->setSpacing(24);

    auto makeStat = [&](QLabel*& lbl, const QString& title,
                         const QString& color) {
        QWidget* box = new QWidget;
        box->setFixedSize(120, 80);
        box->setStyleSheet(QString(
            "background:%1; border-radius:10px; border:1px solid %2;")
            .arg(C_CARD, C_BORDER));
        QVBoxLayout* bl = new QVBoxLayout(box);
        lbl = new QLabel("0");
        lbl->setAlignment(Qt::AlignCenter);
        lbl->setStyleSheet(
            QString("color:%1; font-size:28px; font-weight:700;").arg(color));
        bl->addWidget(lbl);
        QLabel* t = new QLabel(title);
        t->setAlignment(Qt::AlignCenter);
        t->setStyleSheet(
            QString("color:%1; font-size:10px; font-weight:600;").arg(C_SEC));
        bl->addWidget(t);
        dRow->addWidget(box);
    };

    makeStat(m_doneKnown, "KNOWN",  C_GREEN);
    makeStat(m_doneHard,  "HARD",   C_RED);
    dl->addLayout(dRow);

    QHBoxLayout* dBtns = new QHBoxLayout;
    dBtns->setSpacing(12);
    m_btnAgain = new QPushButton("🔄  Again");
    m_btnHome  = new QPushButton("🏠  Home");
    for (auto* b : {m_btnAgain, m_btnHome}) {
        b->setFixedHeight(40);
        b->setCursor(Qt::PointingHandCursor);
        b->setStyleSheet(QString(
            "QPushButton { background:%1; color:%2; border:1px solid %3; "
            "border-radius:8px; font-size:13px; padding:0 20px; }"
            "QPushButton:hover { background:%4; }")
            .arg(C_CARD, C_TEXT, C_BORDER, C_HOVER));
    }
    connect(m_btnAgain, &QPushButton::clicked, this, [this]{
        startSession(m_lang, m_filter, m_mode);
    });
    connect(m_btnHome, &QPushButton::clicked, this, [this]{
        emit navigateTo("home");
    });
    dBtns->addWidget(m_btnAgain);
    dBtns->addWidget(m_btnHome);
    dl->addLayout(dBtns);

    m_mainStack->addWidget(m_donePage);   // index 1
    m_mainStack->setCurrentIndex(0);
}

// ════════════════════════════════════════════════════
//  EventFilter — card container click → flip
// ════════════════════════════════════════════════════

bool FlashcardView::eventFilter(QObject* obj, QEvent* event)
{
    if (obj == m_cardContainer &&
        event->type() == QEvent::MouseButtonRelease)
    {
        onFlip();
        return true;
    }
    return QWidget::eventFilter(obj, event);
}

// ════════════════════════════════════════════════════
//  Session эхлүүлэх
// ════════════════════════════════════════════════════

void FlashcardView::buildWordList()
{
    m_words.clear();
    UserProgress* prog = progress();
    const QVector<Word>& all = m_db->words(m_lang);

    for (const Word& w : all) {
        bool include = false;
        if (m_filter == "all")     include = true;
        else if (m_filter == "new")     include = prog->isNew(w.id);
        else if (m_filter == "hard")    include = prog->isHard(w.id);
        else if (m_filter == "learned") include = prog->isLearned(w.id);
        else include = (w.section == m_filter);   // HSK level

        if (include) m_words.append(w);
    }

    // Flash mode: sessionSize-ээр хязгаарлана, зөвхөн new/hard
    if (m_mode == SessionMode::Flash) {
        // Эхлээд hard, дараа new
        QVector<Word> hard, newW;
        for (const Word& w : m_words) {
            if (prog->isHard(w.id))    hard.append(w);
            else if (prog->isNew(w.id)) newW.append(w);
        }
        m_words.clear();
        m_words << hard << newW;
        int size = qMin(m_settings->sessionSize, m_words.size());
        m_words.resize(size);
    }

    // Review: зөвхөн hard
    if (m_mode == SessionMode::Review) {
        m_words.erase(
            std::remove_if(m_words.begin(), m_words.end(),
                [&](const Word& w){ return !prog->isHard(w.id); }),
            m_words.end());
    }
}

void FlashcardView::startSession(const QString& lang,
                                  const QString& filter,
                                  SessionMode    mode)
{
    m_lang   = lang;
    m_filter = filter;
    m_mode   = mode;
    m_index  = 0;
    m_flipped = false;
    m_known  = 0;
    m_hard   = 0;
    m_autoTimer->stop();

    buildWordList();

    if (m_words.isEmpty()) {
        // Хоосон session
        m_doneKnown->setText("0");
        m_doneHard->setText("0");
        m_mainStack->setCurrentIndex(1);
        return;
    }

    m_mainStack->setCurrentIndex(0);
    renderCard();
    setFocus();
}

// ════════════════════════════════════════════════════
//  Карт дүрслэх
// ════════════════════════════════════════════════════

void FlashcardView::renderCard()
{
    if (m_words.isEmpty()) return;
    const Word& w = m_words[m_index];
    bool isChinese = (m_lang == "zh");

    // Progress
    m_progressBar->setRange(0, m_words.size());
    m_progressBar->setValue(m_index);
    m_counterLbl->setText(
        QString("%1 / %2").arg(m_index + 1).arg(m_words.size()));

    // Front
    m_frontTag->setText(isChinese ? "CHINESE (HANZI)" : "ENGLISH");
    m_frontWord->setText(w.english);   // EN үг эсвэл Ханз

    // Reverse mode тохиргоо
    bool rev = m_settings && m_settings->reverseCards;
    if (rev) {
        m_frontTag->setText("MONGOLIAN");
        m_frontWord->setText(w.mongolian);
        m_backTag->setText(isChinese ? "CHINESE" : "ENGLISH");
        m_backTrans->setText(w.english);
    } else {
        m_backTag->setText("MONGOLIAN");
        m_backTrans->setText(w.mongolian);
    }

    // Pinyin
    m_backPinyin->setText(w.pinyin);
    bool hasPinyin = isChinese && !w.pinyin.isEmpty() &&
                     (!m_settings || m_settings->showPinyin);
    m_backPinyin->setVisible(hasPinyin && m_flipped);

    // Section badge
    m_backSection->setText(w.section);
    m_backSection->setVisible(!w.section.isEmpty() && m_flipped);

    // Flip харагдал
    m_frontHint->setVisible(!m_flipped);
    m_backTag->setVisible(m_flipped);
    m_backTrans->setVisible(m_flipped);
    m_backPinyin->setVisible(hasPinyin && m_flipped);
    m_backSection->setVisible(!w.section.isEmpty() && m_flipped);

    // Action button text
    m_btnKnown->setEnabled(m_flipped);
    m_btnHard->setEnabled(m_flipped);
    m_btnKnown->setStyleSheet(m_flipped ?
        QString("QPushButton { background:%1; color:%2; border:none; "
                "border-radius:8px; font-size:13px; font-weight:600; }")
        .arg(C_GREEN, C_TEXT) :
        QString("QPushButton { background:%1; color:%2; border:none; "
                "border-radius:8px; font-size:13px; }")
        .arg(C_HOVER, C_MUT));
    m_btnHard->setStyleSheet(m_flipped ?
        QString("QPushButton { background:%1; color:%2; border:none; "
                "border-radius:8px; font-size:13px; font-weight:600; }")
        .arg(C_RED, C_TEXT) :
        QString("QPushButton { background:%1; color:%2; border:none; "
                "border-radius:8px; font-size:13px; }")
        .arg(C_HOVER, C_MUT));

    m_btnPrev->setEnabled(m_index > 0);
}

// ════════════════════════════════════════════════════
//  Slots
// ════════════════════════════════════════════════════

void FlashcardView::onFlip()
{
    m_flipped = !m_flipped;
    renderCard();
}

void FlashcardView::onNext()
{
    if (m_index + 1 >= m_words.size()) {
        showCompletionScreen();
        return;
    }
    m_index++;
    m_flipped = false;
    renderCard();
}

void FlashcardView::onPrev()
{
    if (m_index > 0) {
        m_index--;
        m_flipped = false;
        renderCard();
    }
}

void FlashcardView::onMarkKnown()
{
    if (!m_flipped) { onFlip(); return; }
    if (m_words.isEmpty()) return;
    const Word& w = m_words[m_index];
    progress()->markLearned(w.id);
    m_known++;
    emit progressChanged();
    onNext();
}

void FlashcardView::onMarkHard()
{
    if (!m_flipped) { onFlip(); return; }
    if (m_words.isEmpty()) return;
    const Word& w = m_words[m_index];
    progress()->markHard(w.id);
    m_hard++;
    emit progressChanged();
    onNext();
}

void FlashcardView::onExit()
{
    m_autoTimer->stop();
    emit navigateTo("home");
}

void FlashcardView::showCompletionScreen()
{
    m_autoTimer->stop();
    m_doneKnown->setText(QString::number(m_known));
    m_doneHard->setText(QString::number(m_hard));
    m_mainStack->setCurrentIndex(1);
}

// ════════════════════════════════════════════════════
//  Keyboard shortcuts
//  Space=flip, K=known, H=hard, ←→=navigate, Esc=exit
// ════════════════════════════════════════════════════

void FlashcardView::keyPressEvent(QKeyEvent* event)
{
    switch (event->key()) {
    case Qt::Key_Space:  onFlip();      break;
    case Qt::Key_K:      onMarkKnown(); break;
    case Qt::Key_H:      onMarkHard();  break;
    case Qt::Key_Right:  onNext();      break;
    case Qt::Key_Left:   onPrev();      break;
    case Qt::Key_Escape: onExit();      break;
    default: QWidget::keyPressEvent(event);
    }
}
