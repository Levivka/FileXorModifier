#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QFile>
#include <QTextStream>
#include <QByteArray>
#include <QMessageBox>
#include <QDebug>
#include <QFileInfo>
#include<QDir>
#include<QTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    timer = new QTimer(this);

    //Начальные данные элементов
    setWindowTitle("Шифрование файлов");

    ui->nameRepeat->addItem("Добавить счетчик");
    ui->nameRepeat->addItem("Перезаписать");

    ui->executeMode->addItem("Разовый");
    ui->executeMode->addItem("По таймеру");

    ui->timer->setEnabled(false);

    ui->xorKey->setMaxLength(16);

    connect(ui->executeMode, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int index) {
        index == 1 ? ui->timer->setEnabled(true) : ui->timer->setEnabled(false);
    });
    connect(ui->executeButton, &QPushButton::clicked, this, &MainWindow::ExecuteModeCheck);
    connect(ui->xorKey, &QLineEdit::textChanged, this, &MainWindow::ValidateXorKey);
    connect(timer, &QTimer::timeout, this, &MainWindow::Execute);
}

void MainWindow::ExecuteModeCheck() {
    if (ui->timer->isEnabled()) {
        int interval = ui->timer->value() * 1000;
        timer->setInterval(interval);
        timer->start();
    }
    else {
        Execute();
    }
}


void MainWindow::Execute() {
    QString xorKey = ui->xorKey->text();
    QString fileMask = ui->mask->text();
    QString outputDir = ui->outputDir->text();
    int fileOverWrite = ui->nameRepeat->currentIndex();

    QDir dir(QFileInfo(fileMask).absolutePath());
    QStringList files = dir.entryList({QFileInfo(fileMask).fileName()}, QDir::Files);
    if (files.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Файлы по заданной маске не найдены.");
        return;
    }

    for (const QString &fileName : files) {
        QString filePath = dir.absoluteFilePath(fileName);
        QByteArray fileData;

        // Валидация файла и загрузка данных
        if (!ValidateFile(filePath, fileData)) {
            continue;
        }

        QByteArray xorKeyBytes = QByteArray::fromHex(ui->xorKey->text().toUtf8());
        // XOR-операция
        QByteArray result = XorCalculation(fileData, xorKeyBytes);

        // Действия с файлами
        FileActions(filePath, result, fileOverWrite, outputDir);
    }


}

bool MainWindow::ValidateFile(const QString &filePath, QByteArray &fileData) {
    QFile file(filePath);

    //Проверки:
    //на существование файла
    if (!file.exists(filePath)) {
        QMessageBox::warning(this, "Ошибка", "Файла не существует по пути: " + filePath + '.');
        return false;
    }

    //На возможность открытия
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "Ошибка", "Не удалось открыть файл по пути: " + filePath + '.');
        return false;
    }

    //На наличие контента в нём + запись данных
    fileData = file.readAll();
    if (fileData.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Файл: " + filePath + " пустой.");
        return false;
    }
    file.close();

    return true;
}

void MainWindow::ValidateXorKey(const QString &xorKey) {
    QString xorKeyText = ui->xorKey->text().toUpper();
    xorKeyText.remove(QRegularExpression("[^0-9A-F]"));
    ui->xorKey->setText(xorKeyText);
}

QByteArray MainWindow::XorCalculation(const QByteArray &input, const QByteArray &key)
{
    QByteArray result;
    int keyLength = key.size();

    // XOR операция
    for (int i = 0; i < input.size(); ++i) {
        result.append(input[i] ^ key[i % keyLength]);
    }
    return result;
}

void MainWindow::FileActions(const QString &filePath, const QByteArray &result, const int fileOverWrite, const QString outputDir)
{
    bool deleteFile = ui->fileDelete->isChecked();
    QFileInfo fileInfo(filePath);
    QString outputFilePath = outputDir + "/" + fileInfo.fileName();

    int fileId = 1;
    if(fileOverWrite == 0 && QFile(filePath).exists()) {
        while (QFile::exists(outputFilePath)) {
            outputFilePath = outputDir + QString("%1%2_%3.%4")
                                            .arg("/")
                                            .arg(fileInfo.baseName())
                                            .arg(fileId++)
                                            .arg(fileInfo.suffix());
        }
    }

    QFile outputFile(outputFilePath);
    if (!outputFile.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, "Ошибка", "Не удалось сохранить файл по пути: " + outputFilePath);
        return;
    }

    //Hex формат для записи в файл
    QByteArray hexResult = result.toHex(' ').toUpper();
    outputFile.write(hexResult);
    outputFile.close();

    if (deleteFile) {
        if (!QFile::remove(filePath)) {
            QMessageBox::warning(this, "Ошибка", "Не удалось удалить файл по пути: " + filePath);
        }
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}
