
#include <Keypad_I2C.h> // I2C Keypad library by Joe Young https://github.com/joeyoung/arduino_keypads // I2C LCD Library by Francisco Malpartida https://bitbucket.org/fmalpartida/new-liquidcrystal/wiki/Home
#include <LiquidCrystal_I2C.h>

//I2C pins declaration
LiquidCrystal_I2C i2c_lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE); 

#define lcd_addr 0x27     // I2C address of typical I2C LCD Backpack
#define keypad_addr 0x20  // I2C address of I2C Expander module (A0-A1-A2 dip switch to off position)

// LCD Pins to I2C LCD Backpack - These are default for HD44780 LCD's
#define Rs_pin 0
#define Rw_pin 1
#define En_pin 2
#define BACKLIGHT_PIN 3
#define D4_pin 4
#define D5_pin 5
#define D6_pin 6
#define D7_pin 7

int redButtonPin = 13;
int greenButtonPin = 12;
int buzzerPin = 10;

int code[4] = {0, 0, 0, 0};
int code_input[4] = {-1, -1, -1, -1};


struct RGBLed {
  int redPin;
  int greenPin;
};

RGBLed led1 = {9, 8};
RGBLed led2 = {7, 6};
RGBLed led3 = {5, 4};
RGBLed led4 = {3, 2};

// Define the keypad pins
const byte ROWS = 4; 
const byte COLS = 3;
char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};

// Keypad pins connected to the I2C-Expander pins P0-P6
byte rowPins[ROWS] = {0, 1, 2, 3}; // connect to the row pinouts of the keypad
byte colPins[COLS] = {4, 5, 6};    // connect to the column pinouts of the keypad

// Create instance of the Keypad name I2C_Keypad and using the PCF8574 chip
Keypad_I2C I2C_Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS, keypad_addr, PCF8574 );

void beep(int ms, uint8_t freq) {
  analogWrite(buzzerPin, freq);
  delay(ms);
  analogWrite(buzzerPin, 255);
}

void setup() {

  pinMode(buzzerPin, OUTPUT);
  analogWrite(buzzerPin, 255);

  pinMode(led1.redPin, OUTPUT);
  pinMode(led1.greenPin, OUTPUT);
  // pinMode(bluePin, OUTPUT);
  pinMode(led2.redPin, OUTPUT);
  pinMode(led2.greenPin, OUTPUT);
  
  pinMode(led3.redPin, OUTPUT);
  pinMode(led3.greenPin, OUTPUT);
  
  pinMode(led4.redPin, OUTPUT);
  pinMode(led4.greenPin, OUTPUT);

  // Define pin #12 as input and activate the internal pull-up resistor
  pinMode(redButtonPin, INPUT_PULLUP);
  pinMode(greenButtonPin, INPUT_PULLUP);

  i2c_lcd.begin (16,2); //  our LCD is a 16x2, change for your LCD if needed 
  
  // LCD Backlight ON
  i2c_lcd.setBacklightPin(BACKLIGHT_PIN,POSITIVE);
  i2c_lcd.setBacklight(HIGH); 
  i2c_lcd.clear(); // Clear the LCD screen
                
  I2C_Keypad.begin();
  Serial.begin(9600);

  randomSeed(analogRead(0));
  code[0] = random(9);
  code[1] = random(9);
  code[2] = random(9);
  code[3] = random(9);

}

enum states {
  MENU, 
  CHOOSE_DIFF,
  GAME,
  EASY_CHECK_CODE,
  HARD_CHECK_CODE,
  FINAL,
  CHOOSE_GIVE_UP,
  FINAL_GIVE_UP
};

enum states state = MENU;
bool isEasy = true;

int cursor = 0;
int pos_code[4] = {5, 7, 9, 11};
unsigned long counter = 0;

bool selectNo = true;

bool get_button_input(int buttonPin) {
  for (int i = 0; i < 8; i++) {
    int buttonValue = digitalRead(buttonPin);
    
    if (buttonValue) {
      return false;
    }
  }

  return true;
}

