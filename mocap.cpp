#include <sstream>
#include <iostream>
#include <string>
#include <vector>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "multitrack.h"

using namespace cv;
using namespace std;

// set HSV value
int iLowH = 45;
int iHighH = 75;

int iLowS = 50;
int iHighS = 255;

int iLowV = 40;
int iHighV = 255;

// max number of trackable objects
const int MAX_NUM_OBJECTS = 6;
const int MIN_OBJECT_AREA = 500;

int ALL_X[2][50];
int ALL_Y[2][50];

// window variable
const string trackbarWindowName = "HSV Trackbars";
const string imageOrignalWindowLeft = "Image Original Left";
const string imageOrignalWindowRight = "Image Original Right";
const string imageThresholdWindowLeft = "Image Thresholded Left";
const string imageThresholdWindowRight = "Image Thresholded Right";

int numBall;

// camera distance -> calib (extrinsics)
float d = 2.8; // round from T = 2.8

// principal point -> calib (intrinsics)
int cx1 = 308; // M1 (camera1)
int cx2 = 322; // M2 (camera2)

// focal length -> calib (intrinsics)
int f = 550; // M1 (camera1) & M2 (camera2)

// line A coord x1z1
float a_x1 = 0.5 * cx2;
int a_z1 = -f;

// line A coord x2z2
int a_z2 = 0;

// line B coord x1z1
float b_x1 = 0.5 * cx1 + d;
int b_z1 = -f;

// line B coord x2z2
int b_z2 = 0;


string intToString(int number)
{
	std::stringstream ss;
	ss << number;
	return ss.str();
}

float get_z(float ax1, int ax2, int az1, int az2, float bx1, int bx2, int bz1, int bz2)
{
  float ba = ax2 - ax1;
  float da = az2 - az1;

  /* get coord two line crosses
  (x - ax1)*da = (z - az1)*ba
  (da*x)-(da*ax1) = (ba*z) - (ba*az1)
  (da*x) - (ba*z) - (da*ax1) + (ba*az1) = 0
  */

  float bb = bx2 - bx1;
  float db = bz2 - bz1;

  /* get coord two line crosses
  (x - bx1)*db = (z - bz1)*bb
  (db*x)-(db*bx1) = (bb*z) - (bb*bz1)
  (db*x) - (bb*z) - (db*bx1) + (bb*bz1) = 0
  */

  float z1 = db * ((da * ax1) + (ba * az1));
  float z2 = da * ((db * bx1) + (bb * bz1));
  float z3 = da * bb;
  float z4 = db * ba;

  return ((z1 - z2) / (z3 - z4));
}

void drawObject(vector<TrackPoint> theBallPoints, Mat &frame, int numObject, int camNumber)
{
	for (int i = 0; i < theBallPoints.size(); i++)
	{
		cv::circle(frame, cv::Point(theBallPoints.at(i).getXPos(), theBallPoints.at(i).getYPos()), 10, cv::Scalar(0, 0, 255));
		cv::putText(frame, intToString(theBallPoints.at(i).getXPos())+ " , " + intToString(theBallPoints.at(i).getYPos()), cv::Point(theBallPoints.at(i).getXPos(), theBallPoints.at(i).getYPos() + 20), 1, 1, Scalar(0, 255, 0));

		// printf("Point X camera%d = %d Point Y camera%d = %d\n", camNumber, theBallPoints.at(i).getXPos(), camNumber, theBallPoints.at(i).getYPos());

		ALL_X[camNumber][i] = theBallPoints.at(i).getXPos();
		ALL_Y[camNumber][i] = theBallPoints.at(i).getYPos();
	}
}

void trackFilteredObject(Mat threshold, Mat HSV, Mat &imgOriginal, int camNumber)
{
	// store point coordinate
	vector <TrackPoint> ballPoints;

  int posX, posY;

	Mat temp;
	threshold.copyTo(temp);

  // store point location by contour
	vector < vector<Point> > contours;
  // store number of detected contour
	vector <Vec4i> hierarchy;

  // find contours of filtered image using openCV findContours function
	findContours(temp, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE );

	// use moments method to find our filtered object
	double refArea = 0;
	bool objectFound = false;

  if (hierarchy.size() > 0)
	{
		int numObjects = hierarchy.size();

		// if number of objects greater than MAX_NUM_OBJECTS we have a noisy filter
		if (numObjects < MAX_NUM_OBJECTS)
		{
      for (int i = 0; i >= 0; i = hierarchy[i][0])
			{
				Moments moment = moments((cv::Mat)contours[i]);

				double area = moment.m00;

				if(area > MIN_OBJECT_AREA)
				{
					TrackPoint ballPoint;

          posX = moment.m10 / area;
          posY = moment.m01 / area;

					ballPoint.setXPos(posX);
					ballPoint.setYPos(posY);

					ballPoints.push_back(ballPoint);

					numBall = ballPoints.size();

					objectFound = true;
				}
				else
				{
					objectFound = false;

					numBall = 0;
				}
			}

			if(objectFound == true)
			{
				drawObject(ballPoints, imgOriginal, numObjects, camNumber);
			}
		}
		else
		{
			putText(imgOriginal, "TOO MUCH NOISE! ADJUST FILTER", Point(0, 50), 1, 2, Scalar(0, 0, 255), 2);
		}
	}
}

