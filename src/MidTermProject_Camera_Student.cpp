/* INCLUDES FOR THIS PROJECT */
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <cmath>
#include <limits>
#include <opencv2/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d.hpp>
#include <deque>
#include <opencv2/xfeatures2d.hpp>
#include <opencv2/xfeatures2d/nonfree.hpp>

#include "dataStructures.h"
#include "matching2D.hpp"

using namespace std;


/* MAIN PROGRAM */
int main(int argc, const char* argv[])
{

    /* INIT VARIABLES AND DATA STRUCTURES */

    // data location
    string dataPath = "../";
    //dataPath = "C:/Users/arhyr/Desktop/sensor fusion/SFND_2D_Feature_Tracking";

    // camera
    string imgBasePath = dataPath + "images/";
    string imgPrefix = "KITTI/2011_09_26/image_00/data/000000"; // left camera, color
    string imgFileType = ".png";
    int imgStartIndex = 0; // first file index to load (assumes Lidar and camera names have identical naming convention)
    int imgEndIndex = 9;   // last file index to load
    int imgFillWidth = 4;  // no. of digits which make up the file index (e.g. img-0001.png)

    // misc
    int dataBufferSize = 2;       // no. of images which are held in memory (ring buffer) at the same time // list of data frames which are held in memory at the same time
    bool bVis = false;            // visualize results

    /* MAIN LOOP OVER ALL IMAGES */
    //string detectorType = "SHITOMASI"; //SHITOMASI, HARRIS, FAST, BRISK, ORB, AKAZE, SIFT
    //string descriptorType = "BRISK"; //BRISK, BRIEF, ORB, FREAK, AKAZE, SIFT
    string matcherType = "MAT_BF";        // MAT_BF, MAT_FLANN
    //string descriptorType = "DES_BINARY"; // DES_BINARY, DES_HOG
    string selectorType = "SEL_KNN";       // SEL_NN, SEL_KNN

    vector<string> detectorTypes{ "SHITOMASI", "HARRIS", "FAST", "BRISK", "ORB", "AKAZE", "SIFT" };
    vector<string> descriptorTypes{ "BRISK", "BRIEF", "ORB", "FREAK", "AKAZE", "SIFT" };
    vector<double> times;
    int counter = 1;
    bool output = false;
    for (auto i = detectorTypes.begin(); i != detectorTypes.end(); i++) {
        for (auto j = descriptorTypes.begin(); j != descriptorTypes.end(); j++) {
            string detectorType = *i;
            string descriptorType = *j;
            //Akaze descriptor can work only with akaze point
            if (detectorType.compare("AKAZE") != 0 && descriptorType.compare("AKAZE") == 0)
                continue;
            if (detectorType.compare("SIFT") == 0 && descriptorType.compare("ORB") == 0)
                continue;
            deque<DataFrame> dataBuffer;
            int num_keyPoints = 0;
            int num_matches = 0;
            double time = 0;

            for (size_t imgIndex = 0; imgIndex <= imgEndIndex - imgStartIndex; imgIndex++)
            {
                /* LOAD IMAGE INTO BUFFER */
                double t = (double)cv::getTickCount();
                // assemble filenames for current index
                ostringstream imgNumber;
                imgNumber << setfill('0') << setw(imgFillWidth) << imgStartIndex + imgIndex;
                string imgFullFilename = imgBasePath + imgPrefix + imgNumber.str() + imgFileType;

                // load image from file and convert to grayscale
                cv::Mat img, imgGray;
                img = cv::imread(imgFullFilename);
                cv::cvtColor(img, imgGray, cv::COLOR_BGR2GRAY);

                //// STUDENT ASSIGNMENT
                //// TASK MP.1 -> replace the following code with ring buffer of size dataBufferSize

                // push image into data frame buffer
                DataFrame frame;
                frame.cameraImg = imgGray;
                dataBuffer.push_back(frame);
                if (dataBuffer.size() > dataBufferSize) {
                    dataBuffer.pop_front();
                }

                //// EOF STUDENT ASSIGNMENT
                // std::cout << "#1 : LOAD IMAGE INTO BUFFER done" << endl;

                /* DETECT IMAGE KEYPOINTS */

                // extract 2D keypoints from current image
                vector<cv::KeyPoint> keypoints; // create empty feature list for current image

                //// STUDENT ASSIGNMENT
                //// TASK MP.2 -> add the following keypoint detectors in file matching2D.cpp and enable string-based selection based on detectorType
                //// -> HARRIS, FAST, BRISK, ORB, AKAZE, SIFT

                if (detectorType.compare("SHITOMASI") == 0)
                {
                    detKeypointsShiTomasi(keypoints, imgGray, false);
                }
                else if (detectorType.compare("HARRIS") == 0)
                {
                    detKeypointsHarris(keypoints, imgGray, false);
                }
                else if (detectorType.compare("FAST") == 0)
                {
                    detKeypointsModern(keypoints, imgGray, detectorType, false);
                }
                else if (detectorType.compare("BRISK") == 0)
                {
                    detKeypointsModern(keypoints, imgGray, detectorType, false);
                }
                else if (detectorType.compare("ORB") == 0)
                {
                    detKeypointsModern(keypoints, imgGray, detectorType, false);
                }
                else if (detectorType.compare("AKAZE") == 0)
                {
                    detKeypointsModern(keypoints, imgGray, detectorType, false);
                }
                else if (detectorType.compare("SIFT") == 0)
                {
                    detKeypointsModern(keypoints, imgGray, detectorType, false);
                }
                //// EOF STUDENT ASSIGNMENT

                //// STUDENT ASSIGNMENT
                //// TASK MP.3 -> only keep keypoints on the preceding vehicle

                // only keep keypoints on the preceding vehicle
                bool bFocusOnVehicle = true;
                cv::Rect vehicleRect(535, 180, 180, 150);

                vector<cv::KeyPoint> temp;
                if (bFocusOnVehicle)
                {
                    for (auto it = keypoints.begin(); it != keypoints.end(); it++) {
                        if (vehicleRect.contains(it->pt)) {
                            temp.emplace_back(*it);
                        }
                    }
                }
                keypoints = temp;


                //// EOF STUDENT ASSIGNMENT

                // optional : limit number of keypoints (helpful for debugging and learning)
                bool bLimitKpts = false;
                if (bLimitKpts)
                {
                    int maxKeypoints = 50;

                    if (detectorType.compare("SHITOMASI") == 0)
                    { // there is no response info, so keep the first 50 as they are sorted in descending quality order
                        keypoints.erase(keypoints.begin() + maxKeypoints, keypoints.end());
                    }
                    cv::KeyPointsFilter::retainBest(keypoints, maxKeypoints);
                    std::cout << " NOTE: Keypoints have been limited!" << endl;
                }

                // push keypoints and descriptor for current frame to end of data buffer
                (dataBuffer.end() - 1)->keypoints = keypoints;
                // std::cout << "#2 : DETECT KEYPOINTS done" << endl;

                /* EXTRACT KEYPOINT DESCRIPTORS */

                //// STUDENT ASSIGNMENT
                //// TASK MP.4 -> add the following descriptors in file matching2D.cpp and enable string-based selection based on descriptorType
                //// -> BRIEF, ORB, FREAK, AKAZE, SIFT

                cv::Mat descriptors;

                descKeypoints((dataBuffer.end() - 1)->keypoints, (dataBuffer.end() - 1)->cameraImg, descriptors, descriptorType);
                //// EOF STUDENT ASSIGNMENT

                // push descriptors for current frame to end of data buffer
                (dataBuffer.end() - 1)->descriptors = descriptors;

                // std::cout << "#3 : EXTRACT DESCRIPTORS done" << endl;

                if (dataBuffer.size() > 1) // wait until at least two images have been processed
                {

                    /* MATCH KEYPOINT DESCRIPTORS */

                    vector<cv::DMatch> matches;

                    //// STUDENT ASSIGNMENT
                    //// TASK MP.5 -> add FLANN matching in file matching2D.cpp
                    //// TASK MP.6 -> add KNN match selection and perform descriptor distance ratio filtering with t=0.8 in file matching2D.cpp

                    matchDescriptors((dataBuffer.end() - 2)->keypoints, (dataBuffer.end() - 1)->keypoints,
                        (dataBuffer.end() - 2)->descriptors, (dataBuffer.end() - 1)->descriptors,
                        matches, descriptorType, matcherType, selectorType);

                    //// EOF STUDENT ASSIGNMENT

                    // store matches in current data frame
                    (dataBuffer.end() - 1)->kptMatches = matches;

                    // std::cout << "#4 : MATCH KEYPOINT DESCRIPTORS done" << endl;

                    num_keyPoints += keypoints.size();
                    num_matches += matches.size();
                    time += ((double)cv::getTickCount() - t) / cv::getTickFrequency();

                    // visualize matches between current and previous image
                    bVis = false;
                    if (bVis)
                    {
                        cv::Mat matchImg = ((dataBuffer.end() - 1)->cameraImg).clone();
                        cv::drawMatches((dataBuffer.end() - 2)->cameraImg, (dataBuffer.end() - 2)->keypoints,
                            (dataBuffer.end() - 1)->cameraImg, (dataBuffer.end() - 1)->keypoints,
                            matches, matchImg,
                            cv::Scalar::all(-1), cv::Scalar::all(-1),
                            vector<char>(), cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);

                        string windowName = "Matching keypoints between two camera images";
                        cv::namedWindow(windowName, 7);
                        cv::imshow(windowName, matchImg);
                        std::cout << "Press key to continue to next image" << endl;
                        cv::waitKey(0); // wait for key to be pressed
                    }
                    bVis = false;
                }
            } // eof loop over all images
            if (!output) {
                std::cout << to_string(counter) << ". " << "DetectorType " << detectorType << " got keyPoints " << num_keyPoints << std::endl;
                output = true;
                counter++;
            }
            std::cout << "* " << "DescriptorType " << descriptorType << " matches " << num_matches << " features in total " << time << " seconds" << std::endl;
            times.emplace_back(time);
        }
        output = false;
    }
    sort(times.begin(), times.end());
    for (auto time : times) {
        std::cout << to_string(time) << endl;
    }
    return 0;
}
