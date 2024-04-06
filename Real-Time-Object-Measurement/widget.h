#ifndef WIDGET_H
#define WIDGET_H
#include"Eigen"
#include <QWidget>
#include<opencv.hpp>
#include"func.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
using namespace std;
using namespace cv;
QT_END_NAMESPACE
class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();
    std::vector<func> getContours(Mat& inputimg,double minArea=1000,int filter=0, bool draw =false,vector<int> cThr={180,180});
    static int scale;
    std::vector<cv::Point> reorder( std::vector<cv::Point>& myPoints);
    Mat warpImg(Mat &img, std::vector<Point> &points, int w, int h, int pad=20);
    std::vector<cv::Point2f> convertPointsToPointF(const std::vector<cv::Point>& points);
    double dis(cv::Point &p1,cv::Point &p2);
    Mat getimg(Mat srcimg,bool isCapture);
private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();
    void timerEvent(QTimerEvent *e);

private:
    Ui::Widget *ui;
    VideoCapture cap;
    int timerid;

};
#endif // WIDGET_H
