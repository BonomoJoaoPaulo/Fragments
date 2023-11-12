#ifndef FRAGMENTER_H
#define FRAGMENTER_H

#include <iostream>
#include <vector>
#include <mutex>
#include <condition_variable>
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
    std::mutex& mutex;
    std::condition_variable& condition_var;
    bool& ready;

public:
    Fragmenter(bool& readyPtr, std::mutex& mutexPtr, std::condition_variable& cvPtr) :
    ready(readyPtr), mutex(mutexPtr), condition_var(cvPtr) {}

    void addObserver(Observer* observer) {
        observers.push_back(observer);
    }

    void removeObserver(Observer* observer) {
        std::cout << "F: RemoveObserver is not implemented." << std::endl;  
    }

    void fragment(const cv::Mat& originalImage, int numFragments) {
        int fragmentWidth = originalImage.cols / numFragments;
        int fragmentHeight = originalImage.rows;

        for (int i = 0; i < numFragments; i++) {
            std::cout << "F: Fragmenting image: " << i + 1 << "/" << numFragments << std::endl;
            int fragmentOffset = i * fragmentWidth;
            cv::Mat fragment = originalImage(cv::Rect(fragmentOffset, 0, fragmentWidth, fragmentHeight));

            if (!fragment.empty()) {
                std::cout << "F: Fragment offset: " << fragmentOffset << std::endl;
                FragmentInfo info;
                cv::imencode(".jpg", fragment, info.data_array);
                info.size = fragment.total() * fragment.elemSize();
                info.offset = fragmentOffset;

                // Notify all observers about the fragment info
                for (Observer* observer : observers) {
                    std::cout << "F: Notifying observer." << std::endl;
                    observer->notify(info);
                }
                {
                std::unique_lock<std::mutex> lock(mutex);
                    // Set ready to true and notify waiting thread
                    std::cout << "F: Setting ready to true." << std::endl;
                    ready = true;
                    condition_var.notify_all();
                }
                std::cout << "F: Notified all observers." << std::endl;

            } else {
                std::cerr << "F. Error: Fragment is invalid." << std::endl;
            }
        }
    }
};

#endif // FRAGMENTER_H