#include "widget.h"
#include "ui_widget.h"
#include "QFileDialog"
#include"algorithm"
#include"QDebug"
int Widget::scale=3;
Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    cap.open(0);
}

Widget::~Widget()
{
    delete ui;
}

std::vector<func > Widget::getContours(Mat &srcimg, double minArea, int filter, bool draw, vector<int> cThr)
{
    vector<func> finalCountours;
    Mat Grayimg,Blurimg,Canimg,Dilaimg,Eroimg;
    cvtColor(srcimg,Grayimg,cv::COLOR_BGR2GRAY);
    cv::GaussianBlur(Grayimg,Blurimg,cv::Size(5,5),1);
    cv::Canny(Grayimg,Canimg,cThr[0],cThr[1]);
    Mat kernel=cv::Mat::ones(5,5,CV_64F);
    cv::dilate(Canimg,Dilaimg,kernel,cv::Point(-1,-1),3);
    cv::erode(Dilaimg,Eroimg,kernel,Point(-1,-1),2);
    std::vector<std::vector<cv::Point>> contours;
    //hierarchy是一个std::vector<cv::Vec4i>类型，
    //而不是std::vector<int>。cv::Vec4i是一个包含四个整数的向量，
    //用于表示轮廓之间的父子关系。这四个整数分别代表下一个轮廓、上一个轮廓、
    //第一个子轮廓和父轮廓在contours中的索引。如果某个值是负数，则表示没有对应的轮廓。
    std::vector<cv::Vec4i> hierarchy;
    cv::findContours(Eroimg,contours,hierarchy,cv::RETR_EXTERNAL,cv::CHAIN_APPROX_SIMPLE);
    for(auto i =contours.begin();i!=contours.end();i++)
    {
        double area = cv::contourArea(*i);
        if(area>minArea){
           double peri = cv::arcLength(*i,true);
           std::vector<cv::Point> approx;
           cv::approxPolyDP(*i,approx,0.02*peri,true);
           cv::Rect bbox=boundingRect(approx);

           qDebug()<<area;
           if(approx.size()==filter) //检测是不是四方体
           finalCountours.push_back(func(approx.size(),area,bbox,approx,static_cast<int>(i-contours.begin())));
       }

    }
    std::sort(finalCountours.begin(),finalCountours.end(),[&](func &f1,func&f2){
        return f1.area>f2.area;
    });
    if(draw)
    {
        for(auto j=finalCountours.begin();j!=finalCountours.end();j++)
        {
            cv::drawContours(srcimg,contours,j->index,cv::Scalar(0,0,255),3,LINE_8,hierarchy,0);
        }

    }
    return finalCountours;
}

void Widget::on_pushButton_clicked()
{
    //通过文件对话框选择路径
     QString filepath =QFileDialog::getOpenFileName(this);
     ui->lineEdit->setText(filepath);
    if(filepath == "") return;
     //处理图片
    Mat srcimg =imread(filepath.toStdString());
    cv::resize(srcimg,srcimg,cv::Size(ui->label->width(),ui->label->height()),0.5,0.5);
    Mat outimg=getimg(srcimg,false);
    //把opencv里面的Mat格式数据（BGR）转Qt里面的QImage(RGB)
    cvtColor(outimg,outimg, COLOR_BGR2RGB);
    QImage image(outimg.data,outimg.cols, outimg.rows,outimg.step1(),QImage::Format_RGB888);
    QPixmap mmp = QPixmap::fromImage(image);
    mmp=mmp.scaled(ui->label->size());
    ui->label->setPixmap(mmp);
}

std::vector<Point> Widget::reorder(std::vector<Point> &myPoints)
{
    // 确保输入点数为4
    if (myPoints.size() != 4) {
       std::vector<Point> mynull;
       return mynull;
    }

    // 创建一个用于存储新顺序点的向量
    std::vector<cv::Point> myPointsNew(4);

    // 计算每个点的x和y坐标之和
    std::vector<float> sums(4);
    for (size_t i = 0; i < 4; ++i) {
        sums[i] = myPoints[i].x+ myPoints[i].y;
    }

    // 找到最小和最大和值的索引
    auto minSumIt = std::min_element(sums.begin(), sums.end());
    auto maxSumIt = std::max_element(sums.begin(), sums.end());
    size_t minSumIdx = std::distance(sums.begin(), minSumIt);
    size_t maxSumIdx = std::distance(sums.begin(), maxSumIt);

    // 左上角和右下角
    myPointsNew[0] = myPoints[minSumIdx];
    //qDebug()<<"左上角"<<myPointsNew[0].x<<myPointsNew[0].y;
    myPointsNew[3] = myPoints[maxSumIdx];
    //qDebug()<<"右下角"<<myPointsNew[3].x<<myPointsNew[3].y;

    // 计算每个点的x和y坐标之差
    std::vector<float> diffs(4);
    for (size_t i = 0; i < 4; ++i) {
        diffs[i] = myPoints[i].x - myPoints[i].y;
    }

    // 找到最小和最大差值的索引
    auto minDiffIt = std::min_element(diffs.begin(), diffs.end());
    auto maxDiffIt = std::max_element(diffs.begin(), diffs.end());
    size_t minDiffIdx = std::distance(diffs.begin(), minDiffIt);
    size_t maxDiffIdx = std::distance(diffs.begin(), maxDiffIt);

    // 右上角和左下角
    myPointsNew[1] = myPoints[maxDiffIdx];
    //qDebug()<<"右上角"<<myPointsNew[1].x<<myPointsNew[1].y;
    myPointsNew[2] = myPoints[minDiffIdx];
    //qDebug()<<"左下角"<<myPointsNew[2].x<<myPointsNew[2].y;

    return myPointsNew;
}

