#include "simple_matrix.h"  //Import the library

simpleMatrix disp(10, true, 2, true);
#define BUTTON1 2
#define BUTTON2 3

uint8_t current_y;
int8_t current_block_width;
uint8_t last_y_level;
bool first_block;
bool press_bt1;

struct block_struct{
  uint8_t block_width;
  uint8_t current_x;
  uint8_t y_pos;
  bool move_dir;
};

block_struct current_block;

uint8_t delay_time;

void setup(){
  Serial.begin(9600);
  disp.begin();
  disp.setIntensity(0x02);
  disp.clearDisplay();

  pinMode(BUTTON1, INPUT_PULLUP);
  pinMode(BUTTON2, INPUT_PULLUP);
  delay(100);
  
  press_bt1 = 0;
  last_y_level = 0;
  first_block = 1;

//  entry_animation();
  current_y = 0;
  current_block_width = 4;
  init_block(&current_block, current_y, current_block_width);
  delay_time = new_delay_time();

  EIFR |= (1 << 0);
  attachInterrupt(digitalPinToInterrupt(BUTTON1), press_button, CHANGE);
}


void loop(){
  delay(delay_time);
  move_block(&current_block);
}

void entry_animation(){
  for(int i=0;i<2;i++){
    disp.fillDisplay();
    _delay_ms(500);
    disp.clearDisplay();
    _delay_ms(500);
  }
}

void game_over(){
  for(int i=0;i<2;i++){
    disp.fillDisplay();
    _delay_ms(500);
    disp.clearDisplay();
    _delay_ms(500);
  }
  reset_game();
}

void reset_game(){
  current_y = 0;
  first_block = 1;
  current_block_width = 4;
  init_block(&current_block, current_y, current_block_width);
  delay_time = new_delay_time();
  EIFR |= (1 << 0);
}

void press_button(){
  _delay_ms(25);
  if(digitalRead(BUTTON1) == 0 && press_bt1 == 0){
    press_bt1 = 1;
    place_block(&current_block);
  }
  else if(digitalRead(BUTTON1) == 1){
    press_bt1 = 0;
  }
  EIFR |= (1 << 0);
}

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
  for(int i=0;i<b->block_width;i++){
    disp.setPixel(i, b->y_pos);
  }
}

uint8_t new_delay_time(){
  if(current_y >= 0 && current_y <= 3){
    return 100;
  }
  if(current_y > 3 && current_y <= 8){
    return 75;
  }
  return (16-current_y)*10;
}

void place_block(block_struct *curr_b){
  uint8_t new_y_level = 0;
  for(int i=curr_b->current_x; i<(curr_b->current_x+curr_b->block_width); i++){
    if(first_block == 0){
      if((last_y_level & (1 << i)) == 0){
        current_block_width--;
        disp.clearPixel(i, curr_b->y_pos);
      }
      else{
        new_y_level |= (1 << i);
      }
    }
    else{
       new_y_level |= (1 << i);
    }
  }
  last_y_level = new_y_level;
  first_block = 0;
  

  if(current_block_width <= 0){
    game_over();
    return;
  }

  delay_time = new_delay_time();
  current_y++;
  init_block(curr_b, current_y, current_block_width);
}
