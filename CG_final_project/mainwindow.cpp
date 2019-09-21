#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QString>
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("Fu Kejie's Interval Scanline Z-Buffer");
}

MainWindow::~MainWindow()
{
    delete ui;
}



void MainWindow::on_actionOpen_triggered()
{
    ui->visualWindow->goPaint = 0;// stop paint window
    QString filename = QFileDialog::getOpenFileName(this,tr("Open File"), ".", tr("OBJ Files(*.obj)"));

    ui->visualWindow->Z_Buffer.readOBJ(filename);
    ui->visualWindow->Z_Buffer.zoom();

}

void MainWindow::on_pushButton_dispaly_clicked()
{
    ui->visualWindow->goPaint = 1;
    ui->visualWindow->update();
    QString str = "Total polygons =" + QString::number(ui->visualWindow->Z_Buffer.nFaces);
    ui->label_npoly->setText(str);
    QString str1 = "Time used in Z-buffer =" + QString::number(ui->visualWindow->timeused)+"us";
    ui->label_time->setText(str1);
}


void MainWindow::on_pushButton_zoom_clicked()
{
        ui->visualWindow->zoom();
        update();
}

void MainWindow::on_pushButton_clear_clicked()
{
     ui->visualWindow->goPaint = 0;
     ui->visualWindow->Z_Buffer.clear();
     update();

}

void MainWindow::on_pushButton_load_clicked()
{
    ui->visualWindow->goPaint = 0;// stop paint window
    QString filename = QFileDialog::getOpenFileName(this,tr("Open File"), ".", tr("OBJ Files(*.obj)"));
    ui->visualWindow->Z_Buffer.readOBJ(filename);
    ui->visualWindow->Z_Buffer.zoom();
}
