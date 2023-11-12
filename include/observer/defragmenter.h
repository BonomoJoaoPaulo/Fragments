#ifndef DEFRAGMENTER_H
#define DEFRAGMENTER_H

#include "fragmenter.h"
#include <opencv2/opencv.hpp>

class Defragmenter : public Observer {
private:
    cv::Mat reconstructedImage;

public:
    Defragmenter(int rows, int cols) : reconstructedImage(rows, cols, CV_8UC3, cv::Scalar(0, 0, 0)) {}

    void notify(const FragmentInfo& fragmentInfo) override {
        cv::Mat fragment = cv::imdecode(fragmentInfo.data_array, cv::IMREAD_COLOR);
        cv::Rect roi(fragmentInfo.offset, 0, fragment.cols, fragment.rows);

        if (roi.x + roi.width <= reconstructedImage.cols && roi.y + roi.height <= reconstructedImage.rows) {
            fragment.copyTo(reconstructedImage(roi));
        } else {
            std::cerr << "Error: Fragment position exceeds image bounds." << std::endl;
        }
    }

    cv::Mat getReconstructedImage() const {
        return reconstructedImage;
    }
};

#endif // DEFRAGMENTER_H