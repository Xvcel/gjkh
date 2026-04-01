#pragma once
#include <QWidget>
#include <QLabel>
#include <QProgressBar>
#include <QScrollArea>
#include <QPainter>

#include "WordDatabase.h"
#include "UserProgress.h"
#include "AppSettings.h"

// 14 хоногийн bar chart
class DailyHistoryChart : public QWidget
{
    Q_OBJECT
public:
    explicit DailyHistoryChart(QWidget* parent = nullptr);
    void setData(UserProgress* prog, int dailyGoal);
protected:
    void paintEvent(QPaintEvent*) override;
private:
    struct Bar { QString lbl; int val; bool today; };
    QVector<Bar> m_bars;
    int m_max = 1, m_goal = 28;
};

class StatsView : public QWidget
{
    Q_OBJECT
public:
    explicit StatsView(WordDatabase*  db,
                       UserProgress*  progressEN,
                       UserProgress*  progressZH,
                       AppSettings*   settings,
                       QWidget*       parent = nullptr);

    void refresh(const QString& lang);

private:
    void setupUi();

    WordDatabase*  m_db         = nullptr;
    UserProgress*  m_progressEN = nullptr;
    UserProgress*  m_progressZH = nullptr;
    AppSettings*   m_settings   = nullptr;
    QString        m_lang       = "en";

    UserProgress* progress() const {
        return m_lang == "en" ? m_progressEN : m_progressZH;
    }

    // Overall
    QLabel*             m_pctLbl     = nullptr;
    QLabel*             m_learnSub   = nullptr;
    QProgressBar*       m_bar        = nullptr;
    QLabel*             m_detLearn   = nullptr;
    QLabel*             m_detHard    = nullptr;
    QLabel*             m_detNew     = nullptr;
    QLabel*             m_detToday   = nullptr;
    QLabel*             m_streakLbl  = nullptr;

    // Chart
    DailyHistoryChart*  m_chart      = nullptr;

    // HSK level section
    QWidget*            m_levelCard  = nullptr;
    QWidget*            m_levelRows  = nullptr;
};
