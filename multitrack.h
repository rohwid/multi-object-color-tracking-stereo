#include <string>

using namespace std;

class TrackPoint {
private:
  int xPos, yPos;

public:
  TrackPoint();
  virtual ~TrackPoint();

  int getXPos();
  void setXPos(int x);

  int getYPos();
  void setYPos(int y);
};