void morphOps(Mat &imgThres)
{
  // morphological opening (removes small objects from the foreground) camera 1
  erode(imgThres, imgThres, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
  dilate(imgThres, imgThres, getStructuringElement(MORPH_ELLIPSE, Size(8, 8)));

  // morphological closing (removes small holes from the foreground) camera 1
  dilate(imgThres, imgThres, getStructuringElement(MORPH_ELLIPSE, Size(8, 8)));
  erode(imgThres, imgThres, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
}

void openTrackbars()
{
  // create a window called "Control"
  namedWindow(trackbarWindowName, CV_WINDOW_AUTOSIZE);

  // Hue (0 - 179)
  createTrackbar("LowH", trackbarWindowName, &iLowH, 256);
  createTrackbar("HighH", trackbarWindowName, &iHighH, 256);

  // Saturation (0 - 255)
  createTrackbar("LowS", trackbarWindowName, &iLowS, 256);
  createTrackbar("HighS", trackbarWindowName, &iHighS, 256);

  // Value (0 - 255)
  createTrackbar("LowV", trackbarWindowName, &iLowV, 256);
  createTrackbar("HighV", trackbarWindowName, &iHighV, 256);
}

int main( int argc, char** argv )
{
  // image variable
  Mat imgOriginal[2], imgHSV[2], imgThresholded[2];

  // get x position for calculate z
  int posX[2];

  // enable camera calibration
  bool calibMode = true;

  // capture the video from webcam
  VideoCapture capleft(1); // CAM1
  VideoCapture capright(2); // CAM2

  // if not success, exit program
  if (!capleft.isOpened() || !capright.isOpened())
  {
    cout << "Cannot open the web cam" << endl;
    return -1;
  }

  if (calibMode == true)
  {
    openTrackbars();
  }

  while(true)
  {
    bool bSuccess[2];

		int posX[2];

    // read a new frame from video
    bSuccess[0] = capleft.read(imgOriginal[0]); // CAM1 640x480
    bSuccess[1] = capright.read(imgOriginal[1]); // CAM2 640x80

    // break loop if not success
    if (!bSuccess[0] || !bSuccess[1])
    {
      cout << "Cannot read a frame from video stream.." << endl;
      break;
    }

    if(calibMode == true)
    {
      for(int i = 0; i < 2; i++)
      {
        // HSV image
        cvtColor(imgOriginal[i], imgHSV[i], COLOR_BGR2HSV);

        // threshold the image
        inRange(imgHSV[i], Scalar(iLowH, iLowS, iLowV), Scalar(iHighH, iHighS, iHighV), imgThresholded[i]);

        morphOps(imgThresholded[i]);

        trackFilteredObject(imgThresholded[i], imgHSV[i], imgOriginal[i], i);

				//printf("Point X camera %d = %d Point Y camera %d = %d\n", i, ALL_X[i][0], i, ALL_Y[i][0]);
				//printf("Point X camera %d = %d Point Y camera %d = %d\n", i, ALL_X[i][1], i, ALL_Y[i][1]);
				//printf("Point X camera %d = %d Point Y camera %d = %d\n", i, ALL_X[i][2], i, ALL_Y[i][2]);
      }

			if (numBall < 1)
			{
				cout << "Object not found!" << endl;
			}
			else
			{
				for (int i = 0; i < numBall; i++) {
					// please parse a_x2[0] and b_x2[1]
					float z = get_z(a_x1, ALL_X[0][i], a_z1, a_z2, b_x1, ALL_X[1][i], b_z1, b_z2);
					cout << "Depth = "<< z << " Object " << i + 1 << endl;
				}
			}
    }

    // show the original image
    imshow("Original camera 1", imgOriginal[0]);
    imshow("Original camera 2", imgOriginal[1]);

    // show the thresholded image
    imshow("Thresholded Image camera 1", imgThresholded[0]);
    imshow("Thresholded Image camera 2", imgThresholded[1]);

    // wait for 'esc' key press for 30ms for break loop
    if (waitKey(30) == 27)
    {
      cout << "esc key is pressed by user" << endl;
      break;
    }
  }

  return 0;
}
