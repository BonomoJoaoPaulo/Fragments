#ifndef FRAGMENTER_H
#define FRAGMENTER_H

#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>

struct FragmentInfo {
    std::vector<uchar> data_array;
    int size;
    int offset;
};

class Observer {
public:
    virtual void notify(const FragmentInfo& fragmentInfo) = 0;
    virtual ~Observer() = default;
};

class Fragmenter {
private:
    std::vector<Observer*> observers;

public:
    void addObserver(Observer* observer) {
        observers.push_back(observer);
    }

    void removeObserver(Observer* observer) {
        // Implement removal logic if needed
    }

    void fragment(const cv::Mat& originalImage, int numFragments) {
        int fragmentWidth = originalImage.cols / numFragments;
        int fragmentHeight = originalImage.rows;

        for (int i = 0; i < numFragments; i++) {
            std::cout << "Fragmenting image: " << i + 1 << "/" << numFragments << std::endl;
            int fragmentOffset = i * fragmentWidth;
            cv::Mat fragment = originalImage(cv::Rect(fragmentOffset, 0, fragmentWidth, fragmentHeight));

            if (!fragment.empty()) {
                FragmentInfo info;
                cv::imencode(".jpg", fragment, info.data_array);
                info.size = fragment.total() * fragment.elemSize();
                info.offset = fragmentOffset;

                for (Observer* observer : observers) {
                    observer->notify(info);
                }
            } else {
                std::cerr << "Error: Fragment is invalid." << std::endl;
            }
        }
    }
};

#endif // FRAGMENTER_H