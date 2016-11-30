/** \file shapemotion.c
 *  \brief This is a simple shape motion demo.
 *  This demo creates two layers containing shapes.
 *  One layer contains a rectangle and the other a circle.
 *  While the CPU is running the green LED is on, and
 *  when the screen does not need to be redrawn the CPU
 *  is turned off along with the green LED.
 */  
#include <stdio.h>
#include <stdlib.h>
#include <msp430.h>
#include <libTimer.h>
#include <lcdutils.h>
#include <lcddraw.h>
#include <p2switches.h>
#include <shape.h>
#include <abCircle.h>
#include "speaker.h"

#define GREEN_LED BIT6

u_int score = 0;
u_int level = 1;
u_int bombs = 2;
u_char gameOn = 1; /**< is game currently running */

AbRect rect10 = {abRectGetBounds, abRectCheck, {0,1}}; /**< 1x3 rectangle for laser */
AbShip  ship = {abShipGetBounds, abShipCheck, 20};     /**< ship */

AbRectOutline fieldOutline = {	/* playing field */
  abRectOutlineGetBounds, abRectOutlineCheck,   
  {screenWidth/2 - 1, screenHeight/2 - 10}
};

Layer laser = {		/**< Layer with an laser shape */
  (AbShape *)&rect10,
  {(screenWidth/2)-10, screenHeight-15},
  {0,0}, {0,0},				   
  COLOR_GREEN,
  0,
};

Layer layer3 = {		/**< Layer with an red circle */
  (AbShape *)&circle6,
  {(screenWidth/2)-30, 30},
  {0,0}, {0,0},				 
  COLOR_RED,
  &laser,
};

Layer layer2 = {		/**< Layer with an red circle */
  (AbShape *)&circle6,
  {(screenWidth/2), 30}, 
  {0,0}, {0,0},				  
  COLOR_RED,
  &layer3,
};

Layer layer1 = {		/**< Layer with an red circle */
  (AbShape *)&circle6,
  {(screenWidth/2)+30, 30}, 
  {0,0}, {0,0},				
  COLOR_RED,
  &layer2,
};

Layer fieldLayer = {		/* playing field as a layer */
  (AbShape *) &fieldOutline,
  {screenWidth/2, screenHeight/2+9},
  {0,0}, {0,0},			   
  COLOR_GRAY,
  &layer1
};

Layer shipLayer = {		/**< Layer with a ship */
  (AbShape *)&ship,
  {screenWidth/2-10, screenHeight-15}, 
  {0,0}, {0,0},			       
  COLOR_WHITE,
  &fieldLayer,
};

/** Moving Layer
 *  Linked list of layer references
 *  Velocity represents one iteration of change (direction & magnitude)
 */
typedef struct MovLayer_s {
  Layer *layer;
  Vec2 velocity;
  struct MovLayer_s *next;
} MovLayer;

/* initial value of {0,0} will be overwritten */
MovLayer ml5 = { &laser, {0,0}, 0 };       /**< laser */
MovLayer ml4 = { &layer3, {-1,1}, &ml5 };  /**< asteroid3 */
MovLayer ml3 = { &layer2, {1,1}, &ml4 };   /**< asteroid2 */
MovLayer ml2 = { &layer1, {1,1}, &ml3 };   /**< asteroid1 */ 
MovLayer ml0 = { &shipLayer, {0,0}, &ml2 };/**< ship */ 



