#pragma once
#include <QMainWindow>
#include <QStackedWidget>
#include <QPushButton>
#include <QLabel>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include "Word.h"
#include "AppSettings.h"
#include "WordDatabase.h"
#include "UserProgress.h"

class FlashcardView;
class BrowseView;
class StatsView;
class SettingsDialog;

// ── Үндсэн цонх ──
// HTML-ийн #shell → sidebar + #main бүтцийг хэрэгжүүлнэ.
// Sidebar: logo, хэл солих, навигаци, шүүлт товчнууд, явц
// Main:    topbar + content area (QStackedWidget)

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override = default;

    // External call: view шилжүүлэх
    void navigateTo(const QString& view);

public slots:
    void onLanguageChanged(const QString& lang);
    void onNavigate(const QString& view);
    void onSideFilterChanged(const QString& filter);
    void onProgressChanged();
    void onOpenSettings();

protected:
    void closeEvent(QCloseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    bool eventFilter(QObject* obj, QEvent* event) override;

private:
    // ── UI байгуулалт ──
    void setupUi();
    void setupSidebar();
    void setupTopbar();
    void setupContentArea();
    void setupStyles();

    // ── Sidebar ──
    QWidget*        buildSidebar();
    QWidget*        buildSidebarLogo();
    QWidget*        buildSidebarNav();
    QWidget*        buildSidebarFilters();
    QWidget*        buildSidebarStats();
    void            buildHSKFilters(QVBoxLayout* layout);

    // ── Sidebar update ──
    void updateSidebarStats();
    void updateSidebarFilters();
    void setSidebarActiveNav(const QString& view);
    void setSidebarActiveFilter(const QString& filter);

    // ── Topbar ──
    void updateTopbar(const QString& view);

    // ── Helper ──
    QPushButton* makeSidebarNavBtn(const QString& icon,
                                   const QString& label,
                                   const QString& viewName);
    QPushButton* makeSidebarFilterBtn(const QString& icon,
                                      const QString& label,
                                      const QString& filterName,
                                      const QString& pillId);

    // ── Өгөгдөл болон тохиргоо ──
    WordDatabase*   m_db        = nullptr;
    UserProgress*   m_progressEN = nullptr;
    UserProgress*   m_progressZH = nullptr;
    AppSettings     m_settings;
    QString         m_currentLang   = "en";
    QString         m_currentView   = "home";
    QString         m_currentFilter = "all";

    UserProgress* progress() const {
        return m_currentLang == "en" ? m_progressEN : m_progressZH;
    }

    // ── Sidebar widgets ──
    QWidget*        m_sidebar       = nullptr;
    QPushButton*    m_btnLangEN     = nullptr;
    QPushButton*    m_btnLangZH     = nullptr;

    // Nav buttons
    QPushButton*    m_navHome       = nullptr;
    QPushButton*    m_navBrowse     = nullptr;
    QPushButton*    m_navFlash      = nullptr;
    QPushButton*    m_navReview     = nullptr;
    QPushButton*    m_navStats      = nullptr;

    // Filter buttons
    QPushButton*    m_filterAll     = nullptr;
    QPushButton*    m_filterNew     = nullptr;
    QPushButton*    m_filterHard    = nullptr;
    QPushButton*    m_filterLearned = nullptr;
    QPushButton*    m_filterHSK[6]  = {};      // HSK 1..6

    // Filter pill labels (тоо харуулах)
    QLabel*         m_pillAll       = nullptr;
    QLabel*         m_pillNew       = nullptr;
    QLabel*         m_pillHard      = nullptr;
    QLabel*         m_pillLearned   = nullptr;
    QLabel*         m_pillHSK[6]    = {};

    // HSK section (зөвхөн ZH хэлэнд харагдана)
    QWidget*        m_hskSection    = nullptr;

    // Sidebar stats
    QLabel*         m_sbPct         = nullptr;
    QProgressBar*   m_sbBar         = nullptr;
    QLabel*         m_sbToday       = nullptr;
    QLabel*         m_sbGoal        = nullptr;
    QLabel*         m_sbStreak      = nullptr;

    // ── Topbar widgets ──
    QWidget*        m_topbar        = nullptr;
    QLabel*         m_topHeading    = nullptr;
    QLabel*         m_topSub        = nullptr;
    QPushButton*    m_btnSettings   = nullptr;

    // ── Content area ──
    QStackedWidget* m_stack         = nullptr;
    FlashcardView*  m_flashView     = nullptr;
    BrowseView*     m_browseView    = nullptr;
    StatsView*      m_statsView     = nullptr;
    QWidget*        m_homeView      = nullptr;

    // Home view widgets
    QLabel*         m_homeGreeting  = nullptr;
    QLabel*         m_homeSub       = nullptr;
    QLabel*         m_homeTotal     = nullptr;
    QLabel*         m_homeLearned   = nullptr;
    QLabel*         m_homeHard      = nullptr;
    QLabel*         m_homeToday     = nullptr;
    QWidget*        m_levelSection  = nullptr;

    QWidget* buildHomeView();
    void     updateHomeView();
    void     buildLevelBars(QVBoxLayout* layout);

    // ── Settings ──
    SettingsDialog* m_settingsDialog = nullptr;

    // ── Style helper ──
    static QString sidebarBtnStyle(bool active, bool gold = false);
    static QString filterBtnStyle (bool active, bool gold = false);
};
