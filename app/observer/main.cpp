#include <iostream>
#include <opencv2/opencv.hpp>

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