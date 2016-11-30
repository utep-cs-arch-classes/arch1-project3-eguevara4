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

    /* ship is made from 6 rectagles */

    if (p < bounds.botRight.axes[axis]-9 && p > bounds.topLeft.axes[axis]+9
	&& p2 < bounds.botRight.axes[axis+1]-3 && p2 > bounds.topLeft.axes[axis+1]+1)/**< rectangle 0 */
      within = 1;
    /*       0
             0
             0
             0
             0
             0
             0 
                      */      
    
    if (p < bounds.botRight.axes[axis]-8 && p > bounds.topLeft.axes[axis]+8
 && p2 < bounds.botRight.axes[axis+1]-2 && p2 > bounds.topLeft.axes[axis+1]+3)/**< rectangle 1 */
      within = 1;
    /*       0
            111
            111
            111
            111
            111
            111        
            111       */

    if (p < bounds.botRight.axes[axis]-7 && p > bounds.topLeft.axes[axis]+7
 && p2 < bounds.botRight.axes[axis+1]-3 && p2 > bounds.topLeft.axes[axis+1]+8)/**< rectangle 2 */
      within = 1;
    /*       0
            111
            111
           22222
           22222
           22222
           22222        
            111       */

    if (p < bounds.botRight.axes[axis]-2 && p > bounds.topLeft.axes[axis]+2
 && p2 < bounds.botRight.axes[axis+1]-3 && p2 > bounds.topLeft.axes[axis+1]+13)/**< rectangle 3 */
      within = 1;
    /*       0
            111
            111
           22222
           22222
      333333333333333
      333333333333333   
            111       */
    
    if (p < bounds.botRight.axes[axis]-17 && p > bounds.topLeft.axes[axis]+1
 && p2 < bounds.botRight.axes[axis+1]-2 && p2 > bounds.topLeft.axes[axis+1]+8)/**< rectangle 4 */
      within = 1;
    /*       0
            111
            111
      4    22222
      4    22222
      433333333333333
      433333333333333   
      4     111       */
    
    if (p < bounds.botRight.axes[axis]-1 && p > bounds.topLeft.axes[axis]+17
 && p2 < bounds.botRight.axes[axis+1]-2 && p2 > bounds.topLeft.axes[axis+1]+8)/**< rectangle 5 */
      within = 1;
    /*       0
            111
            111
      4    22222    5
      4    22222    5
      433333333333335
      433333333333335   
      4     111     5 */
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
