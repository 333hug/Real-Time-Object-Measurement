#ifndef FUNC_H
#define FUNC_H

#include<opencv.hpp>
class func
{
public:  
    int length;
    double area;
    cv::Rect bbox;
    std::vector<cv::Point> approx;
    int index;
    func();
    func(int length,double area,cv::Rect bbox,std::vector<cv::Point> approx, int index)
    {
        this->length=length;
        this->area=area;
        this->bbox=bbox;
        this->approx=approx;
        this->index=index;
        //this->i=i;
    }
};

#endif // FUNC_H
