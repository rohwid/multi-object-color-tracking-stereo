#include <iostream>
#include <string>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

using namespace cv;
using namespace std;

int main( int argc, char** argv )
{
  // Capture the video from webcam
  VideoCapture capleft(1);
  VideoCapture capright(2);

  // if not success, exit program
  if ( !capleft.isOpened() || !capright.isOpened())
  {
    cout << "Cannot open the web cam" << endl;
    return -1;
  }

  // Create a window called "Control"
  namedWindow("Control", CV_WINDOW_AUTOSIZE);

  int iLowH = 0;
  int iHighH = 90;

  int iLowS = 122;
  int iHighS = 255;

  int iLowV = 81;
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
  /* ===================================== */

  int iLastX[2], iLastY[2];
  for (int i=0; i<2; i++)
  {
    iLastX[i] = -1;
    iLastY[i] = -1;
  }

  // Capture a temporary image from the camera
  Mat imgTmp[2];
  capleft.read(imgTmp[0]);
  capright.read(imgTmp[1]);

  while (true)
  {
    Mat imgOriginal[2];
    bool bSuccess[2];

    bSuccess[0] = capleft.read(imgOriginal[0]); // read a new frame from video
    bSuccess[1] = capright.read(imgOriginal[1]);

    if (!bSuccess[0]||!bSuccess[1]) //if not success, break loop
    {
      cout << "Cannot read a frame from video stream" << endl;
      break;
    }

    Mat imgHSV[2], imgThresholded[2];
    for(int i=0; i<2; i++)
    {
      cvtColor(imgOriginal[i], imgHSV[i], COLOR_BGR2HSV);
      inRange(imgHSV[i], Scalar(iLowH, iLowS, iLowV), Scalar(iHighH, iHighS, iHighV), imgThresholded[i]); //Threshold the imag

      // morphological opening (removes small objects from the foreground) camera 1
      erode(imgThresholded[i], imgThresholded[i], getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );
      dilate(imgThresholded[i], imgThresholded[i], getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );

      // morphological closing (removes small holes from the foreground) camera 1
      dilate(imgThresholded[i], imgThresholded[i], getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );
      erode(imgThresholded[i], imgThresholded[i], getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );
    }

    //show the thresholded image
    imshow("Thresholded Image camera 1", imgThresholded[0]);
    imshow("Thresholded Image camera 2", imgThresholded[1]);

    //show the original image
    imshow("Original camera 1", imgOriginal[0]);
    imshow("Original camera 2", imgOriginal[1]);

    if (waitKey(30) == 27) //wait for 'esc' key press for 30ms. If 'esc' key is pressed, break loop
    {
      cout << "esc key is pressed by user" << endl;
      break;
    }
  }

  return 0;
}
