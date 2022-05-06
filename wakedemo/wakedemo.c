#include <msp430.h>
#include <libTimer.h>
#include "lcdutils.h"
#include "lcddraw.h"
#include "buzzer.h"
// WARNING: LCD DISPLAY USES P1.0.  Do not touch!!! 


typedef struct{
  short col,row;
} Pos;


Pos positions[] = {
  {10,10},
  {10, screenHeight - 10},
  {screenWidth - 10, screenHeight -10},
  {screenWidth - 10, 10},
  {screenWidth/2, screenHeight/2}
};




#define NUM_POSITION 5
unsinged short sqColors[] = {COLOR_RED, COLOR_GREEN, COLOR_ORANGE, COLOR_BLUE};
#define NUM_COLORS 4
#define BG_COLOR COLOR_BLACK
char current_position = 0, current_color= 0;
#define LED BIT6
#define SWITCHES 15


int redrawScreen = 1;

static char 
switch_update_interrupt_sense()
{
  char p2val = P2IN;
  /* update switch interrupt to detect changes from current buttons */
  P2IES |= (p2val & SWITCHES);	/* if switch up, sense down */
  P2IES &= (p2val | ~SWITCHES);	/* if switch down, sense up */
  return p2val;
}

void 
switch_init()			/* setup switch */
{  
  P2REN |= SWITCHES;		/* enables resistors for switches */
  P2IE |= SWITCHES;		/* enable interrupts from switches */
  P2OUT |= SWITCHES;		/* pull-ups for switches */
  P2DIR &= ~SWITCHES;		/* set switches' bits for input */
  switch_update_interrupt_sense();
}

int switches = 0;

void
switch_interrupt_handler()
{
  char p2val = switch_update_interrupt_sense();
  switches = ~p2val & SWITCHES;
  if(switches & SWITCHES){
    redrawScreen = 1;
    for(char swNum = 0; swNum < 4; swNum++){
      int swFlag = 1 << swNum;
      if(switches & swFlag){
	current_position = swNum;
	break;
      }
    }
  }
}
void wdt_c_handler()
{
  static int sec2Count = 0;
  static int sec1Count = 0;
  if(sec2Count++ >= 125){
    sec2Count = 0;
    current_color = (current_color+1) % NUM_SQCOLORS;
    redrawScreeen = 1;
    buzzer_set_period(400);
  }
  if (sec1Count >= 250) {	     
    sec1Count = 0;
    current_position = (current_position+1) % NUM_POSITIONS;
    redrawScreen = 1;
    buzzer_set_period(1000);
  }
}
  
void update_shape();

void main()
{
  
  P1DIR |= LED;		/**< Green led on when CPU on */
  P1OUT |= LED;
  configureClocks();
  lcd_init();
  switch_init();
  
  enableWDTInterrupts();      /**< enable periodic interrupt */
  or_sr(0x8);	              /**< GIE (enable interrupts) */
  
  clearScreen(COLOR_BLUE);
  while (1) {			/* forever */
    if (redrawScreen) {
      redrawScreen = 0;
      update_shape();
    }
    P1OUT &= ~LED;	/* led off */
    or_sr(0x10);	/**< CPU OFF */
    P1OUT |= LED;	/* led on */
  }
}

    
    
void
update_shape()
{
  static char row = last_position = 0, last_color = 0;
  redrawScreen = 0;
  int pos, color;
  and_sr(~8);
  pos = current_position;
  color = current_color;
  or_sr(8);
  if (pos== last_position && color == last_color) {
    return;
  }
  short col = position[last_position].col;
  short row = position[last_position].row;
  if(pos != last_[position){
      fillRectangle(col-5, row-5,10,10, BG_COLOR);
  }
    col = position[pos].col;
    row = position[pos].row;
    fillRectangle(col-5, row-5, 10,10, sqColors[color]);
    last_positioon = pos;
    last_color = color;
}


/* Switch on S2 */
void
__interrupt_vec(PORT2_VECTOR) Port_2(){
  if (P2IFG & SWITCHES) {	      /* did a button cause this interrupt? */
    P2IFG &= ~SWITCHES;		      /* clear pending sw interrupts */
    switch_interrupt_handler();	/* single handler for all switches */
  }
}
