#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>

//#define NFLOOR 76
#define NFLOOR 111.214

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    //set up variables
    ui->setupUi(this);
    err = new QMessageBox(this);
    outFile = new QFile();
    textLabel = new QCPItemText(ui->graph);

    end_freq = "4.4e9";
    start_freq = "400e6";
    applyFreq = false;
    fileOpen = false;
    idenAccountedFor = false;
    lastFreq = -1;
    start_temp = start_freq;
    end_temp = end_freq;

    //Configure Graph
    ui->graph->addGraph();
    ui->graph->addGraph();
    ui->graph->graph(0)->setAdaptiveSampling(true);
    ui->graph->graph(1)->setAdaptiveSampling(true);
    ui->graph->graph(0)->setVisible(false);
    ui->graph->graph(1)->setVisible(true);
    ui->graph->graph(0)->setPen(QPen(Qt::red));
    ui->graph->xAxis->setLabel("Frequency (Hz)");
    ui->graph->yAxis->setLabel("Magnitude (dB)");
    ui->graph->xAxis->setRange(start_freq.toFloat(), end_freq.toFloat());
    ui->graph->yAxis->setRange(0, -120);
    ui->graph->setBackground(QWidget::palette().light());

    //Configure UI
    ui->hidden->hide();
    ui->iden->setChecked(true);
    ui->filter->setChecked(true);
    ui->unfilter->setChecked(false);

    //Configure "Applying" Graph Label
    ui->graph->addItem(textLabel);
    textLabel->setPositionAlignment(Qt::AlignTop|Qt::AlignHCenter);
    textLabel->position->setType(QCPItemPosition::ptAxisRectRatio);
    textLabel->position->setCoords(0.5, 0.5);
    textLabel->setText("Applying Settings");
    textLabel->setFont(QFont(font().family(), 16)); // make font a bit larger
    textLabel->setVisible(false);
    ui->graph->replot();

}

MainWindow::~MainWindow()
{
    //Free pointers
    delete ui;
    delete err;
    delete textLabel;
    delete outFile;
}

void MainWindow::printError(QString title, QString str){
    err->critical(0,title,str);
}

void MainWindow::closeUI()
{
    if(fileOpen == true){ //Close file before we exit
        outFile->close();
        qDebug() << "File Closed";
    }
}

/******* SIGNAL PROCESSING CLASSES ******/
void MainWindow::dataParse(QString buffer)
{
    //Split packet
    QStringList pieces = buffer.split(" ");
    QString identity = "";

    if(pieces.size() == 4 ){ //Sanity checks

        //Get graph bounds for future use
        double f_upper = ui->graph->xAxis->range().upper;
        double f_lower = ui->graph->xAxis->range().lower;

        if(applyFreq == true){ //If we're getting new start/end frequencies
            if(pieces[1].toFloat() == start_freq.toFloat()){
                applyFreq = false;
                textLabel->setVisible(false); //Re-Hide the "Applying" text
            }
        }
        else{

            //Filtered Signal
            if(isSignal(pieces[2].toFloat(),pieces[3].toFloat())){
                ui->graph->graph(1)->addData(pieces[1].toFloat(), pieces[2].toFloat());
                if(ui->iden->isChecked()){
                    identity = identify(pieces[1]);
                }
            }
            else{
                ui->graph->graph(1)->addData(pieces[1].toFloat(),-NFLOOR);
                if(ui->iden->isChecked())
                    identity = "N/A";
                idenAccountedFor = false;
            }

            ui->graph->graph(0)->addData(pieces[1].toFloat(), pieces[2].toFloat());

            //Axis rescaling code
            if(ui->filter->isChecked() && ui->unfilter->isChecked()){
                ui->graph->graph(1)->rescaleValueAxis();
                ui->graph->graph(0)->rescaleValueAxis();
            }
            else if(ui->filter->isChecked()){
                ui->graph->graph(1)->rescaleValueAxis();
            }
            else if(ui->unfilter->isChecked()){
                ui->graph->graph(0)->rescaleValueAxis();
            }

            if(pieces[1].toFloat() > f_upper){ //We've exceeded the right of the graph (Should never happen)
                f_upper = pieces[1].toFloat();
                ui->graph->xAxis->setRange(f_lower,f_upper);
                qDebug() << "Exceeded graph";
            }
            if(pieces[1].toFloat() < f_lower){ //We've exceeded the left of the graph (Should even less happen)
                f_lower = pieces[1].toFloat();
                ui->graph->xAxis->setRange(f_lower,f_upper);
                qDebug() << "De-ceeded graph";
            }

            if(lastFreq != -1 && lastFreq > pieces[1].toFloat()){ //We've gone backwards in frequency, probably a roll over
                //Flush data to image if requested
                if(ui->imageBool->isChecked()){
                    QString dt = QDateTime::currentDateTime().toString("yyyy-MM-dd_hh:mm:ss:zz");
                    QString filename = "D_" + start_freq + "_" + end_freq + "_" + dt + "_" + ".png";
                    ui->graph->savePng(filename,2000,1000,1,-1);
                }
                clearPlot();
            }
            lastFreq = pieces[1].toFloat(); //Store to compare to next packet

            //Write to file
            if(fileOpen == true){
                QTextStream out(outFile);
                out << pieces[1] << "\t" << pieces[2] << "\t" << identity << "\n";
            }
        }
        ui->graph->replot();
        QCoreApplication::processEvents(); //We spend a lot of time here, lets just process events to be safe...
    }
}

