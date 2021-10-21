#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCloseEvent>
#include <QFileDialog>
#include <QMessageBox>
#include <QGraphicsPixmapItem>
#include <QImage>
#include <QPixmap>
#include <QTime>
#include <QTimer>
#include <QtConcurrent/QtConcurrentRun>
#include <vector>
//#include <libuvc/libuvc.h>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/video.hpp>
//#include "graph.h"

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
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_horizontalSlider_valueChanged(int value);

    void on_horizontalSlider_3_valueChanged(int value);

    void on_horizontalSlider_2_valueChanged(int value);

    void on_horizontalSlider_4_valueChanged(int value);

    void on_openButton_clicked();

    void on_comboBox_currentIndexChanged(int index);

    void on_pushButton_3_clicked();

    void on_pushButton_4_clicked();

    void on_Timer1_tick();

private:
    Ui::MainWindow *ui;
    //graph *graphWindow;
    QTimer *Timer1;
    static void frameCap(void *ptr);
    uint cvCount();

    void closeEvent(QCloseEvent *event) override;
    void showEvent(QShowEvent *event) override;
    bool event(QEvent *event) override;

    int devIndex;

    QFuture<void> cb_future;
    cv::VideoCapture inputVideo;
    cv::VideoWriter outputVideo;
    cv::Mat mainimage;
    cv::Mat currframe;
    cv::Mat thrsimage;
    QGraphicsPixmapItem pixmap;
    bool first_run;
    bool opened;
    bool online;
    uint pcount;
    QTime clock;
};

#endif // MAINWINDOW_H
