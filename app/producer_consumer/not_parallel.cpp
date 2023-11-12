#include <iostream>
#include <thread>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <opencv4/opencv2/highgui/highgui.hpp>
#include <opencv4/opencv2/core.hpp>
#include <opencv4/opencv2/imgcodecs.hpp>
#include <opencv4/opencv2/imgproc.hpp>
#include <opencv4/opencv2/dnn.hpp>

struct FragmentInfo {
    std::vector<uchar> data_array; // Serialized fragment data
    int size;               // Size of the fragment data
    int offset;             // Offset of the fragment in the original image
};

std::vector<FragmentInfo> buffer;
std::mutex mtx;
std::condition_variable condVar;
const int numFragments = 48;

cv::Mat reconstructedImage;

void producer(cv::Mat matImage, int fragmentHeight, int fragmentWidth) {
    // Calculate the new width to ensure all fragments have the same size
    int newWidth = fragmentWidth * numFragments;

    if (matImage.cols != newWidth) {
        // Resize the image to the new dimensions
        std::cout << "Resizing image from " << fragmentWidth <<  " to " << newWidth << "x" << fragmentHeight << std::endl;
        cv::resize(matImage, matImage, cv::Size(newWidth, fragmentHeight));
    }

    for (int i = 0; i < numFragments; i++) {
        int fragmentOffset = i * fragmentWidth;
        cv::Mat fragment = matImage(cv::Rect(fragmentOffset, 0, fragmentWidth, fragmentHeight));
        //cv::imshow("Fragment", fragment);

        if (!fragment.empty()) {
            FragmentInfo info;
            cv::imencode(".jpg", fragment, info.data_array);
            //info.data_array = std::vector<uchar>(fragment);
            //info.data_array = fragment;
            info.size = fragment.total() * fragment.elemSize();
            info.offset = fragmentOffset;

            std::cout << "Produced fragment " << i << " of " << numFragments << std::endl;
            std::cout << "PRODUCER INFO: " << info.size << " " << info.offset << std::endl;

            //std::unique_lock<std::mutex> lock(mtx);
            //condVar.wait(lock, [] { return buffer.size() < numFragments; });

            buffer.push_back(info);

            //lock.unlock();
            //condVar.notify_all();
        } else {
            std::cerr << "Error: Fragment is invalid." << std::endl;
        }
    }

    std::cout << buffer.size() << std::endl;

    std::cout << "Producer finished." << std::endl;
}

void consumer(int fragmentHeight, int fragmentWidth) {
    std::cout << "Consumer started." << std::endl;

    std::vector<int> randomOrder(numFragments);
    for (int i = 0; i < numFragments; i++) {
        randomOrder[i] = i;
    }
    std::random_shuffle(randomOrder.begin(), randomOrder.end());

    for (int i = 0; i < numFragments; i++) {
        int randomIndex = randomOrder[i];
        //std::unique_lock<std::mutex> lock(mtx);
        //condVar.wait(lock, [] { return !buffer.empty(); });

        FragmentInfo info = buffer[randomIndex];
        //buffer.pop();
        std::cout << "Consumed fragment " << randomIndex << " of " << numFragments << std::endl;
        std::cout << "CONSUMER INFO: " << info.size << " " << info.offset << std::endl;

        //lock.unlock();
        //condVar.notify_all();

        // Reconstruct the image using the received fragment
        
        cv::Mat fragment = cv::imdecode(info.data_array, cv::IMREAD_COLOR);
        cv::Rect roi(info.offset, 0, fragment.cols, fragment.rows);

        //cv::imshow("Fragment Consumed", fragment);
        //cv::waitKey(0);

        // Ensure the fragment is within the bounds of the reconstructed image
        if (roi.x + roi.width <= reconstructedImage.cols && roi.y + roi.height <= reconstructedImage.rows) {
            fragment.copyTo(reconstructedImage(roi));
            //cv::imshow("Reconstructed Image Here", reconstructedImage);
            //cv::waitKey(0);
        } else {
            std::cerr << "Error: Fragment position exceeds image bounds." << std::endl;
        }
    }
}

int main() {
    // Load an image as a base for the producer
    cv::Mat originalImage = cv::imread("../media/inputs/fiat_strada.jpeg");

    if(!originalImage.data)  // check if we succeeded
        std::cout << "\nError opening image file!\n" << std::endl;
    else {
        std::cout << "\nImage file opened successfully!\n" << std::endl;
        cv::imshow("Original Image", originalImage);
        cv::waitKey(0);
    }

    // Create an empty image for reconstruction
    reconstructedImage = cv::Mat(originalImage.rows, originalImage.cols, originalImage.type());

    int fragmentWidth = originalImage.cols / numFragments;
    int fragmentHeight = originalImage.rows;

    //std::thread producerThread(producer, originalImage, fragmentHeight, fragmentWidth);
    //std::thread consumerThread(consumer, fragmentHeight, fragmentWidth);

    //producerThread.join();
    //consumerThread.join();

    producer(originalImage, fragmentHeight, fragmentWidth);
    consumer(fragmentHeight, fragmentWidth);

    // Display the reconstructed image
    cv::imshow("Reconstructed Image", reconstructedImage);
    cv::waitKey(0);

    std::cout << "Program finished." << std::endl;

    return 0;
}