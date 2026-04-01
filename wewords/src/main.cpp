#include <QApplication>
#include <QFont>
#include <QFontDatabase>
#include "MainWindow.h"

int main(int argc, char* argv[])
{
    // Windows DPI scaling
    QApplication::setHighDpiScaleFactorRoundingPolicy(
        Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);

    QApplication app(argc, argv);
    app.setApplicationName("WeWords");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("WeWords");

    // Dark theme palette (HTML-ийн --bg: #0d0d0d-тэй тохирно)
    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window,          QColor(0x0d, 0x0d, 0x0d));
    darkPalette.setColor(QPalette::WindowText,      QColor(0xed, 0xe9, 0xe3));
    darkPalette.setColor(QPalette::Base,            QColor(0x13, 0x13, 0x13));
    darkPalette.setColor(QPalette::AlternateBase,   QColor(0x19, 0x19, 0x19));
    darkPalette.setColor(QPalette::Text,            QColor(0xed, 0xe9, 0xe3));
    darkPalette.setColor(QPalette::Button,          QColor(0x20, 0x20, 0x20));
    darkPalette.setColor(QPalette::ButtonText,      QColor(0xed, 0xe9, 0xe3));
    darkPalette.setColor(QPalette::Highlight,       QColor(0xc9, 0xa9, 0x6e));
    darkPalette.setColor(QPalette::HighlightedText, QColor(0x0d, 0x0d, 0x0d));
    darkPalette.setColor(QPalette::BrightText,      QColor(0xff, 0xff, 0xff));
    darkPalette.setColor(QPalette::ToolTipBase,     QColor(0x28, 0x28, 0x28));
    darkPalette.setColor(QPalette::ToolTipText,     QColor(0xed, 0xe9, 0xe3));
    darkPalette.setColor(QPalette::PlaceholderText, QColor(0x44, 0x44, 0x40));
    app.setPalette(darkPalette);

    // Default font
    QFont appFont("Segoe UI", 9);
    appFont.setStyleHint(QFont::SansSerif);
    app.setFont(appFont);

    // Үндсэн цонх
    MainWindow window;
    window.setWindowTitle("WeWords — Vocabulary Studio");
    window.setMinimumSize(900, 600);
    window.resize(1200, 750);
    window.show();

    return app.exec();
}