Mat Widget::warpImg(Mat &img, std::vector<Point> &points, int w, int h, int pad)
{
    // 重新排序点
     std::vector<cv::Point2f> pts1 = convertPointsToPointF(reorder(points));

    // 定义目标点
    std::vector<cv::Point2f> pts2;
    pts2.push_back(cv::Point2f(0, 0));
    pts2.push_back(cv::Point2f(w, 0));
    pts2.push_back(cv::Point2f(0, h));
    pts2.push_back(cv::Point2f(w, h));

    // 计算透视变换矩阵
    cv::Mat matrix = cv::getPerspectiveTransform(pts1, pts2);

    // 应用透视变换
    cv::Mat imgWarp;
    cv::warpPerspective(img, imgWarp, matrix, cv::Size(w, h));

    // 裁剪图像
    cv::Rect roi(pad, pad, imgWarp.cols - 2*pad, imgWarp.rows - 2*pad);
    imgWarp = imgWarp(roi);

    return imgWarp;
}

std::vector<Point2f> Widget::convertPointsToPointF(const std::vector<Point> &points)
{
    std::vector<cv::Point2f> points2f;
        points2f.reserve(points.size()); // 预分配内存以提高效率
        for (const auto& point : points) {
            cv::Point2f point2f(static_cast<float>(point.x), static_cast<float>(point.y));
            points2f.push_back(point2f);
        }
        return points2f;
}
double Widget::dis(Point &p1, Point &p2)
{
    return std::sqrt(std::pow(p1.x/scale-p2.x/scale,2)+std::pow(p1.y/scale-p2.y/scale,2));
}

Mat Widget::getimg(Mat srcimg,bool isCapture)
{

    std::vector<cv::Point> biggestContours;
    auto conts=getContours(srcimg,3000,4,false);
   if(conts.size()!=0)
   {
      biggestContours=conts[0].approx;
   }
   else
   {   //没有检测到A4纸，输出原始图像
       //把opencv里面的Mat格式数据（BGR）转Qt里面的QImage(RGB)
       cvtColor(srcimg,srcimg, COLOR_BGR2RGB);
       QImage image(srcimg.data,srcimg.cols, srcimg.rows,srcimg.step1(),QImage::Format_RGB888);
       QPixmap mmp = QPixmap::fromImage(image);
       ui->label->setPixmap(mmp);
       qDebug()<<"1";
       return srcimg;
   }
   Mat warpimg=warpImg(srcimg,biggestContours,210*scale,297*scale);

   auto conts2=getContours(warpimg,1000,4,true);
   bool flag=false;
   for(auto i:conts2)
   {
       flag=true;
       //cv::polylines(warpimg,i.approx,true,cv::Scalar(0,255,0),2);
       auto npoints= reorder(i.approx);
       double newW=std::round(dis(npoints[0],npoints[1])/10 * 10.0) / 10.0;
       double newH=std::round(dis(npoints[0],npoints[2])/10 * 10.0) / 10.0;
       cv::arrowedLine(warpimg,npoints[0],npoints[1],Scalar(255,255,0),3,8,0,0.05);
       cv::arrowedLine(warpimg,npoints[0],npoints[2],Scalar(255,255,0),3,8,0,0.05);
       //横线
       //第三个参数点的位置表示字体的左下角，所以取中点
       cv::Point pos;
       pos.x=(npoints[0].x+npoints[1].x)/2;
       pos.y=(npoints[0].y+npoints[1].y)/2;
       cv::putText(warpimg,QString("%1cm").arg(newW).toStdString(),pos,cv::FONT_HERSHEY_COMPLEX_SMALL,1.5,
                   cv::Scalar(255, 0, 255));
       //竖线
       pos.x=(npoints[0].x+npoints[2].x)/2;
       pos.y=(npoints[0].y+npoints[2].y)/2;
       cv::putText(warpimg,QString("%1cm").arg(newH).toStdString(),pos,cv::FONT_HERSHEY_COMPLEX_SMALL,1.5,
                   cv::Scalar(255, 0, 255));
   }
   if(isCapture&&flag)
   {
       //把opencv里面的Mat格式数据（BGR）转Qt里面的QImage(RGB)
       cvtColor(warpimg,warpimg, COLOR_BGR2RGB);
       QImage image(warpimg.data,warpimg.cols, warpimg.rows,warpimg.step1(),QImage::Format_RGB888);
       QPixmap mmp = QPixmap::fromImage(image);
       ui->label->setPixmap(mmp);
       killTimer(this->timerid);

   }
   return warpimg;
}



void Widget::on_pushButton_2_clicked()
{
      if(ui->pushButton_2->text()=="打开摄像头")
      {
          ui->pushButton_2->setText("关闭摄像头");

           timerid=startTimer(100);
           if(!cap.isOpened())
               cap.open(0);
      }
      else
      {
          ui->pushButton_2->setText("打开摄像头");
          ui->label->clear();
          cap.release();
          killTimer(timerid);
      }
}

void Widget::timerEvent(QTimerEvent *e)
{
      cv::Mat srcimg;
     if(cap.grab())
     {
         cap.read(srcimg);
     }
    if(srcimg.data!=nullptr)
     getimg(srcimg,true);
}
