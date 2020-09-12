#include <EEPROM.h>
#include <LiquidCrystal.h>
#include <Keypad.h>

// Configuration of Keypad rows and columns
const byte ROWS = 4; //four rows
const byte COLS = 3; //three columns
char keys[ROWS][COLS] = {
    {'1','2','3'},
    {'4','5','6'},
    {'7','8','9'},
    {'*','0','#'}
};
byte rowPins[ROWS] = {10, 9, 8, 7}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {13, 12, 11}; //connect to the column pinouts of the keypad

// LCD and Keypad cofigured with interface pins
LiquidCrystal lcd(A0, A1, A2, A3, A4, A5);
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );


void setup() {
  cli();
  // set up the LCD's number of columns and rows:
  lcd.begin(20, 4);
  lcd.setCursor(4,0);
  lcd.print("Main Menu");
  lcd.setCursor(0,1);
  
  // setup for rotation of motors
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  
  // setup for blink
  pinMode(2, OUTPUT);
  
  //set timer1 interrupt at 1Hz
  TCCR1A = 0;// set entire TCCR1A register to 0
  TCCR1B = 0;// same for TCCR1B
  TCNT1  = 0;//initialize counter value to 0
  // set compare match register for 1hz increments
  OCR1A = 15624;// = (16*10^6) / (15*1024) - 1 (must be <65536)
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS10 and CS12 bits for 1024 prescaler
  TCCR1B |= (1 << CS12) | (1 << CS10);  
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);
  
  sei();
}


void singleExec(){
    char k1[4] = {'1','2','3','4'};  // default masterKey
    char k2[4] = {'2','2','2','2'};  // default sideKeyA
    char k3[4] = {'3','3','3','3'};  // default sideKeyB
    char k4[4] = {'9','9','9','9'};  // default baitKey

    EEPROM.put(1, k1);
    EEPROM.put(5, k2);
    EEPROM.put(9, k3);
    EEPROM.put(13, k4);
}

void loop() {

}
