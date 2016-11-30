#include "shape.h"


/** Check function required by AbShape
 *  abShip returns true if the ship includes the selected pixel
 */
int 
abShipCheck(const AbShip *ship, const Vec2 *centerPos, const Vec2 *pixel)
{
  Region bounds;
  abShipGetBounds(ship, centerPos, &bounds);
  int within = 0, axis;
  for (axis = 0; axis < 1; axis ++) {
    int p = pixel->axes[axis];
    int p2 = pixel->axes[axis+1];
    if (p < bounds.botRight.axes[axis]-9 && p > bounds.topLeft.axes[axis]+9
 && p2 < bounds.botRight.axes[axis+1]-3 && p2 > bounds.topLeft.axes[axis+1]+1)
      within = 1;

    if (p < bounds.botRight.axes[axis]-8 && p > bounds.topLeft.axes[axis]+8
 && p2 < bounds.botRight.axes[axis+1]-2 && p2 > bounds.topLeft.axes[axis+1]+3)
      within = 1;

    if (p < bounds.botRight.axes[axis]-7 && p > bounds.topLeft.axes[axis]+7
 && p2 < bounds.botRight.axes[axis+1]-3 && p2 > bounds.topLeft.axes[axis+1]+8)
      within = 1;

    if (p < bounds.botRight.axes[axis]-2 && p > bounds.topLeft.axes[axis]+2
 && p2 < bounds.botRight.axes[axis+1]-3 && p2 > bounds.topLeft.axes[axis+1]+13)
      within = 1;
    
    if (p < bounds.botRight.axes[axis]-17 && p > bounds.topLeft.axes[axis]+1
 && p2 < bounds.botRight.axes[axis+1]-2 && p2 > bounds.topLeft.axes[axis+1]+8)
      within = 1;
    
    if (p < bounds.botRight.axes[axis]-1 && p > bounds.topLeft.axes[axis]+17
 && p2 < bounds.botRight.axes[axis+1]-2 && p2 > bounds.topLeft.axes[axis+1]+8)
      within = 1;
  }
  return within;
}
  
/** Check function required by AbShape
 *  abShipGetBounds computes a ships's bounding box
 */
void 
abShipGetBounds(const AbShip *ship, const Vec2 *centerPos, Region *bounds)
{
  int size = ship->size, halfSize = size / 2;
  bounds->topLeft.axes[0] = centerPos->axes[0] - halfSize;
  bounds->topLeft.axes[1] = centerPos->axes[1] - halfSize;
  bounds->botRight.axes[0] = centerPos->axes[0] + halfSize;
  bounds->botRight.axes[1] = centerPos->axes[1] + halfSize;
}
