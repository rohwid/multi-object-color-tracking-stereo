#include "multitrack.h"

using namespace std;

TrackPoint::TrackPoint(void)
{

}

TrackPoint::~TrackPoint(void)
{

}

int TrackPoint::getXPos()
{
  return xPos;
}

void TrackPoint::setXPos(int x)
{
  xPos = x;
}

int TrackPoint::getYPos()
{
  return yPos;
}

void TrackPoint::setYPos(int y)
{
  yPos = y;
}
