/**
 * Stacker Game 
 * By Electro707
 * 
 * This program uses my Simple LED Matrix Library to create a stacker game (like the one found in malls and arcades)
 * 
 */
#include "simple_matrix.h"  //Import the library

simpleMatrix disp(10, 2);
#define BUTTON1 2
#define BUTTON2 3

uint8_t current_y;
int8_t current_block_width;
uint8_t last_y_level;
bool first_block;
unsigned long button_last_press;

struct block_struct{
  uint8_t block_width;
  uint8_t current_x;
  uint8_t y_pos;
  bool move_dir;
};

block_struct current_block;

uint8_t delay_time;

void setup(){
  disp.begin();
  disp.setIntensity(0x02);
  disp.verticalDisplays();
  disp.clearDisplay();
  SPI.setClockDivider(SPI_CLOCK_DIV2);

  pinMode(BUTTON1, INPUT_PULLUP);
  pinMode(BUTTON2, INPUT_PULLUP);
  
  button_last_press = 0;
  last_y_level = 0;
  first_block = 1;

  entry_animation();
  current_y = 0;
  current_block_width = 4;
  init_block(&current_block, current_y, current_block_width);
  delay_time = new_delay_time();

  // Attach an interrupt to the buttom
  EIFR |= (1 << 0);
  attachInterrupt(digitalPinToInterrupt(BUTTON1), press_button, FALLING);
}


void loop(){
  // Main loop, which just includes the game logic (which is to move the block), and a delay
  delay(delay_time);

  noInterrupts();
  move_block(&current_block);
  interrupts();
}

/**
 * Function that shows an animation when the Arduino boots
 */
void entry_animation(){
  for(int i=0;i<16;i++){
    disp.setRowPixel(0, 7, i);
    _delay_ms(50);
  }
  _delay_ms(1000);
  disp.clearDisplay();
}

/**
 * Function that creates an animation when the game is over
 */
void game_over(){
  _delay_ms(500);
  // After delay, clear every row sequentially below the one which made you lose
  for(int i=current_y-1; i>=0;i--){
    disp.clearRowPixel(0, 7, i);
    _delay_ms(100);
  }
  _delay_ms(1000);
  disp.clearDisplay();
  reset_game();
}

/**
 * Function that creates an animation when you win the game (CONGRATS)
 */
void win_game(){
  _delay_ms(500);
  // Sequentially fill LEDs row by row from bottom to top
  for(int i=0;i<16;i++){
    disp.setRowPixel(0, 7, i);
    _delay_ms(50);
  }
  // Sequentially clear LEDs row by row from bottom to top
  for(int i=0;i<16;i++){
    disp.clearRowPixel(0, 7, i);
    _delay_ms(50);
  }
  reset_game();
}

/**
 * Function that gets called when the game is reset, so both after winning and losing
 */
void reset_game(){
  current_y = 0;
  first_block = 1;
  current_block_width = 4;
  init_block(&current_block, current_y, current_block_width);
  delay_time = new_delay_time();
  EIFR |= (1 << 0);
}

/**
 * Function that gets called when a buttom is pressed
 */
void press_button(){
  if((millis() - button_last_press) < 500){
    return;
  }
  button_last_press = millis();

  place_block(&current_block);
  EIFR |= (1 << 0);
}

/**
 * 
 * 
 */
void move_block(block_struct *b){
  if(b->move_dir && b->current_x == 0){
      b->move_dir = false;
  }
  else if(b->current_x + b->block_width == 8){
      b->move_dir = true;
  }

  if(b->move_dir){
    b->current_x--;
    disp.setPixel(b->current_x, b->y_pos);
    disp.clearPixel(b->current_x+b->block_width, b->y_pos);
  } else {
    b->current_x++;
    disp.clearPixel(b->current_x-1, b->y_pos);
    disp.setPixel(b->current_x+b->block_width-1, b->y_pos);
  }
}

void init_block(block_struct *b, int y, int width){
  b->y_pos = y;
  b->block_width = width;
  b->move_dir = 0;
  b->current_x = 0;
  disp.setRowPixel(0, b->block_width-1, b->y_pos);
}

uint8_t new_delay_time(){
  // For the first 6 rows, go easy on the player
  if(current_y <= 6){
    return 125;
  }
  // if(current_y > 3 && current_y <= 8){
  //   return 75;
  // }
  return (16-current_y)*10;
}

void place_block(block_struct *curr_b){
  uint8_t new_y_level = 0;
  for(int i=curr_b->current_x; i<(curr_b->current_x+curr_b->block_width); i++){
    if(first_block == 0){
      if((last_y_level & (1 << i)) == 0){
        current_block_width--;
      }
      else{
        new_y_level |= (1 << i);
      }
    }
    else{
       new_y_level |= (1 << i);
    }
  }

  if(current_block_width <= 0){
    game_over();
    return;
  }
  else if(current_y == 15){
    win_game();
    return;
  }

  if(first_block == 0){
    for(int i=curr_b->current_x; i<=(curr_b->current_x+curr_b->block_width); i++){
      if((last_y_level & (1 << i)) == 0){
        disp.clearPixel(i, curr_b->y_pos);
      }
    }
  }

  last_y_level = new_y_level;
  first_block = 0;

  delay_time = new_delay_time();
  current_y++;
  init_block(curr_b, current_y, current_block_width);
}
