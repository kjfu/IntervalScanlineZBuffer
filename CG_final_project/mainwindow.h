#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "interval_scanline_zbuffer.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();


private slots:
    void on_actionOpen_triggered();

    void on_pushButton_dispaly_clicked();

    void on_pushButton_zoom_clicked();

    void on_pushButton_clear_clicked();

    void on_pushButton_load_clicked();

private:
    Ui::MainWindow *ui;


};

#endif // MAINWINDOW_H
