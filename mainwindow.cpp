#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //graphWindow = new graph();
    Timer1 = new QTimer();
    Timer1->setInterval(1000);
    connect(Timer1, SIGNAL(timeout()), this, SLOT(on_Timer1_tick()));
    //Timer1->start();
    pcount = 0;
    online = false;
    opened = false;
    first_run = true;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::showEvent(QShowEvent *event)
{
    bool avail = true;
    int i = 0;
    while(avail)
    {
        avail = inputVideo.open(i + cv::CAP_ANY);
        if(avail)
        {
            ui->comboBox->addItem(QString::asprintf("Camera %d", i));
            inputVideo.release();
            i++;
        }
    }

    if(i == 0)
    {
        QMessageBox::critical(this, QString("Error!"), QString("No camera found!"));
        exit(0);
    }
    event->accept();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    online = false;
    inputVideo.release();
    event->accept();
}

bool MainWindow::event(QEvent *event)
{
    if(event->type() == QEvent::User)
    {
        // Main image processing algorithm !!!
        using namespace cv;

        if(online)
        {
            pcount += cvCount();
            ui->lineEdit->setText(QString::number(pcount));
            ui->lineEdit2->setText(QString::asprintf("%.1f", clock.elapsed() / 1000.0));
            ui->lineEdit3->setText(QString::asprintf("%.1f", 1000.0 * double(pcount) / double(clock.elapsed())));
            bitwise_or(currframe, mainimage, mainimage);

        }
        else
        {
            currframe.copyTo(mainimage);
        }

        if(ui->checkBox->isChecked())
        {
            threshold(mainimage, thrsimage, ui->horizontalSlider_4->value(), 255, 0);
            imshow("Video", thrsimage);
        }
        else
        {
            imshow("Video", mainimage);
        }

        currframe.release();
        return (true);
    }
    return QWidget::event(event);
}

void MainWindow::on_Timer1_tick()
{
    static uint prev_pcount;
    static int sec_passed;

    if(pcount - prev_pcount > 0)
    {
        ui->lineEdit3->setText(QString::asprintf("%.1f", double(pcount - prev_pcount) / double(sec_passed + 1)));
        prev_pcount = pcount;
        sec_passed = 0;
    }
    else
    {
        sec_passed++;
    }
}

uint MainWindow::cvCount()
{
    using namespace  std;
    using namespace cv;
    vector<vector<Point>> contours;
    vector<Vec4i> hierarchy;

    Mat temp;
    threshold(currframe, temp, ui->horizontalSlider_4->value(), 255, 0);
    findContours(temp, contours, hierarchy, RETR_TREE, CHAIN_APPROX_NONE);
    temp.release();

    return (static_cast<uint>(contours.size()));
}

void MainWindow::frameCap(void *ptr)
{
    using namespace cv;
    MainWindow *mwnd = static_cast<MainWindow*>(ptr);

    while(mwnd->opened)
    {
        mwnd->inputVideo.read(mwnd->currframe);
        if(mwnd->currframe.empty())
        {
            QMessageBox::critical(mwnd, QString("Error!"), QString("Connection with camera lost!"));
            break;
        }
        cvtColor(mwnd->currframe, mwnd->currframe, COLOR_BGR2GRAY);
        QCoreApplication::postEvent(mwnd, new QEvent(QEvent::User));
    }
}

void MainWindow::on_pushButton_clicked()
{
    if(online == false)
    {
        online = true;
        ui->pushButton->setText("Stop");
        clock.start();
        Timer1->start();
    }
    else
    {
        online = false;
        ui->pushButton->setText("Start");
        Timer1->stop();
        pcount = 0;
    }
}

void MainWindow::on_pushButton_2_clicked()
{
    mainimage = cv::Mat::zeros(mainimage.size(), mainimage.type());
    pcount = 0;
    //clock.restart();
}

void MainWindow::on_horizontalSlider_valueChanged(int value)
{
    ui->label1->setText(QString::asprintf("%.3d", value));
    inputVideo.set(cv::CAP_PROP_BRIGHTNESS, double(value));
}

void MainWindow::on_horizontalSlider_2_valueChanged(int value)
{
    ui->label2->setText(QString::asprintf("%.3d", value));
    inputVideo.set(cv::CAP_PROP_CONTRAST, double(value));
}

void MainWindow::on_horizontalSlider_3_valueChanged(int value)
{
    ui->label3->setText(QString::asprintf("%.3d", value));
    inputVideo.set(cv::CAP_PROP_GAIN, double(value));
}

void MainWindow::on_horizontalSlider_4_valueChanged(int value)
{
    ui->label4->setText(QString::asprintf("%.3d", value));
}

void MainWindow::on_openButton_clicked()
{
    QMessageBox msg;

    if(!opened)
    {
        ui->openButton->setText("Opening...");
        ui->openButton->setEnabled(false);
        ui->comboBox->setEnabled(false);
        qApp->processEvents();

        inputVideo.open(devIndex, cv::CAP_V4L2);
        inputVideo.set(cv::CAP_PROP_FRAME_WIDTH, 1280);
        inputVideo.set(cv::CAP_PROP_FRAME_HEIGHT, 960);
        inputVideo.set(cv::CAP_PROP_FPS, 15);

        double gain = inputVideo.get(cv::CAP_PROP_GAIN);
        double brightness = inputVideo.get(cv::CAP_PROP_BRIGHTNESS);
        double contrast = inputVideo.get(cv::CAP_PROP_CONTRAST);

        ui->horizontalSlider->setValue(int(brightness));
        ui->horizontalSlider_2->setValue(int(contrast));
        ui->horizontalSlider_3->setValue(int(gain));

        opened = true;
        cb_future = QtConcurrent::run(frameCap, this);

        ui->openButton->setText("Close");
        ui->openButton->setEnabled(true);
        ui->horizontalSlider->setEnabled(true);
        ui->horizontalSlider_2->setEnabled(true);
        ui->horizontalSlider_3->setEnabled(true);
        ui->horizontalSlider_4->setEnabled(true);
        ui->pushButton->setEnabled(true);
    }
    else
    {
        opened = false;
        cb_future.waitForFinished();

        inputVideo.release();
        ui->openButton->setText("Open");
        ui->comboBox->setEnabled(true);
        ui->horizontalSlider->setEnabled(false);
        ui->horizontalSlider_2->setEnabled(false);
        ui->horizontalSlider_3->setEnabled(false);
        ui->horizontalSlider_4->setEnabled(false);
        ui->pushButton->setEnabled(false);
    }
}

void MainWindow::on_comboBox_currentIndexChanged(int index)
{
    devIndex = index;
}

void MainWindow::on_pushButton_3_clicked()
{

}

void MainWindow::on_pushButton_4_clicked()
{
    cv::imwrite("shot.jpg", mainimage);
}
