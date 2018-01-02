#include <iostream>
#include <string>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

using namespace cv;
using namespace std;

Mat imgOriginal[2], imgHSV[2], imgThresholded[2];

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

int main( int argc, char** argv )
{
  // Capture the video from webcam
  VideoCapture capleft(1); // CAM1
  VideoCapture capright(2); // CAM2

  // if not success, exit program
  if ( !capleft.isOpened() || !capright.isOpened())
  {
    cout << "Cannot open the web cam" << endl;
    return -1;
  }

  // Create a window called "Control"
  namedWindow("Control", CV_WINDOW_AUTOSIZE);

  int iLowH = 40;
  int iHighH = 78;

  int iLowS = 90;
  int iHighS = 255;

  int iLowV = 70;
  int iHighV = 255;

  /* Create trackbars in "Control" window */
  // Hue (0 - 179)
  createTrackbar("LowH", "Control", &iLowH, 179);
  createTrackbar("HighH", "Control", &iHighH, 179);

  // Saturation (0 - 255)
  createTrackbar("LowS", "Control", &iLowS, 255);
  createTrackbar("HighS", "Control", &iHighS, 255);

  // Value (0 - 255)
  createTrackbar("LowV", "Control", &iLowV, 255);
  createTrackbar("HighV", "Control", &iHighV, 255);

  while (true)
  {
    bool bSuccess[2];

    // read a new frame from video
    bSuccess[0] = capleft.read(imgOriginal[0]); // CAM1
    bSuccess[1] = capright.read(imgOriginal[1]); // CAM2

    // break loop if not success
    if (!bSuccess[0] || !bSuccess[1])
    {
      cout << "Cannot read a frame from video stream" << endl;
      break;
    }

    for(int i=0; i < 2; i++)
    {
      // HSV image
      cvtColor(imgOriginal[i], imgHSV[i], COLOR_BGR2HSV);

      // threshold the image
      inRange(imgHSV[i], Scalar(iLowH, iLowS, iLowV), Scalar(iHighH, iHighS, iHighV), imgThresholded[i]);

      /*
      // Memory for hough circles
      CvMemStorage* storage = cvCreateMemStorage(0);

      // hough detector works better with some smoothing of the image
      cvSmooth(imgThresholded[i], imgThresholded[i], CV_GAUSSIAN, 9, 9);
      CvSeq* circles = cvHoughCircles(imgThresholded[i], storage, CV_HOUGH_GRADIENT, 2, imgThresholded[i] -> height/4, 100, 50, 10, 400);

      for (int j = 0; j < circles -> total; j++)
      {
        float* p = (float*)cvGetSeqElem(circles, j);
        printf("Ball! x=%f y=%f r=%f\n\r",p[0],p[1],p[2]);
        cvCircle(frame, cvPoint(cvRound(p[0]),cvRound(p[1])), 3, CV_RGB(0,255,0), -1, 8, 0);
        cvCircle(frame, cvPoint(cvRound(p[0]),cvRound(p[1])), cvRound(p[2]), CV_RGB(255,0,0), 3, 8, 0);
      }
      */

      // morphological opening (removes small objects from the foreground) camera 1
      erode(imgThresholded[i], imgThresholded[i], getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
      dilate(imgThresholded[i], imgThresholded[i], getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));

      // morphological closing (removes small holes from the foreground) camera 1
      dilate(imgThresholded[i], imgThresholded[i], getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
      erode(imgThresholded[i], imgThresholded[i], getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
    }

    // calculate the moments of the thresholded image
    Moments oMoments[2];

    double dM01[2], dM10[2], dArea[2];

    for(int i=0; i < 2; i++)
    {
      oMoments[i] = moments(imgThresholded[i]);
      dM01[i] = oMoments[i].m01;
      dM10[i] = oMoments[i].m10;
      dArea[i] = oMoments[i].m00;
    }

    // if the area <= 50000,
    // we consider that the there are no object in the image and it's because of the NOISE
    if (dArea[0] > 50000 && dArea[1] >50000)
    {
      // calculate the position of the ball
      int posX[2], posY[2];

      for (int i = 0; i < 2; i++)
      {
        posX[i] = dM10[i] / dArea[i];
        posY[i] = dM01[i] / dArea[i];
      }

      for (int i = 0; i < 2; i++)
      {
        int w = imgThresholded[0].size().width;
        int h = imgThresholded[0].size().height;

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
        int a_x2 = posX[1];
        int a_z2 = 0;

        // line B coord x1z1
        float b_x1 = 0.5 * cx1 + d;
        int b_z1 = -f;

        // line B coord x2z2
        int b_x2 = posX[0];
        int b_z2 = 0;

        float z = get_z(a_x1, a_x2, a_z1, a_z2, b_x1, b_x2, b_z1, b_z2);

        // print log
        cout<< "Point X camera1 = " <<posX[0] << " Point Y camera1= " << posY[0] << endl;
        cout<< "Point X camera2 = " << posX[1] << " Point Y camera2= " << posY[1] << endl;
        cout<< "Depth = "<< z << endl;
        cout<< "Resolution = " << w << " x " << h << endl;
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