bool pressed_one_time(int buttonPin) {
  bool button_input = get_button_input(buttonPin);

  if (!button_input) {
    return false;
  }

  while (get_button_input(buttonPin));

  return true;
}

void menu() {

  i2c_lcd.setCursor(0, 0);
  i2c_lcd.print("Crack the code");

  i2c_lcd.setCursor(0, 1);
  i2c_lcd.print("Start playing?");
  
  if (pressed_one_time(greenButtonPin)) {
    i2c_lcd.clear();

    state = CHOOSE_DIFF;
    isEasy = true;

  }

}

void choose_diff() {
  code[0] = random(9);
  code[1] = random(9);
  code[2] = random(9);
  code[3] = random(9);

  i2c_lcd.setCursor(0, 0);
  i2c_lcd.print("Difficulty?");

  if (isEasy) {
    i2c_lcd.setCursor(0, 1);
    i2c_lcd.print("->");
  } else {
    i2c_lcd.setCursor(9, 1);
    i2c_lcd.print("->");
  }

  i2c_lcd.setCursor(2, 1);
  i2c_lcd.print("Easy");

  i2c_lcd.setCursor(11, 1);
  i2c_lcd.print("Hard");

  char key = I2C_Keypad.getKey();
  if (key == '*') {
    isEasy = true;
    i2c_lcd.clear();
  } else if (key == '#') {
    isEasy = false;
    i2c_lcd.clear();
  }


  if (pressed_one_time(greenButtonPin)) {
    i2c_lcd.clear();

    state = GAME;

    return;
  }


  if (pressed_one_time(redButtonPin)) {
    i2c_lcd.clear();

    state = MENU;

    return;
  }

}

int correct_code[4] ={-1, -1, -1, -1};

void game() {
  i2c_lcd.setCursor(0, 0);
  i2c_lcd.print("Enter your code:");
  for (int i = 0; i < 4; i++) {
    i2c_lcd.setCursor(pos_code[i], 1);
  
    if (code_input[i] == -1) {
      i2c_lcd.print(" ");
    } else {
      i2c_lcd.print(String(code_input[i]));
    }
  }


  // blink
  if (code_input[cursor] == -1) {
    if (counter < 30) {
      i2c_lcd.setCursor(pos_code[cursor], 1);
      i2c_lcd.print("_");
    } else {
      i2c_lcd.setCursor(pos_code[cursor], 1);
      i2c_lcd.print(" ");
    }
  } else {
    if (counter < 20) {
      i2c_lcd.setCursor(pos_code[cursor], 1);
      i2c_lcd.print(" ");
    } else {
      i2c_lcd.setCursor(pos_code[cursor], 1);
      i2c_lcd.print(String(code_input[cursor]));
    }
  }

  // MOVE CURSOR
  char key = I2C_Keypad.getKey();

  if (key == '0' || key == '1' || key == '2' || key == '3' || key == '4' || key == '5' || key == '6' || key == '7' || key == '8' || key == '9') {
    beep(100, 200);
  }

  if (key == '*' || key == '#') {
    beep(50, 220);
  }

  switch (key) {
    case '*':
      i2c_lcd.setCursor(pos_code[cursor], 1);
      
      if (code_input[cursor] == -1) {
        i2c_lcd.print(" ");
      } else {
        i2c_lcd.print(String(code_input[cursor]));
      }
      
      counter = 0;
      cursor--;

      if (cursor < 0) {
        cursor = 3;
      }

    break;
    
    case '#':
      i2c_lcd.setCursor(pos_code[cursor], 1);
      
      if (code_input[cursor] == -1) {
        i2c_lcd.print(" ");
      } else {
        i2c_lcd.print(String(code_input[cursor]));
      }
      
      counter = 0;
      cursor++;
    break;
    
    case '1':
      code_input[cursor] = 1;
      cursor++;
    break;

    case '2':
      code_input[cursor] = 2;
      cursor++;
    break;

    case '3':
      code_input[cursor] = 3;
      cursor++;
    break;

    case '4':
      code_input[cursor] = 4;
      cursor++;
    break;

    case '5':
      code_input[cursor] = 5;
      cursor++;
    break;

    case '6':
      code_input[cursor] = 6;
      cursor++;
    break;

    case '7':
      code_input[cursor] = 7;
      cursor++;
    break;

    case '8':
      code_input[cursor] = 8;
      cursor++;
    break;

    case '9':
      code_input[cursor] = 9;
      cursor++;
    break;

    case '0':
      code_input[cursor] = 0;
      cursor++;
    break;
  }

  cursor = cursor % 4;

  // CHECK
  if (pressed_one_time(greenButtonPin)) { 
    for (int i = 0; i < 4; i++) {
      if (code_input[i] == -1) {
        beep(100, 0);
        beep(200, 50);
        return;
      }
    }

    for (int i = 0; i < 4; i++) {
      if (code_input[i] == code[i]) {
        correct_code[i] = 1;
      } else {
        correct_code[i] = -1;
      }
    }

    if (isEasy) {
      state = EASY_CHECK_CODE;
    } else {
      state = HARD_CHECK_CODE;
    }
    i2c_lcd.clear();

  }

  if (pressed_one_time(redButtonPin)) {
    state = CHOOSE_GIVE_UP;
    selectNo = true;
    i2c_lcd.clear();
  }
}

