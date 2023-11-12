#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <set>
#include <condition_variable>
#include <opencv2/opencv.hpp>

struct FragmentInfo {
    std::vector<uchar> data_array; // Serialized fragment data
    int size;           // Size of the fragment data
    int offset;         // Offset of the fragment in the original image
};

const int numFragments = 48;
std::vector<FragmentInfo> buffer;
std::mutex mtx;
std::condition_variable condVar;

cv::Mat reconstructedImage;
std::set<int> consumedFragments;


void producer(cv::Mat matImage, int fragmentHeight, int fragmentWidth) {
    std::cout << "Producer started." << std::endl;

    int newWidth = fragmentWidth * numFragments;

    if (matImage.cols != newWidth) {
        cv::resize(matImage, matImage, cv::Size(newWidth, fragmentHeight));
    }

    for (int i = 0; i < numFragments; i++) {
        int fragmentOffset = i * fragmentWidth;
        cv::Mat fragment = matImage(cv::Rect(fragmentOffset, 0, fragmentWidth, fragmentHeight));

        if (!fragment.empty()) {
            FragmentInfo info;
            cv::imencode(".jpg", fragment, info.data_array);
            info.size = info.data_array.size();
            info.offset = fragmentOffset;

            std::cout << "Produced fragment " << i << " of " << numFragments << std::endl;
            std::cout << "PRODUCER INFO: " << info.size << " " << info.offset << std::endl;

            std::unique_lock<std::mutex> lock(mtx);
            condVar.wait(lock, [] { return buffer.size() < numFragments; });

            buffer.push_back(info);

            lock.unlock();
            condVar.notify_all();
        } else {
            std::cerr << "Error: Fragment is invalid." << std::endl;
        }
    }

    std::cout << "Producer finished." << std::endl;
}

void consumer(int fragmentHeight, int fragmentWidth) {
    std::cout << "Consumer started." << std::endl;

    int reconstructedFragments = 0;

    std::vector<int> randomOrder(numFragments);
    for (int i = 0; i < numFragments; i++) {
        randomOrder[i] = i;
    }
    std::random_shuffle(randomOrder.begin(), randomOrder.end());

    while (reconstructedFragments < numFragments) {
        for (int i = 0; i < numFragments; i++) {
            int randomIndex = randomOrder[i];

            std::unique_lock<std::mutex> lock(mtx);
            condVar.wait(lock, [] { return !buffer.empty(); });

            if (randomIndex >= buffer.size()) {
                std::cerr << "Error: Random index exceeds buffer size." << std::endl;
                continue;
            }
            else {
                FragmentInfo info = buffer[randomIndex];
                if (consumedFragments.find(randomIndex) == consumedFragments.end()) {
                std::cout << "Consumed fragment " << randomIndex << " of " << numFragments << std::endl;
                std::cout << "CONSUMER INFO size and offset: " << info.size << " " << info.offset << std::endl;

                cv::Mat fragment = cv::imdecode(info.data_array, cv::IMREAD_COLOR);
                cv::Rect roi(info.offset, 0, fragment.cols, fragment.rows);

                if (roi.x + roi.width <= reconstructedImage.cols && roi.y + roi.height <= reconstructedImage.rows) {
                    fragment.copyTo(reconstructedImage(roi));
                } else {
                    std::cerr << "Error: Fragment position exceeds image bounds." << std::endl;
                }
                consumedFragments.insert(randomIndex);
                reconstructedFragments++;
                } else {
                    std::cout << "Fragment " << randomIndex << " already consumed." << std::endl;
                }
            }
            
            lock.unlock();
            condVar.notify_all();
        }
    }

    std::cout << "Consumer finished." << std::endl;
}

int main() {
    cv::Mat originalImage = cv::imread("../media/inputs/fiat_strada.jpeg");

    if (!originalImage.data)
        std::cout << "\nError opening image file!\n"
                  << std::endl;
    else {
        std::cout << "\nImage file opened successfully!\n"
                  << std::endl;
        cv::imshow("Original Image", originalImage);
        cv::waitKey(0);
    }

    reconstructedImage = cv::Mat(originalImage.rows, originalImage.cols, originalImage.type());

    int fragmentWidth = originalImage.cols / numFragments;
    int fragmentHeight = originalImage.rows;

    std::thread producerThread(producer, originalImage, fragmentHeight, fragmentWidth);
    std::thread consumerThread(consumer, fragmentHeight, fragmentWidth);

    producerThread.join();
    consumerThread.join();

    cv::imshow("Reconstructed Image", reconstructedImage);
    cv::waitKey(0);

    return 0;
}
