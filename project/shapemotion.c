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


#define GREEN_LED BIT6

u_int score = 0;
u_int level = 1;
u_int bombs = 2;
u_char gameOn = 1;

AbRect rect10 = {abRectGetBounds, abRectCheck, {0,1}}; /**< 10x10 rectangle */
AbShip  ship = {abShipGetBounds, abShipCheck, 20};

AbRectOutline fieldOutline = {	/* playing field */
  abRectOutlineGetBounds, abRectOutlineCheck,   
  {screenWidth/2 - 1, screenHeight/2 - 10}
};

Layer laser = {		/**< Layer with an orange circle */
  (AbShape *)&rect10,
  {(screenWidth/2)-10, screenHeight-15}, /**< bit below & right of center */
  {0,0}, {0,0},				    /* last & next pos */
  COLOR_GREEN,
  0,
};

Layer layer3 = {		/**< Layer with an orange circle */
  (AbShape *)&circle6,
  {(screenWidth/2)-30, 30}, /**< bit below & right of center */
  {0,0}, {0,0},				    /* last & next pos */
  COLOR_RED,
  &laser,
};

Layer layer2 = {		/**< Layer with an orange circle */
  (AbShape *)&circle6,
  {(screenWidth/2), 30}, /**< bit below & right of center */
  {0,0}, {0,0},				    /* last & next pos */
  COLOR_RED,
  &layer3,
};

Layer layer1 = {		/**< Layer with an orange circle */
  (AbShape *)&circle6,
  {(screenWidth/2)+30, 30}, /**< bit below & right of center */
  {0,0}, {0,0},				    /* last & next pos */
  COLOR_RED,
  &layer2,
};

Layer fieldLayer = {		/* playing field as a layer */
  (AbShape *) &fieldOutline,
  {screenWidth/2, screenHeight/2+9},/**< center */
  {0,0}, {0,0},				    /* last & next pos */
  COLOR_GRAY,
  &layer1
};

Layer shipLayer = {		/**< Layer with a red square */
  (AbShape *)&ship,
  {screenWidth/2-10, screenHeight-15}, /**< center */
  {0,0}, {0,0},				    /* last & next pos */
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
MovLayer ml5 = { &laser, {0,0}, 0 }; 
MovLayer ml4 = { &layer3, {-1,1}, &ml5 };
MovLayer ml3 = { &layer2, {1,1}, &ml4 };
MovLayer ml2 = { &layer1, {1,1}, &ml3 }; /**< not all layers move */ 
MovLayer ml0 = { &shipLayer, {0,0}, &ml2 }; 



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
    }
    if(ml->velocity.axes[1] > 3){
      ml5.velocity.axes[1] = 0;
      ml5.layer->pos.axes[0] = ml0.layer->pos.axes[0];
      ml5.layer->pos.axes[1] = ml0.layer->pos.axes[1]-10;
      ml5.layer->posLast.axes[0] = ml0.layer->posLast.axes[0];
      ml5.layer->posLast.axes[1] = ml0.layer->posLast.axes[1];
      ml5.layer->posNext.axes[0] = ml0.layer->posNext.axes[0];
      ml5.layer->posNext.axes[1] = ml0.layer->posNext.axes[1];
    }
  } /**< for ml */
}

void adjustLevel(MovLayer *ml){
  switch(level){
  case 1:
    if(ml->velocity.axes[0] > 0)
      ml->velocity.axes[0] = 1;
    else
      ml->velocity.axes[0] = -1;
    if(ml->velocity.axes[1] > 0)
      ml->velocity.axes[1] = 1;
    else
      ml->velocity.axes[1] = -1;
    break;
  case 2:
    if(ml->velocity.axes[0] > 0)
      ml->velocity.axes[0] = 1;
    else
      ml->velocity.axes[0] = -1;
    if(ml->velocity.axes[1] > 0)
      ml->velocity.axes[1] = 2;
    else
      ml->velocity.axes[1] = -2;
    break;
  case 3:
    if(ml->velocity.axes[0] > 0)
      ml->velocity.axes[0] = 2;
    else
      ml->velocity.axes[0] = -2;
    if(ml->velocity.axes[1] > 0)
      ml->velocity.axes[1] = 2;
    else
      ml->velocity.axes[1] = -2;
    break;
  case 4:
    if(ml->velocity.axes[0] > 0)
      ml->velocity.axes[0] = 2;
    else
      ml->velocity.axes[0] = -2;
    if(ml->velocity.axes[1] > 0)
      ml->velocity.axes[1] = 3;
    else
      ml->velocity.axes[1] = -3;
    break;
  default:
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

void checkCollision(MovLayer *ml){
  u_char axis;
  u_char step = 0;
  
  ml = ml->next;
  
  for (; ml && step < 3; ml = ml->next) {

    adjustLevel(ml);
    if(ml0.layer->pos.axes[0] < ml->layer->pos.axes[0] + 10 &&
        ml0.layer->pos.axes[0] > ml->layer->pos.axes[0] - 10 &&
        ml0.layer->pos.axes[1] < ml->layer->pos.axes[1] + 10 &&
        ml0.layer->pos.axes[1] > ml->layer->pos.axes[1] - 10){

      gameOn = 0;
      return;
    }
      
      if(ml5.layer->pos.axes[0] < ml->layer->pos.axes[0] + 7 &&
        ml5.layer->pos.axes[0] > ml->layer->pos.axes[0] - 7 &&
        ml5.layer->pos.axes[1] < ml->layer->pos.axes[1] - 4 &&
        ml5.layer->pos.axes[1] > ml->layer->pos.axes[1] - 15){

	score++;
	if(level < 5){
          level = (score/10)+1;
	}else{
	  level = 5;
	}
	if(score % 20 == 0){
	  bombs++;
	}
	if(ml5.velocity.axes[1] < -5 && ml5.layer->pos.axes[1] < 30){
          ml5.layer->color = COLOR_BLACK;
        }
	if(ml5.velocity.axes[1] < -3){
          ml5.velocity.axes[1] = 0;
          ml5.layer->pos.axes[0] = ml0.layer->pos.axes[0];
	  ml5.layer->pos.axes[1] = ml0.layer->pos.axes[1]-10;
	  ml5.layer->posLast.axes[0] = ml0.layer->posLast.axes[0];
          ml5.layer->posLast.axes[1] = ml0.layer->posLast.axes[1];
	  ml5.layer->posNext.axes[0] = ml0.layer->posNext.axes[0];
	  ml5.layer->posNext.axes[1] = ml0.layer->posNext.axes[1];
          ml5.layer->color = COLOR_BLACK;
        }
        ml->layer->posNext.axes[1] = 30;
        return;
      }/**< for axis */
    step++;
  }
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
  p2sw_init(15);

  layerInit(&shipLayer);
  layerDraw(&shipLayer);

  layerGetBounds(&fieldLayer, &fieldFence);

  enableWDTInterrupts();      /**< enable periodic interrupt */
  or_sr(0x8);	              /**< GIE (enable interrupts) */
  
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
    }
  }
}

char down = 0;
/** Watchdog timer interrupt handler. 15 interrupts/sec */
void wdt_c_handler()
{
  static short count = 0;
  P1OUT |= GREEN_LED;		      /**< Green LED on when cpu on */
  count ++;
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
    }
    if(~switches & 4){
      down = 1;
    }
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
    }
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
      }
    }
    redrawScreen = 1;
    count = 0;
  }
  
  P1OUT &= ~GREEN_LED;		    /**< Green LED off when cpu off */
}