void easy_check_game() {
  int corr = 0;

  for (int i = 0; i < 4; i++) {
    if (correct_code[i] == 1) {
      corr++;
    }
  }

  i2c_lcd.setCursor(0, 0);
  i2c_lcd.print(String(corr));
  i2c_lcd.setCursor(2, 0);
  i2c_lcd.print("correct");

  for (int i = 0; i < 4; i++) {
    i2c_lcd.setCursor(pos_code[i], 1);
    
    if (correct_code[i] == 1) {
      i2c_lcd.print(String(code_input[i]));
    } else {
      i2c_lcd.print("X");
    }
  }

  setColor(255, 0, 0, led1);
  setColor(255, 0, 0, led2);
  setColor(255, 0, 0, led3);
  setColor(255, 0, 0, led4);
  
  if (corr >= 1) {
    setColor(0, 255, 0, led1);
  }
  if (corr >= 2) {
    setColor(0, 255, 0, led2);
  }
  if (corr >= 3) {
    setColor(0, 255, 0, led3);
  }
  if (corr == 4) {
    setColor(0, 255, 0, led4);
  }

  if (pressed_one_time(greenButtonPin)) {
    if (corr == 4) {
      state = FINAL;
      i2c_lcd.clear();
      return;
    }

    for (int i = 0; i < 4; i++) {
      code_input[i] = -1;
    }

    cursor = 0;
    state = GAME;
    i2c_lcd.clear();
  }

  if (pressed_one_time(redButtonPin)) {
    state = CHOOSE_GIVE_UP;
    selectNo = true;
    i2c_lcd.clear();
  }

}

void hard_check_game() {
  int corr = 0;

  for (int i = 0; i < 4; i++) {
    if (correct_code[i] == 1) {
      corr++;
    }
  }

  i2c_lcd.setCursor(0, 0);
  i2c_lcd.print(String(corr));
  i2c_lcd.setCursor(2, 0);
  i2c_lcd.print("correct");

  
  if (pressed_one_time(greenButtonPin)) {
    if (corr == 4) {
      state = FINAL;
      i2c_lcd.clear();
      return;
    }

    for (int i = 0; i < 4; i++) {
      code_input[i] = -1;
    }

    cursor = 0;
    state = GAME;
    i2c_lcd.clear();
  }

  setColor(255, 0, 0, led1);
  setColor(255, 0, 0, led2);
  setColor(255, 0, 0, led3);
  setColor(255, 0, 0, led4);

  if (corr >= 1) {
    setColor(0, 255, 0, led1);
  }
  if (corr >= 2) {
    setColor(0, 255, 0, led2);
  }
  if (corr >= 3) {
    setColor(0, 255, 0, led3);
  }
  if (corr == 4) {
    setColor(0, 255, 0, led4);
  }

  if (pressed_one_time(redButtonPin)) {
    state = CHOOSE_GIVE_UP;
    selectNo = true;
    i2c_lcd.clear();
  }

}