movLayerDraw(MovLayer *movLayers, Layer *layers)
{
  int row, col;
  MovLayer *movLayer;

  and_sr(~8);			/**< disable interrupts (GIE off) */
  for (movLayer = movLayers; movLayer; movLayer = movLayer->next) { /* for each moving layer */
    Layer *l = movLayer->layer;
    l->posLast = l->pos;
    l->pos = l->posNext;
  }
  
  u_int temp = score;              /* Draws the score */
  u_int offset = 120;
  drawString5x7(60,1,"Score:",COLOR_GRAY,COLOR_BLACK);
  drawString5x7(1,1,"Level:",COLOR_GRAY,COLOR_BLACK);
  drawString5x7(1,10,"Bombs:   ",COLOR_GRAY,COLOR_BLACK);
  
  while(temp){
    drawDigit5x7(offset,1,temp%10,COLOR_GRAY,COLOR_BLACK);
    temp /= 10;
    offset -= 6;
  }
  
  temp = level;              /* Draws the level */
  offset = 50;
  while(temp){
    drawDigit5x7(offset,1,temp%10,COLOR_GRAY,COLOR_BLACK);
    temp /= 10;
    offset -= 6;
  }
  
  temp = bombs;              /* Draws the bombs left */
  offset = 50;
  if(bombs != 0){
  while(temp){
    drawDigit5x7(offset,10,temp%10,COLOR_GRAY,COLOR_BLACK);
    temp /= 10;
    offset -= 6;
  }
  }else{
    drawString5x7(1,10,"Bombs:   ",COLOR_GRAY,COLOR_BLACK);
    drawString5x7(1,10,"Bombs:  0",COLOR_GRAY,COLOR_BLACK);
  }
  
  or_sr(8);			/**< disable interrupts (GIE on) */


  for (movLayer = movLayers; movLayer; movLayer = movLayer->next) { /* for each moving layer */
    Region bounds;
    layerGetBounds(movLayer->layer, &bounds);
    lcd_setArea(bounds.topLeft.axes[0], bounds.topLeft.axes[1], 
		bounds.botRight.axes[0], bounds.botRight.axes[1]);
    for (row = bounds.topLeft.axes[1]; row <= bounds.botRight.axes[1]; row++) {
      for (col = bounds.topLeft.axes[0]; col <= bounds.botRight.axes[0]; col++) {
	Vec2 pixelPos = {col, row};
	u_int color = bgColor;
	Layer *probeLayer;
	for (probeLayer = layers; probeLayer; 
	     probeLayer = probeLayer->next) { /* probe all layers, in order */
	  if (abShapeCheck(probeLayer->abShape, &probeLayer->pos, &pixelPos)) {
	    color = probeLayer->color;
	    break; 
	  } /* if probe check */
	} // for checking all layers at col, row
	lcd_writeColor(color);
      } // for col
    } // for row
  } // for moving layer being updated.
}	  



//Region fence = {{10,30}, {SHORT_EDGE_PIXELS-10, LONG_EDGE_PIXELS-10}}; /**< Create a fence region */

/** Advances a moving shape within a fence
 *  
 *  \param ml The moving shape to be advanced
 *  \param fence The region which will serve as a boundary for ml
 */
void mlAdvance(MovLayer *ml, Region *fence)
{
  Vec2 newPos;
  u_char axis;
  Region shapeBoundary;
  
  for (; ml; ml = ml->next) {
    vec2Add(&newPos, &ml->layer->posNext, &ml->velocity);
    abShapeGetBounds(ml->layer->abShape, &newPos, &shapeBoundary);
    for (axis = 0; axis < 2; axis ++) {
      if ((shapeBoundary.topLeft.axes[axis] < fence->topLeft.axes[axis]) ||
	  (shapeBoundary.botRight.axes[axis] > fence->botRight.axes[axis])) {
	int velocity = ml->velocity.axes[axis] = -ml->velocity.axes[axis];
	newPos.axes[axis] += (2*velocity);
      }	/**< if outside of fence */
    } /**< for axis */
    ml->layer->posNext = newPos;
    
    if(ml->velocity.axes[1] < -5 && ml->layer->pos.axes[1] < 50){
      ml->layer->color = COLOR_BLACK;
    } /**< if laser too close to top boundary make the next laser black this is so the last laser image wont show */
    if(ml->velocity.axes[1] > 3){
      ml5.velocity.axes[1] = 0;
      ml5.layer->pos.axes[0] = ml0.layer->pos.axes[0];
      ml5.layer->pos.axes[1] = ml0.layer->pos.axes[1]-10;
      ml5.layer->posLast.axes[0] = ml0.layer->posLast.axes[0];
      ml5.layer->posLast.axes[1] = ml0.layer->posLast.axes[1];
      ml5.layer->posNext.axes[0] = ml0.layer->posNext.axes[0];
      ml5.layer->posNext.axes[1] = ml0.layer->posNext.axes[1];
    } /**< reset the position of the laser under the ship */
  } /**< for ml */
}

