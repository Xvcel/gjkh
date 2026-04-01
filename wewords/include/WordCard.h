#pragma once
#include <QWidget>
#include <QLabel>
#include <QMouseEvent>
#include "Word.h"

class WordCardWidget : public QWidget
{
    Q_OBJECT
public:
    explicit WordCardWidget(const Word& word, QWidget* parent = nullptr);
    const Word& word() const { return m_word; }

signals:
    void clicked(const Word& word);

protected:
    void mousePressEvent(QMouseEvent*) override;
    void enterEvent(QEnterEvent*)      override;
    void leaveEvent(QEvent*)           override;
    void paintEvent(QPaintEvent*)      override;

private:
    void setupUi();

    Word    m_word;
    bool    m_hovered = false;

    QLabel* m_wordLbl   = nullptr;
    QLabel* m_pinyinLbl = nullptr;
    QLabel* m_mnLbl     = nullptr;
    QLabel* m_badge     = nullptr;
    QLabel* m_secBadge  = nullptr;
};