void final() {

  i2c_lcd.setCursor(0, 0);
  i2c_lcd.print("Congratulations!");

  i2c_lcd.setCursor(0, 1);
  i2c_lcd.print("Try again?");

  if (pressed_one_time(greenButtonPin)) {
    for (int i = 0; i < 4; i++) {
      code_input[i] = -1;
    }

    cursor = 0;
    state = MENU;

    i2c_lcd.clear();
  }

  setColor(0, 0, 0, led1);
  setColor(0, 0, 0, led2);
  setColor(0, 0, 0, led3);
  setColor(0, 0, 0, led4);
  
}


void choose_give_up() {

  i2c_lcd.setCursor(0, 0);
  i2c_lcd.print("Give up?");

  i2c_lcd.setCursor(3, 1);
  i2c_lcd.print("No");

  i2c_lcd.setCursor(10, 1);
  i2c_lcd.print("Yes");


  char key = I2C_Keypad.getKey();
  if (key == '*' || key == '#') {
    selectNo = !selectNo;
  }

  if (selectNo) {
    i2c_lcd.setCursor(8, 1);
    i2c_lcd.print("  ");
    i2c_lcd.setCursor(1, 1);
  } else {
    i2c_lcd.setCursor(1, 1);
    i2c_lcd.print("  ");
    i2c_lcd.setCursor(8, 1);
  }

  i2c_lcd.print("->");

  if (pressed_one_time(greenButtonPin)) {
    if (selectNo) {
      state = GAME;
      i2c_lcd.clear();
    } else {
      for (int i = 0; i < 4; i++) {
        code_input[i] = -1;
      }

      setColor(0, 0, 0, led1);
      setColor(0, 0, 0, led2);
      setColor(0, 0, 0, led3);
      setColor(0, 0, 0, led4);
      state = FINAL_GIVE_UP;
      i2c_lcd.clear();
    }
  }

}

void final_give_up() {
  i2c_lcd.setCursor(0, 0);
  i2c_lcd.print("Code was:");

  for (int i = 0; i < 4; i++) {
    i2c_lcd.setCursor(10 + i, 0);
    i2c_lcd.print(String(code[i]));
  }

  i2c_lcd.setCursor(3, 1);
  if (counter < 30) {
    i2c_lcd.print("Play again");
  } else {
    i2c_lcd.print("          ");
  }

  if (pressed_one_time(greenButtonPin)) {
    state = CHOOSE_DIFF;
    isEasy = true;
    i2c_lcd.clear();
  }
}

void loop() {
  counter++;
  counter %= 50;
  Serial.print(String(code[0]));
  Serial.print(String(code[1]));
  Serial.print(String(code[2]));
  Serial.println(String(code[3]));

  switch (state) {
    case MENU:
      menu();
      break;
    case CHOOSE_DIFF:
      choose_diff();
      break;
    case GAME:
      game();
      break;
    case EASY_CHECK_CODE:
      easy_check_game();
      break;
    case HARD_CHECK_CODE:
      hard_check_game();
      break;
    case FINAL:
      final();
      break;
    case FINAL_GIVE_UP:
      final_give_up();
      break;
    case CHOOSE_GIVE_UP:
      choose_give_up();
      break;
  }
}

void setColor(int redValue, int greenValue, int blueValue, RGBLed led) {
  analogWrite(led.redPin, redValue);
  analogWrite(led.greenPin, greenValue);
}