bool MainWindow::engCheck(QString in)
{
    QRegExp rx("(\\e|\\E)");
    QStringList pieces = in.split( rx );
    if(pieces.size() == 1){ //assuming raw number
        float temp = pieces[0].toFloat();
        if( temp > 4400000000.0 || temp < 400000000.0)
            return false;
    }
    else if(pieces.size() == 2){ //assuming e
        float temp1 = pieces[0].toFloat();
        float temp2 = pieces[1].toFloat();
        float temp = temp1*pow(10, temp2);
        if( temp > 4400000000.0 || temp < 400000000.0)
            return false;
    }
    else
        return false;
    return true;
}

QString MainWindow::identify(QString buffer){
    if ((buffer.toFloat() > 400*pow(10.0,6.0)) && (buffer.toFloat() <406*pow(10.0,6.0))){
        return ("Time SIG Freq, Space Rsrch, MET.AIDS");
    }
    if ((buffer.toFloat() > 406*pow(10.0,6.0)) && (buffer.toFloat() <420*pow(10.0,6.0))){
        return ("Mobile SAT, Space Rsrch");
    }
    if ((buffer.toFloat() > 420*pow(10.0,6.0)) && (buffer.toFloat() <450*pow(10.0,6.0))){
        return ("Radio LOC, Amateur");
    }
    if ((buffer.toFloat() > 450*pow(10.0,6.0)) && (buffer.toFloat() <470*pow(10.0,6.0))){
        return ("Land Mobile, MET.SAT");
    }
    if ((buffer.toFloat() > 470*pow(10.0,6.0)) && (buffer.toFloat() <608*pow(10.0,6.0))){
        return ("TV Broadcast,");
    }
    if ((buffer.toFloat() > 608*pow(10.0,6.0)) && (buffer.toFloat() <614*pow(10.0,6.0))){
        return ("Land Mobile, Radio ASTRO");
    }
    if ((buffer.toFloat() > 614*pow(10.0,6.0)) && (buffer.toFloat() <698*pow(10.0,6.0))){
        return ("TV Broadcast,");
    }
    if ((buffer.toFloat() > 698*pow(10.0,6.0)) && (buffer.toFloat() <806*pow(10.0,6.0))){
        return ("3G, 4G");
    }
    if ((buffer.toFloat() > 806*pow(10.0,6.0)) && (buffer.toFloat() <902*pow(10.0,6.0))){
        return ("Aeronautical Mobile, Land Mobile");
    }
    if ((buffer.toFloat() > 902*pow(10.0,6.0)) && (buffer.toFloat() <928*pow(10.0,6.0))){
        return ("Radio LOC, Amateur");
    }
    if ((buffer.toFloat() > 928*pow(10.0,6.0)) && (buffer.toFloat() <960*pow(10.0,6.0))){
        return ("Land Mobile, FIXED");
    }
    if ((buffer.toFloat() > 960*pow(10.0,6.0)) && (buffer.toFloat() <1215*pow(10.0,6.0))){
        return ("Aeronautical RaidoNAV");
    }
    if ((buffer.toFloat() > 960*pow(10.0,6.0)) && (buffer.toFloat() <1215*pow(10.0,6.0))){
        return ("Aeronautical RaidoNAV");
    }
    if ((buffer.toFloat() > 1215*pow(10.0,6.0)) && (buffer.toFloat() <1390*pow(10.0,6.0))){
        return ("RdaioLOC, Aeronautical RadioNAV");
    }
    if ((buffer.toFloat() > 1390*pow(10.0,6.0)) && (buffer.toFloat() <1435*pow(10.0,6.0))){
        return ("Earth Expl, Land Mobile");
    }
    if ((buffer.toFloat() > 1435*pow(10.0,6.0)) && (buffer.toFloat() <1525*pow(10.0,6.0))){
        return ("Aeronautical TELEMETERING");
    }
    if ((buffer.toFloat() > 1525*pow(10.0,6.0)) && (buffer.toFloat() <1660*pow(10.0,6.0))){
        return ("Mobile SAT, Aeronautical Mobile SAT, Radio SAT");
    }
    if ((buffer.toFloat() > 1660*pow(10.0,6.0)) && (buffer.toFloat() <2200*pow(10.0,6.0))){
        return ("Astronomy, GSM, CDMA 3G 4G, ");
    }
    if ((buffer.toFloat() > 2200*pow(10.0,6.0)) && (buffer.toFloat() <2290*pow(10.0,6.0))){
        return ("Space OPS, Mobile, 4G");
    }
    if ((buffer.toFloat() > 2290*pow(10.0,6.0)) && (buffer.toFloat() <2320*pow(10.0,6.0))){
        return ("Amature, Radio LOC, Mobile");
    }
    if ((buffer.toFloat() > 2320*pow(10.0,6.0)) && (buffer.toFloat() <2400*pow(10.0,6.0))){
        return ("Radio LOC, Mobile, BCST SAT");
    }
    if ((buffer.toFloat() > 2400*pow(10.0,6.0)) && (buffer.toFloat() <2500*pow(10.0,6.0))){
        return ("Wireless Communication Service");
    }
    if ((buffer.toFloat() > 2500*pow(10.0,6.0)) && (buffer.toFloat() <3000*pow(10.0,6.0))){
        return ("Marine time NAV, RadioLOC, MET.AIDS, AeronautNAV");
    }
    if ((buffer.toFloat() > 3000*pow(10.0,6.0)) && (buffer.toFloat() <3650*pow(10.0,6.0))){
        return ("RadioLOC, Marine Time, AeronautNAV");
    }
    if ((buffer.toFloat() > 3650*pow(10.0,6.0)) && (buffer.toFloat() <4000*pow(10.0,6.0))){
        return ("Mobile Fixed Sat");
    }
    else{
        return ("Other");
    }
}

