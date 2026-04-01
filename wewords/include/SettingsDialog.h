#pragma once
#include <QDialog>
#include <QCheckBox>
#include <QComboBox>
#include <QPushButton>
#include <QSpinBox>
#include "AppSettings.h"
#include "UserProgress.h"

class SettingsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SettingsDialog(AppSettings*   settings,
                            UserProgress*  progressEN,
                            UserProgress*  progressZH,
                            QWidget*       parent = nullptr);

    AppSettings getSettings() const;

private slots:
    void onResetAll();
    void onResetLearned();

private:
    void setupUi();
    QWidget* makeRow(const QString& label, const QString& sub, QWidget* ctrl);

    AppSettings*   m_settings   = nullptr;
    UserProgress*  m_progressEN = nullptr;
    UserProgress*  m_progressZH = nullptr;

    QCheckBox*  m_reverse    = nullptr;
    QCheckBox*  m_autoAdv    = nullptr;
    QSpinBox*   m_sessSize   = nullptr;
    QSpinBox*   m_dailyGoal  = nullptr;
    QCheckBox*  m_pinyin     = nullptr;
    QPushButton* m_okBtn     = nullptr;
};
