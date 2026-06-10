#include "mainwindow.h"

#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUI();
}

MainWindow::~MainWindow() {}

void MainWindow::setupUI()
{
    QWidget *central = new QWidget;
    QHBoxLayout *mainLayout = new QHBoxLayout;

    table = new QTableWidget;
    table->setColumnCount(3);
    table->setHorizontalHeaderLabels({"Parameter", "Value", "Status"});
    table->horizontalHeader()->setStretchLastSection(true);

    QVBoxLayout *rightLayout = new QVBoxLayout;

    QPushButton *btnLoad = new QPushButton("Load CSV");
    QPushButton *btnAnalyze = new QPushButton("Analyze");

    output = new QTextEdit;
    output->setReadOnly(true);
    output->setPlaceholderText("Analysis Output...");

    chatBox = new QTextEdit;
    chatBox->setReadOnly(true);

    chatInput = new QLineEdit;
    chatInput->setPlaceholderText("Ask AI... (anomaly / summary / max)");

    QPushButton *btnSend = new QPushButton("Send");

    QHBoxLayout *chatLayout = new QHBoxLayout;
    chatLayout->addWidget(chatInput);
    chatLayout->addWidget(btnSend);

    rightLayout->addWidget(btnLoad);
    rightLayout->addWidget(btnAnalyze);
    rightLayout->addWidget(output);
    rightLayout->addWidget(chatBox);
    rightLayout->addLayout(chatLayout);

    QSplitter *splitter = new QSplitter;
    splitter->addWidget(table);

    QWidget *rightWidget = new QWidget;
    rightWidget->setLayout(rightLayout);

    splitter->addWidget(rightWidget);
    splitter->setStretchFactor(0, 3);
    splitter->setStretchFactor(1, 2);

    mainLayout->addWidget(splitter);
    central->setLayout(mainLayout);
    setCentralWidget(central);

    QPushButton *btnLoad2 = new QPushButton("Compare File");
    QPushButton *btnGraph = new QPushButton("Show Graph");
    QPushButton *btnExport = new QPushButton("Export Report");

    rightLayout->addWidget(btnLoad2);
    rightLayout->addWidget(btnGraph);
    rightLayout->addWidget(btnExport);

    setWindowTitle("InsightFlow - Smart Analyzer");

    connect(btnLoad, &QPushButton::clicked, this, &MainWindow::loadFile);
    connect(btnAnalyze, &QPushButton::clicked, this, &MainWindow::analyzeData);
    connect(btnSend, &QPushButton::clicked, this, &MainWindow::handleChat);
    connect(btnLoad2, &QPushButton::clicked, this, &MainWindow::loadSecondFile);
    connect(btnGraph, &QPushButton::clicked, this, &MainWindow::showGraph);
    connect(btnExport, &QPushButton::clicked, this, &MainWindow::exportReport);

    output->setStyleSheet("background:#111;color:#00FFAA;font-size:14px;");
    chatBox->setStyleSheet("background:#1e1e1e;color:white;");
    table->setStyleSheet("QTableWidget {gridline-color: #444;}");

    setStyleSheet("QPushButton {padding:6px;font-weight:bold;}");
}

void MainWindow::loadFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Open CSV", "", "*.csv");

    if(fileName.isEmpty()) return;

    QFile file(fileName);
    if(!file.open(QIODevice::ReadOnly)) return;

    data.clear();

    QTextStream in(&file);

    while(!in.atEnd()) {
        QString line = in.readLine();
        QStringList parts = line.split(",");

        if(parts.size() >= 2) {
            DataPoint d;
            d.name = parts[0];
            d.value = parts[1].toDouble();
            data.push_back(d);
        }
    }

    populateTable();
}

void MainWindow::populateTable()
{
    table->setRowCount(data.size());

    for(int i = 0; i < data.size(); i++) {
        table->setItem(i, 0, new QTableWidgetItem(data[i].name));
        table->setItem(i, 1, new QTableWidgetItem(QString::number(data[i].value)));

        QTableWidgetItem *status = new QTableWidgetItem;


        if(data[i].value > 80) {
            status->setText("HIGH");
            status->setBackground(Qt::red);
        }
        else if(data[i].value < 10) {
            status->setText("LOW");
            status->setBackground(Qt::yellow);
        }
        else {
            status->setText("OK");
            status->setBackground(Qt::green);
        }

        table->setItem(i, 2, status);
    }
}

