#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QPushButton>
#include <QTextEdit>
#include <QLineEdit>
#include <QSplitter>
#include <QVector>

#include <QtCharts>

struct DataPoint {
    QString name;
    double value;
};

QT_CHARTS_USE_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

    QVector<DataPoint> secondData;

    QChartView *chartView;


public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void loadFile();
    void analyzeData();
    void handleChat();

    void loadSecondFile();
    void exportReport();
    void showGraph();
    QString compareData();


private:
    QVector<DataPoint> data;

    QTableWidget *table;
    QTextEdit *output;
    QTextEdit *chatBox;
    QLineEdit *chatInput;

    void setupUI();
    void populateTable();
    QString runAnalysis();
    QString processQuery(QString query);
    int highCount = 0;
    int lowCount = 0;
};

#endif // MAINWINDOW_H