bool MainWindow::isSignal(float power, float noise){
    if(noise > -NFLOOR)
        return true;
    else
        return false;
}

/******* UI CONTROL CLASSES ******/
void MainWindow::clearPlot(){
    //Since we've rolled over, start a new file
    if(fileOpen){
        QString dt = QDateTime::currentDateTime().toString("yyyy-MM-dd_hh:mm:ss:zz");
        QString filename = "D_" + start_freq + "_" + end_freq + "_" + dt + "_" + ".txt";
        outFile->close();
        outFile->setFileName(filename);
        if (!outFile->open(QIODevice::WriteOnly | QIODevice::Text)){
            emit onError("File Error","Could not open file " + filename + " for writing");
            fileOpen = false;
        }
        else{
            QTextStream out(outFile); //Write a header
            out << "Frequency\tMagnitude\tIdentity\n";
        }
    }
    ui->graph->graph(0)->clearData();
    ui->graph->graph(1)->clearData();
}

void MainWindow::on_reset_clicked()
{
    emit dataWriten(end_freq, start_freq);
    applyFreq = true; //Applying new bounds flag
    textLabel->setVisible(true); //unhide the "Applying" Text
    ui->graph->xAxis->setRange(start_freq.toFloat(), end_freq.toFloat()); //set the range to the new bounds
    clearPlot();
    lastFreq = -1;
}

