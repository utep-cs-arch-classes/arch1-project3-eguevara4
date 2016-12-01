# arch1-project3-eguevara4

This directory contains:
* Code that implements a small shooting game.
* Also see directory shapeLib and lcdLib for added code 

This directory contains the following files and directories:

* mainGame.c: Main program file that initializes all needed components and contains logic for the switches and animation
* Makefile: Makefile for compiling loading and cleaning
* speaker.c: File for speaker functions
* speaker.h: Header file for speaker.c
* wdt_handler.s: Assembly code for WDT

To compile program (requires make in parent directory first):
~~~
$ make
~~~

To load program:
~~~
$ make load
~~~

To delete binaries:
~~~
$ make clean
~~~

# How to use game

## Controls
* S1 move ship left
* S2 move ship right
* S3 drop bomb
* S4 shoot laser
* RESET reset game

## Things to know

When the program is loaded into the MSP430 the screen is cleared and the game begins.
The level, score, and number of bombs display at the top, the default values are 1, 0, 2 respectively.
The asteroids begin to drop once the game has been loaded.

## Gameplay

The objective of the game is to destroy as may asteroids as you can without letting one crash into the ship.
There is a total of 3 asteroids and their velocity increases with every level.
Tha max level is level 5.
Every asteroid destroyed counts as 1 point.
When a bomb is used it counts as 3 points and all asteroids are destroyed.
Every 20 points a bomb is awarded.
Every 10 points the level is increased.
Once an asteroid crashes into the ship the game is over and the game must be reset.

# Switch Ship state

~~~
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
~~~

Assembly
~~~
                       .data
switches             : .word                              ; switches is int
ml0                  : .word                              ; ml0 is pointer
ml5                  : .word                              ; ml5 is pointer

                       .text
switch1              :                                    ; start of state transition code
                       mov   &switches , r12              ; r12 is switches
		       mov   #-1       , r13              ; r13 is all 1s in binary
		       xor   r12       , r13              ; effectively ~switches
		       and   #1        , r13              ; and with 1
		       cmp   #0        , r13              ; r13 - 0
		       JZ    switch2
		       mov   &ml0      , r4               ; r4 is ml0
		       mov   2(r4)     , r5               ; r5 is ml0.velocity
		       mov   #-3       , 0(r5)            ; ml0.velocity.axes[0] = -3
		       mov   &ml5      , r6               ; r6 is ml5
		       mov   2(r6)     , r7               ; r7 is ml5.velocity
		       cmp   #0        , 2(r7)            ; ml5.velocity.axes[1] - 0
		       JNZ   end
		       mov   #-3       , 0(r7)            ; ml5.velocity.axes[0] = -3
		       JMP   end

switch2              : mov   &switches , r12              ; r12 is switches
		       mov   #-1       , r13              ; r13 is all 1s in binary
		       xor   r12       , r13              ; effectively ~switches
		       and   #2        , r13              ; and with 2
		       cmp   #0        , r13              ; r13 - 0
		       JZ    not1or2
		       mov   &ml0      , r4               ; r4 is ml0
		       mov   2(r4)     , r5               ; r5 is ml0.velocity
		       mov   #3        , 0(r5)            ; ml0.velocity.axes[0] = 3
		       mov   &ml5      , r6               ; r6 is ml5
		       mov   2(r6)     , r7               ; r7 is ml5.velocity
		       cmp   #0        , 2(r7)            ; ml5.velocity.axes[1] - 0
		       JNZ   end
		       mov   #3        , 0(r7)            ; ml5.velocity.axes[0] = 3
		       JMP   end

not1or2              : mov   &ml0      , r4               ; r4 is ml0
		       mov   2(r4)     , r5               ; r5 is ml0.velocity
		       mov   #0        , 0(r5)            ; ml0.velocity.axes[0] = 0
		       mov   &ml5      , r6               ; r6 is ml5
		       mov   2(r6)     , r7               ; r7 is ml5.velocity
		       mov   #0        , 0(r7)            ; ml5.velocity.axes[0] = 0
end                  :                                    ; continue function
~~~