void adjustLevel(MovLayer *ml){
  switch(level){
  case 1: //**< level 1 velocity = {1,1} */
    if(ml->velocity.axes[0] > 0)
      ml->velocity.axes[0] = 1;
    else
      ml->velocity.axes[0] = -1;
    if(ml->velocity.axes[1] > 0)
      ml->velocity.axes[1] = 1;
    else
      ml->velocity.axes[1] = -1;
    break;
  case 2: //**< level 2 velocity = {2,1} */
    if(ml->velocity.axes[0] > 0)
      ml->velocity.axes[0] = 1;
    else
      ml->velocity.axes[0] = -1;
    if(ml->velocity.axes[1] > 0)
      ml->velocity.axes[1] = 2;
    else
      ml->velocity.axes[1] = -2;
    break;
  case 3: //**< level 3 velocity = {2,2} */
    if(ml->velocity.axes[0] > 0)
      ml->velocity.axes[0] = 2;
    else
      ml->velocity.axes[0] = -2;
    if(ml->velocity.axes[1] > 0)
      ml->velocity.axes[1] = 2;
    else
      ml->velocity.axes[1] = -2;
    break;
  case 4: //**< level 4 velocity = {3,2} */
    if(ml->velocity.axes[0] > 0)
      ml->velocity.axes[0] = 2;
    else
      ml->velocity.axes[0] = -2;
    if(ml->velocity.axes[1] > 0)
      ml->velocity.axes[1] = 3;
    else
      ml->velocity.axes[1] = -3;
    break;
  default: //**< level 5 (level wont go over 5)  velocity = {3,3} */
    if(ml->velocity.axes[0] > 0)
      ml->velocity.axes[0] = 3;
    else
      ml->velocity.axes[0] = -3;
    if(ml->velocity.axes[1] > 0)
      ml->velocity.axes[1] = 3;
    else
      ml->velocity.axes[1] = -3;
    break;
  }
}

/* checks laser and ship collison with asteroids only */
void checkCollision(MovLayer *ml){
  u_char axis;
  u_char step = 0;
  
  ml = ml->next; /**< skip first layer which is the ship */
  
  for (; ml && step < 3; ml = ml->next) {
    adjustLevel(ml);
    if(ml0.layer->pos.axes[0] < ml->layer->pos.axes[0] + 10 &&
        ml0.layer->pos.axes[0] > ml->layer->pos.axes[0] - 10 &&
        ml0.layer->pos.axes[1] < ml->layer->pos.axes[1] + 10 &&
        ml0.layer->pos.axes[1] > ml->layer->pos.axes[1] - 10){
      gameOn = 0;
      return;
    }/**< if ship collides with asteroid */
      
    if(ml5.layer->pos.axes[0] < ml->layer->pos.axes[0] + 7 &&
        ml5.layer->pos.axes[0] > ml->layer->pos.axes[0] - 7 &&
        ml5.layer->pos.axes[1] < ml->layer->pos.axes[1] - 4 &&
        ml5.layer->pos.axes[1] > ml->layer->pos.axes[1] - 15){
      score++;
      if(level < 5){
        level = (score/10)+1;
      }else{
	level = 5;
      }/**< increase level */
      if(score % 20 == 0){
        bombs++;
      }/**< add bomb if score is a multiple of 20 */
      if(ml5.velocity.axes[1] < -5 && ml5.layer->pos.axes[1] < 30){
        ml5.layer->color = COLOR_BLACK;
      }/**< hide last laser image */
      if(ml5.velocity.axes[1] < -3){
        ml5.velocity.axes[1] = 0;
        ml5.layer->pos.axes[0] = ml0.layer->pos.axes[0];
	ml5.layer->pos.axes[1] = ml0.layer->pos.axes[1]-10;
	ml5.layer->posLast.axes[0] = ml0.layer->posLast.axes[0];
        ml5.layer->posLast.axes[1] = ml0.layer->posLast.axes[1];
	ml5.layer->posNext.axes[0] = ml0.layer->posNext.axes[0];
	ml5.layer->posNext.axes[1] = ml0.layer->posNext.axes[1];
        ml5.layer->color = COLOR_BLACK;
      }/**< reposition laser under ship */
      ml->layer->posNext.axes[1] = 30; //**< move asteroid to the top */
      return;
    }/**< if laser collides with asteroid */
    step++;
  }/**< for asteroid only */
}

u_int bgColor = COLOR_BLACK;     /**< The background color */
int redrawScreen = 1;           /**< Boolean for whether screen needs to be redrawn */

Region fieldFence;		/**< fence around playing field  */

/** Initializes everything, enables interrupts and green LED, 
 *  and handles the rendering for the screen
 */

