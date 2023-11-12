#include <iostream>
#include <opencv2/opencv.hpp>
#include <mutex>

#include "observer/fragmenter.h"
#include "observer/defragmenter.h"

int main() {
    // Load an image as a base for the fragmenter
    cv::Mat originalImage = cv::imread("../media/inputs/fiat_strada.jpeg");

    if (originalImage.empty()) {
        std::cerr << "Error: Image not found." << std::endl;
        return 1;
    }

    // Create an empty image for reconstruction
    cv::Mat reconstructedImage = cv::Mat(originalImage.rows, originalImage.cols, originalImage.type());

    // Create a boolean variable to indicate when a fragment is ready
    bool ready = false;

    //Create a mutex to protect the ready variable
    std::mutex mutex;

    // Shared condition variable
    std::condition_variable condition_var;

    // Create instances of Fragmenter and Defragmenter
    Fragmenter fragmenter(ready, mutex, condition_var);
    Defragmenter defragmenter(originalImage.rows, originalImage.cols, ready, mutex, condition_var);

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