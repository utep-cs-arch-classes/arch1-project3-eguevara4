# makfile configuration
CPU             	= msp430g2553
CFLAGS          	= -mmcu=${CPU} -Os -I../h -I../h

#switch the compiler (for the internal make rules)
CC              = msp430-elf-gcc
AS              = msp430-elf-gcc -mmcu=${CPU} -c

all:game.elf

#additional rules for files
game.elf: ${COMMON_OBJECTS} speaker.o mainGame.o wdt_handler.o
	${CC} -mmcu=${CPU}   -o $@ $^ -L../lib -lTimer -lLcd -lShape -lCircle -lp2sw

load: game.elf
	mspdebug rf2500 "prog $^"

clean:
	rm -f *.o *.elf