void main()
{
  P1DIR |= GREEN_LED;		/**< Green led on when CPU on */		
  P1OUT |= GREEN_LED;

  configureClocks();
  lcd_init();
  speaker_init();
  p2sw_init(15);

  layerInit(&shipLayer);
  layerDraw(&shipLayer);

  layerGetBounds(&fieldLayer, &fieldFence);

  enableWDTInterrupts();      /**< enable periodic interrupt */
  or_sr(0x8);	              /**< GIE (enable interrupts) */

  /* Set initial texts for score, bombs, and level */
  drawString5x7(60,1,"Score:    0",COLOR_GRAY,COLOR_BLACK);
  drawString5x7(1,1,"Level:  1",COLOR_GRAY,COLOR_BLACK);
  drawString5x7(1,10,"Bombs:  1",COLOR_GRAY,COLOR_BLACK);
  
  for(;;) { 
    while (!redrawScreen) { /**< Pause CPU if screen doesn't need updating */
      P1OUT &= ~GREEN_LED;    /**< Green led off witHo CPU */
      or_sr(0x10);	      /**< CPU OFF */
    }
    P1OUT |= GREEN_LED;       /**< Green led on when CPU on */
    redrawScreen = 0;
    
    if(!gameOn){
      drawString5x7(40,80,"YOU LOSE",COLOR_GRAY,COLOR_BLACK);
    }else{
      movLayerDraw(&ml0, &shipLayer);
    }/**< if game over display 'you lose' text and do not animate anymore */
  }
}

char down = 0; /**< used to check if bomb button was pressed */
u_int soundPeriod = 0; /**< counter for sound length */
u_char soundPlaying = 0; /**< is sound currently playing? */
u_int frequency = 500; /**< start frequency of the note that is played */

/** Watchdog timer interrupt handler. 15 interrupts/sec */
void wdt_c_handler()
{
  static short count = 0;
  P1OUT |= GREEN_LED;		      /**< Green LED on when cpu on */
  count ++;
  if(soundPlaying){
    soundPeriod++;
    frequency += 100;
    speaker_set_period(frequency);
  }/**< if sound is playing increase note frequency*/
  
  if(soundPeriod > 20){
    soundPeriod = 0;
    soundPlaying = 0;
    frequency = 200;
    speaker_set_period(0);
  }/**< if sound has played for a certain cycles then stop the sound */
  
  if (count == 15) {
    u_int switches = p2sw_read();
    if(!gameOn){
      return;
    }
    checkCollision(&ml0);
    mlAdvance(&ml0, &fieldFence);
    if (~switches & 1){
      ml0.velocity.axes[0] = -3;
      if(ml5.velocity.axes[1] == 0){
	ml5.velocity.axes[0] = -3;
      }
    }else if(~switches & 2){
      ml0.velocity.axes[0] = 3;
      if(ml5.velocity.axes[1] == 0){
	ml5.velocity.axes[0] = 3;
      }
    }else{
      ml0.velocity.axes[0] = 0;
      ml5.velocity.axes[0] = 0;
    }/**< if S1 is down move ship left else if S2 is down move ship right else dont move the ship */
    
    if(~switches & 4){
      down = 1;
    }/**< if S3 was pressed mark that it was pressed */
    
    if(switches & 4 && down){
      if(bombs > 0){
	bombs--;
	ml2.layer->posNext.axes[1] = 30;
	ml3.layer->posNext.axes[1] = 30;
	ml4.layer->posNext.axes[1] = 30;
	score += 3;
	level = (score/10)+1;
      }
      down = 0;
    }/**< if S3 is up and was previously down then drop bomb (this is like hitting all three asteroids at the same time) */
    
    if(~switches & 8){
      if(ml5.velocity.axes[1] == 0){
	ml5.velocity.axes[1] = -8;
	ml5.velocity.axes[0] = 0;
	ml5.layer->pos.axes[0] = ml0.layer->pos.axes[0];
	ml5.layer->pos.axes[1] = ml0.layer->pos.axes[1]-10;
	ml5.layer->posLast.axes[0] = ml0.layer->posLast.axes[0];
	ml5.layer->posLast.axes[1] = ml0.layer->posLast.axes[1];
	ml5.layer->posNext.axes[0] = ml0.layer->posNext.axes[0];
	ml5.layer->posNext.axes[1] = ml0.layer->posNext.axes[1];
	ml5.layer->color = COLOR_GREEN;
	speaker_set_period(frequency);
	soundPlaying = 1;
      }
    }/**< if S4 is down fire laser by setting its vertical velocity to -8 (Also begin making sound) */
    
    redrawScreen = 1;
    count = 0;
  }
  
  P1OUT &= ~GREEN_LED;		    /**< Green LED off when cpu off */
}
