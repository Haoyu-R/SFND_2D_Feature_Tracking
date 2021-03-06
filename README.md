# SFND 2D Feature Tracking

<img src="images/keypoints.png" width="820" height="248" />

The idea of the camera course is to build a collision detection system - that's the overall goal for the Final Project. As a preparation for this, you will now build the feature tracking part and test various detector / descriptor combinations to see which ones perform best. This mid-term project consists of four parts:

* First, you will focus on loading images, setting up data structures and putting everything into a ring buffer to optimize memory load. 
* Then, you will integrate several keypoint detectors such as HARRIS, FAST, BRISK and SIFT and compare them with regard to number of keypoints and speed. 
* In the next part, you will then focus on descriptor extraction and matching using brute force and also the FLANN approach we discussed in the previous lesson. 
* In the last part, once the code framework is complete, you will test the various algorithms in different combinations and compare them with regard to some performance measures. 

See the classroom instruction and code comments for more details on each of these parts. Once you are finished with this project, the keypoint matching part will be set up and you can proceed to the next lesson, where the focus is on integrating Lidar points and on object detection using deep-learning. 

## Dependencies for Running Locally
* cmake >= 2.8
  * All OSes: [click here for installation instructions](https://cmake.org/install/)
* make >= 4.1 (Linux, Mac), 3.81 (Windows)
  * Linux: make is installed by default on most Linux distros
  * Mac: [install Xcode command line tools to get make](https://developer.apple.com/xcode/features/)
  * Windows: [Click here for installation instructions](http://gnuwin32.sourceforge.net/packages/make.htm)
* OpenCV >= 4.1
  * This must be compiled from source using the `-D OPENCV_ENABLE_NONFREE=ON` cmake flag for testing the SIFT and SURF detectors.
  * The OpenCV 4.1.0 source code can be found [here](https://github.com/opencv/opencv/tree/4.1.0)
* gcc/g++ >= 5.4
  * Linux: gcc / g++ is installed by default on most Linux distros
  * Mac: same deal as make - [install Xcode command line tools](https://developer.apple.com/xcode/features/)
  * Windows: recommend using [MinGW](http://www.mingw.org/)

## Basic Build Instructions

1. Clone this repo.
2. Make a build directory in the top level directory: `mkdir build && cd build`
3. Compile: `cmake .. && make`
4. Run it: `./2D_feature_tracking`.

## Rubric

1. MP.1

Implement a ring buffer using deque:
```
deque<DataFrame> dataBuffer;
```
```
dataBuffer.push_back(frame);
                if (dataBuffer.size() > dataBufferSize) {
                    dataBuffer.pop_front();
                }
```

2. MP.2 + MP. 4

This is done in MidTermProject_Camera_Student.cpp line 52 ~ 65.
```
vector<string> detectorTypes{ "SHITOMASI", "HARRIS", "FAST", "BRISK", "ORB", "AKAZE", "SIFT" };
    vector<string> descriptorTypes{ "BRISK", "BRIEF", "ORB", "FREAK", "AKAZE", "SIFT" };
    for (auto i = detectorTypes.begin(); i != detectorTypes.end(); i++) {
        for (auto j = descriptorTypes.begin(); j != descriptorTypes.end(); j++) {
            string detectorType = *i;
            string descriptorType = *j;
            //Akaze descriptor can work only with akaze point
            if(detectorType.compare("AKAZE") != 0 && descriptorType.compare("AKAZE") == 0)
                continue;
            if(detectorType.compare("SIFT") == 0 && descriptorType.compare("ORB") == 0)
```
  
3. MP.3

This is done using cv::rect::contain instruction in MidTermProject_Camera_Student.cpp in line 145 ~ 154. 
```
vector<cv::KeyPoint> temp;
                if (bFocusOnVehicle)
                {
                    for (auto it = keypoints.begin(); it != keypoints.end(); it++) {
                        if (vehicleRect.contains(it->pt)) {
                            temp.emplace_back(*it);
                        }
                    }
                }
                keypoints  = temp;
```

5. MP.5 + MP.6

This is done in file matching2D_Student.cpp in function matchDescriptors.

```
void matchDescriptors(std::vector<cv::KeyPoint> &kPtsSource, std::vector<cv::KeyPoint> &kPtsRef, cv::Mat &descSource, cv::Mat &descRef,
                      std::vector<cv::DMatch> &matches, std::string descriptorType, std::string matcherType, std::string selectorType)
{
    // configure matcher
    bool crossCheck = false;
    cv::Ptr<cv::DescriptorMatcher> matcher;

    if (matcherType.compare("MAT_BF") == 0)
    {    
        int normType;
        if (descriptorType.compare("SIFT") == 0)
        {
            normType = cv::NORM_L2;
        }
        else{
            // std::cout << "test" << std::endl;
            normType = cv::NORM_HAMMING;
        }
        
        matcher = cv::BFMatcher::create(normType, false);
    }
    else if (matcherType.compare("MAT_FLANN") == 0)
    {
        if (descSource.type() != CV_32F) {
            descSource.convertTo(descSource, CV_32F);
            descRef.convertTo(descRef, CV_32F);
        }
        matcher = cv::DescriptorMatcher::create(cv::DescriptorMatcher::FLANNBASED);
    }
    
    // perform matching task
    if (selectorType.compare("SEL_NN") == 0)
    { // nearest neighbor (best match)

        matcher->match(descSource, descRef, matches); // Finds the best match for each descriptor in desc1
    }
    else if (selectorType.compare("SEL_KNN") == 0)
    { // k nearest neighbors (k=2)
        int k = 2;
        float ratio = 0.8;
        std::vector<std::vector<cv::DMatch>> knn_matches;

        matcher->knnMatch(descSource, descRef, knn_matches, k);

        for (size_t i = 0; i < knn_matches.size(); i++) {
            if (knn_matches[i][0].distance < ratio * knn_matches[i][1].distance) {
                matches.emplace_back(knn_matches[i][0]);
            }
        }

    }
}
```

## Evaluation

Exploring all the possible combinations of detector and descriptor (AKAZE descriptor only works with AKAZE_Point, SIFT doesn't work with ORB), the results are concluded in the following. 

1. DetectorType SHITOMASI got keyPoints 1054

* DescriptorType BRISK matches 767 features in total 3.27234 seconds
* DescriptorType BRIEF matches 944 features in total 0.218777 seconds
* DescriptorType ORB matches 908 features in total 0.226006 seconds
* DescriptorType FREAK matches 768 features in total 0.534048 seconds
* DescriptorType SIFT matches 927 features in total 0.316537 seconds

2. DetectorType HARRIS got keyPoints 231

* DescriptorType BRISK matches 142 features in total 3.22073 seconds
* DescriptorType BRIEF matches 173 features in total 0.199038 seconds
* DescriptorType ORB matches 162 features in total 0.205538 seconds
* DescriptorType FREAK matches 144 features in total 0.542588 seconds
* DescriptorType SIFT matches 163 features in total 0.317492 seconds

3. DetectorType FAST got keyPoints 3675

* DescriptorType BRISK matches 2183 features in total 3.17183 seconds
* DescriptorType BRIEF matches 2831 features in total 0.118374 seconds
* DescriptorType ORB matches 2768 features in total 0.1182 seconds
* DescriptorType FREAK matches 2233 features in total 0.482162 seconds
* DescriptorType SIFT matches 2782 features in total 0.386303 seconds

4. DetectorType BRISK got keyPoints 2498

* DescriptorType BRISK matches 1570 features in total 6.59106 seconds
* DescriptorType BRIEF matches 1704 features in total 3.48385 seconds
* DescriptorType ORB matches 1514 features in total 3.52223 seconds
* DescriptorType FREAK matches 1524 features in total 3.85935 seconds
* DescriptorType SIFT matches 1646 features in total 3.85931 seconds

5. DetectorType ORB got keyPoints 1069

* DescriptorType BRISK matches 751 features in total 3.17002 seconds
* DescriptorType BRIEF matches 545 features in total 0.14438 seconds
* DescriptorType ORB matches 763 features in total 0.178467 seconds
* DescriptorType FREAK matches 420 features in total 0.48884 seconds
* DescriptorType SIFT matches 763 features in total 0.556782 seconds

6. DetectorType AKAZE got keyPoints 1504

* DescriptorType BRISK matches 1215 features in total 3.796 seconds
* DescriptorType BRIEF matches 1266 features in total 0.720245 seconds
* DescriptorType ORB matches 1182 features in total 0.750594 seconds
* DescriptorType FREAK matches 1187 features in total 1.07918 seconds
* DescriptorType AKAZE matches 1259 features in total 1.34859 seconds
* DescriptorType SIFT matches 1270 features in total 0.972742 seconds

7. DetectorType SIFT got keyPoints 1248

* DescriptorType BRISK matches 592 features in total 4.15418 seconds
* DescriptorType BRIEF matches 702 features in total 1.25924 seconds
* DescriptorType FREAK matches 593 features in total 1.58729 seconds
* DescriptorType SIFT matches 800 features in total 1.88088 seconds

The fastest 3 combinations is:
* FAST + ORB : 0.118200s
* FAST + BRIEF :  0.118374s
* ORB + BRIEF : 0.144380s

FAST can also detect most feature (also matching features). Of course, this depends on configurations.