QString MainWindow::runAnalysis()
{
    QString result;
    highCount = 0;
    lowCount = 0;

    for(int i = 0; i < data.size(); i++)
    {
        if(data[i].value > 80) {
            result += "⚠ " + data[i].name + " -> HIGH\n";
            highCount++;

            table->item(i,1)->setBackground(Qt::red);
        }
        else if(data[i].value < 10) {
            result += "⚠ " + data[i].name + " -> LOW\n";
            lowCount++;

            table->item(i,1)->setBackground(Qt::yellow);
        }
        else {
            table->item(i,1)->setBackground(Qt::green);
        }
    }

    result += "\n=== SUMMARY ===\n";
    result += "Total: " + QString::number(data.size()) + "\n";
    result += "High: " + QString::number(highCount) + "\n";
    result += "Low: " + QString::number(lowCount) + "\n";
    result += "\n=== AI INSIGHTS ===\n";

    if(highCount > 2)
        result += "⚠ Multiple high values → potential instability\n";

    if(lowCount > 2)
        result += "⚠ Many low values → underperformance risk\n";

    if(highCount == 0 && lowCount == 0)
        result += "✅ System stable & optimized\n";
        result += "\n✅ System looks stable";

    return result;
}

void MainWindow::analyzeData()
{
    QString res = runAnalysis();
    output->setText(res);
}

QString MainWindow::processQuery(QString query)
{
    query = query.toLower();

    if(query.contains("anomaly"))
        return runAnalysis();

    if(query.contains("summary"))
        return "Total: " + QString::number(data.size()) +
               " | High: " + QString::number(highCount) +
               " | Low: " + QString::number(lowCount);

    if(query.contains("suggest"))
    {
        if(highCount > 0 || lowCount > 0)
            return "Adjust values: keep range between 10 and 80";
        else
            return "Everything already optimized ✅";
    }

    if(query.contains("max")) {
        double maxVal = 0;
        QString name;
        for(auto d : data)
            if(d.value > maxVal){
                maxVal = d.value;
                name = d.name;
            }

        return "Max: " + name + " = " + QString::number(maxVal);
    }

    return "Try: anomaly / summary / suggest / max";
}


void MainWindow::handleChat()
{
    QString input = chatInput->text();
    if(input.isEmpty()) return;

    chatBox->append("You: " + input);

    QString reply = processQuery(input);

    chatBox->append("AI: " + reply);

    chatInput->clear();
}

QString MainWindow::compareData()
{
    QString result = "=== FILE COMPARISON ===\n";

    int size = qMin(data.size(), secondData.size());

    for(int i=0;i<size;i++)
    {
        if(data[i].value != secondData[i].value)
        {
            result += data[i].name + ": "
                    + QString::number(data[i].value)
                    + " → "
                    + QString::number(secondData[i].value)
                    + "\n";
        }
    }

    return result;
}

void MainWindow::exportReport()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Save Report", "", "*.txt");

    if(fileName.isEmpty()) return;

    QFile file(fileName);
    if(!file.open(QIODevice::WriteOnly)) return;

    QTextStream out(&file);

    out << "=== InsightFlow Report ===\n\n";
    out << runAnalysis();

    file.close();

    output->append("\n✅ Report exported");
}


void MainWindow::loadSecondFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Open Second CSV", "", "*.csv");

    if(fileName.isEmpty()) return;

    QFile file(fileName);
    if(!file.open(QIODevice::ReadOnly)) return;

    secondData.clear();

    QTextStream in(&file);

    while(!in.atEnd()) {
        QStringList parts = in.readLine().split(",");
        if(parts.size() >= 2) {
            DataPoint d;
            d.name = parts[0];
            d.value = parts[1].toDouble();
            secondData.push_back(d);
        }
    }

    QString diff = compareData();
    output->setText(diff);
}

void MainWindow::showGraph()
{
    QLineSeries *series = new QLineSeries;
    series->setPointsVisible(true);

    series->setName("Values");

    for(int i=0;i<data.size();i++)
    {
        series->append(i, data[i].value);
    }

    QChart *chart = new QChart;
    chart->addSeries(series);
    chart->setTitle("Data Trend (Parameter vs Value)");

    QCategoryAxis *axisX = new QCategoryAxis;

    for(int i=0;i<data.size();i++)
    {
        axisX->append(data[i].name, i);
    }

    axisX->setTitleText("Parameters");

    QValueAxis *axisY = new QValueAxis;
    axisY->setTitleText("Values");

    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);

    series->attachAxis(axisX);
    series->attachAxis(axisY);

    chart->legend()->setVisible(true);

    QChartView *view = new QChartView(chart);
    view->setRenderHint(QPainter::Antialiasing);

    view->resize(700, 400);
    view->setWindowTitle("Insight Graph");
    view->show();
}