void MainWindow::on_apply_clicked()
{
    //Check to make sure entered values are actually numbers we can use
    bool sCheck = engCheck(start_temp);
    bool eCheck = engCheck(end_temp);

    if(sCheck && eCheck){
        end_freq = end_temp;
        start_freq = start_temp;
        applyFreq = true; //Applying new bounds flag
        textLabel->setVisible(true); //unhide the "Applying" Text
        ui->graph->xAxis->setRange(start_freq.toFloat(), end_freq.toFloat()); //set the range to the new bounds
        clearPlot();
        lastFreq = -1; //reset previous frequency data
        emit dataWriten(end_freq, start_freq);
    }
    else{
        emit onError("Invalid Frequencies","Only Frequencies between 400MHz and 4.4GHz are supported. Please use correct numbers or use Scientific Format (ex: 400e6)");
    }
}

void MainWindow::on_endFreq_returnPressed()
{
    //Check to make sure entered values are actually numbers we can use
    bool sCheck = engCheck(start_temp);
    bool eCheck = engCheck(end_temp);

    if(sCheck && eCheck){
        end_freq = end_temp;
        start_freq = start_temp;
        applyFreq = true; //Applying new bounds flag
        textLabel->setVisible(true); //unhide the "Applying" Text
        ui->graph->xAxis->setRange(start_freq.toFloat(), end_freq.toFloat()); //set the range to the new bounds
        ui->graph->graph(0)->clearData(); //clear graph
        ui->graph->graph(1)->clearData();
        lastFreq = -1; //reset previous frequency data
        emit dataWriten(end_freq, start_freq);
    }
    else{
        emit onError("Invalid Frequencies","Only Frequencies between 400MHz and 4.4GHz are supported. Please use correct numbers or use Scientific Format (ex: 400e6)");
    }
}


void MainWindow::on_startFreq_textEdited(const QString &arg1)
{
    start_temp = arg1.toLocal8Bit().constData();
}

void MainWindow::on_endFreq_textEdited(const QString &arg1)
{
    end_temp = arg1.toLocal8Bit().constData();
}

void MainWindow::on_expand_clicked()
{
    if(ui->hidden->isHidden()){
        ui->hidden->show();
    }
    else{
        ui->hidden->hide();
    }
}

void MainWindow::on_filebool_toggled(bool checked)
{
    if(checked){
        QString dt = QDateTime::currentDateTime().toString("yyyy-MM-dd_hh:mm:ss:zz");
        QString filename = "D_" + start_freq + "_" + end_freq + "_" + dt + "_" + ".txt";
        outFile->setFileName(filename);
        if (!outFile->open(QIODevice::WriteOnly | QIODevice::Text)){
            emit onError("File Error","Could not open file " + filename + " for writing");
            fileOpen = false;
            return;
        }
        QTextStream out(outFile); //Write a header
        out << "Frequency\tMagnitude\tIdentity\n";
        fileOpen = true;
    }
    else{
        outFile->close();
        fileOpen = false;
    }
}

void MainWindow::on_filter_toggled(bool checked)
{
    if(checked == true){
        ui->graph->graph(1)->setVisible(true);
    }
    else{
        ui->graph->graph(1)->setVisible(false);
    }
}

void MainWindow::on_unfilter_toggled(bool checked)
{
    if(checked == true){
        ui->graph->graph(0)->setVisible(true);
    }
    else{
        ui->graph->graph(0)->setVisible(false);
    }
}
