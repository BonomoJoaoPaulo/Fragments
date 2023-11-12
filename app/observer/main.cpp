#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>

// Struct to hold fragment information
struct FragmentInfo {
    std::vector<uchar> data_array; // Serialized fragment data
    int size;                      // Size of the fragment data
    int offset;                    // Offset of the fragment in the original image
};

// Observer class
class Observer {
public:
    virtual void notify(const struct FragmentInfo& fragmentInfo) = 0;
    virtual ~Observer() = default;
};

// Fragmenter class
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

                // Notify all observers about the fragment information
                for (Observer* observer : observers) {
                    observer->notify(info);
                }
            } else {
                std::cerr << "Error: Fragment is invalid." << std::endl;
            }
        }
    }
};

// Defragmenter class (Observer)
class Defragmenter : public Observer {
private:
    cv::Mat reconstructedImage;

public:
    Defragmenter(int rows, int cols) : reconstructedImage(rows, cols, CV_8UC3, cv::Scalar(0, 0, 0)) {}

    void notify(const FragmentInfo& fragmentInfo) override {
        // Reconstruct the image using the received fragment information
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

int main() {
    // Load an image as a base for the fragmenter
    cv::Mat originalImage = cv::imread("../media/inputs/fiat_strada.jpeg");

    if (originalImage.empty()) {
        std::cerr << "Error: Image not found." << std::endl;
        return 1;
    }

    // Create an empty image for reconstruction
    cv::Mat reconstructedImage = cv::Mat(originalImage.rows, originalImage.cols, originalImage.type());

    // Create instances of Fragmenter and Defragmenter
    Fragmenter fragmenter;
    Defragmenter defragmenter(originalImage.rows, originalImage.cols);

    // Add the defragmenter as an observer to the fragmenter
    fragmenter.addObserver(&defragmenter);

    // Fragment the image
    int numFragments = 48;
    fragmenter.fragment(originalImage, numFragments);

    // Get the reconstructed image from the defragmenter
    cv::Mat finalImage = defragmenter.getReconstructedImage();

    // Display the original image and the reconstructed image
    cv::imshow("Original Image", originalImage);
    cv::imshow("Reconstructed Image", finalImage);
    cv::waitKey(0);

    return 0;
}
