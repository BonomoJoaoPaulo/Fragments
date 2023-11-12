#ifndef DEFRAGMENTER_H
#define DEFRAGMENTER_H

#include <opencv2/opencv.hpp>
#include <mutex>
#include <condition_variable>

#include "fragmenter.h"

class Defragmenter : public Observer {
private:
    cv::Mat reconstructedImage;
    std::mutex& mutex;
    std::condition_variable& condition_var;
    bool& ready;

public:
    Defragmenter(int rows, int cols, bool& readyPtr, std::mutex& mutexPtr, std::condition_variable& cvPtr) :
    reconstructedImage(rows, cols, CV_8UC3, cv::Scalar(0, 0, 0)), ready(readyPtr), mutex(mutexPtr), condition_var(cvPtr) {}

    void notify(const FragmentInfo& fragmentInfo) override {
        std::cout << "D: notified." << std::endl;
        {
            std::unique_lock<std::mutex> lock(mutex);

            // Wait until fragment is ready
            if (condition_var.wait_for(lock, std::chrono::milliseconds(500), [&]() { return ready; })) {
                std::cout << "D: Received fragment." << std::endl;
                std::cout << "D: Fragment offset: " << fragmentInfo.offset << std::endl;
                std::cout << "D: Fragment size: " << fragmentInfo.size << std::endl;
                // Reconstruct the image using the received fragment info
                cv::Mat fragment = cv::imdecode(fragmentInfo.data_array, cv::IMREAD_COLOR);
                cv::Rect roi(fragmentInfo.offset, 0, fragment.cols, fragment.rows);

                if (roi.x + roi.width <= reconstructedImage.cols && roi.y + roi.height <= reconstructedImage.rows) {
                    fragment.copyTo(reconstructedImage(roi));
                } else {
                    std::cerr << "Error: Fragment position exceeds image bounds." << std::endl;
                }
                ready = false;
            } else {
                std::cerr << "D. Error: Timeout while waiting for fragment." << std::endl;
            }
        }
    }

    cv::Mat getReconstructedImage() const {
        return reconstructedImage;
    }
};

#endif // DEFRAGMENTER_H