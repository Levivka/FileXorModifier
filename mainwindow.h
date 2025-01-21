#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
private slots:
    void Execute();

private:
    QTimer *timer;
    void ExecuteModeCheck();
    void ValidateXorKey(const QString &xorKeyValue);
    bool ValidateFile(const QString &filePath, QByteArray &fileData);
    QByteArray XorCalculation(const QByteArray &input, const QByteArray &key);
    void FileActions(const QString &filePath, const QByteArray &result, int fileOverWrite, const QString outputDir);

private:

    Ui::MainWindow *ui;
};


#endif // MAINWINDOW_H
