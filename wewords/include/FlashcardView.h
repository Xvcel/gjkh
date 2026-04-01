#pragma once
#include <QWidget>
#include <QVector>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>
#include <QTimer>
#include <QPropertyAnimation>
#include <QStackedWidget>
#include <QKeyEvent>

#include "Word.h"
#include "WordDatabase.h"
#include "UserProgress.h"
#include "AppSettings.h"

enum class SessionMode { Flash, Level, Quick, Queue, Review };

class FlashcardView : public QWidget
{
    Q_OBJECT
public:
    explicit FlashcardView(WordDatabase*  db,
                           UserProgress*  progressEN,
                           UserProgress*  progressZH,
                           AppSettings*   settings,
                           QWidget*       parent = nullptr);

    // MainWindow-оос дуудагдана
    void startSession(const QString& lang,
                      const QString& filter,
                      SessionMode    mode);

signals:
    void sessionFinished();
    void progressChanged();
    void navigateTo(const QString& view);

public slots:
    void onFlip();
    void onNext();
    void onPrev();
    void onMarkKnown();
    void onMarkHard();
    void onExit();

protected:
    void keyPressEvent(QKeyEvent* event) override;
    bool eventFilter(QObject* obj, QEvent* event) override;

private:
    void setupUi();
    void renderCard();
    void updateProgressBar();
    void showCompletionScreen();
    void animateFlip();
    void buildWordList();

    // Data
    WordDatabase*  m_db         = nullptr;
    UserProgress*  m_progressEN = nullptr;
    UserProgress*  m_progressZH = nullptr;
    AppSettings*   m_settings   = nullptr;
    QString        m_lang       = "en";
    QString        m_filter     = "all";
    SessionMode    m_mode       = SessionMode::Flash;

    QVector<Word>  m_words;
    int            m_index      = 0;
    bool           m_flipped    = false;
    int            m_known      = 0;
    int            m_hard       = 0;

    UserProgress* progress() const {
        return m_lang == "en" ? m_progressEN : m_progressZH;
    }

    // UI
    QStackedWidget*     m_mainStack     = nullptr;

    // Card widgets
    QWidget*            m_cardPage      = nullptr;
    QProgressBar*       m_progressBar   = nullptr;
    QLabel*             m_counterLbl    = nullptr;
    QPushButton*        m_btnExit       = nullptr;

    QWidget*            m_cardContainer = nullptr;
    QLabel*             m_frontTag      = nullptr;
    QLabel*             m_frontWord     = nullptr;
    QLabel*             m_frontHint     = nullptr;
    QLabel*             m_backTag       = nullptr;
    QLabel*             m_backPinyin    = nullptr;
    QLabel*             m_backTrans     = nullptr;
    QLabel*             m_backSection   = nullptr;

    QPushButton*        m_btnPrev       = nullptr;
    QPushButton*        m_btnHard       = nullptr;
    QPushButton*        m_btnKnown      = nullptr;

    // Completion
    QWidget*            m_donePage      = nullptr;
    QLabel*             m_doneKnown     = nullptr;
    QLabel*             m_doneHard      = nullptr;
    QPushButton*        m_btnAgain      = nullptr;
    QPushButton*        m_btnHome       = nullptr;

    QTimer*             m_autoTimer     = nullptr;
};
