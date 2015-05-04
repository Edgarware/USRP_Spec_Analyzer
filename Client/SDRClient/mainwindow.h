#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include "qcustomplot.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    bool engCheck(QString in);
    bool isSignal(float power, float noise);
    QString identify(QString buffer);
    void clearPlot();

signals:
    void dataWriten(QString f_up, QString f_down);
    void onError(QString title, QString err);

public slots:
    void closeUI();
    void dataParse(QString buffer);
    void printError(QString title, QString str);

private slots:
    void on_apply_clicked();
    void on_reset_clicked();
    void on_startFreq_textEdited(const QString &arg1);
    void on_endFreq_textEdited(const QString &arg1);
    void on_expand_clicked();
    void on_filebool_toggled(bool checked);
    void on_filter_toggled(bool checked);
    void on_unfilter_toggled(bool checked);
    void on_endFreq_returnPressed();

private:
    Ui::MainWindow *ui;
    QMessageBox *err;
    QCPItemText *textLabel;
    QFile *outFile;
    float lastFreq;
    bool applyFreq;
    bool fileOpen;
    bool idenAccountedFor;
    QString end_temp, start_temp;
    QString end_freq, start_freq;
    //QVector<double> noiseVector;
    //QVector<double> powerVector;
    //QVector<double> freqVector;
};

#endif // MAINWINDOW_H
