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

// Variables used for Rotation
int lockstateA = 1;  // starts as opened
int lockstateB = 1;  // starts as opened

// Variables Used Listed Here
char masterKey[4] ;// masterKey
char sideKeyA[4]  ;// sideKeyA
char sideKeyB[4]  ;// sideKeyB
char baitKey[4]   ;// baitKey
char tempPass[4];  // temp password taken from user

char state = '0';  // states

int successful   = 0;  // total successful attempts
int unsuccessful = 0;  // total unsuccessful attempts

int SFA = 0;  //  Increase Successive Failure Attempts
boolean toggle1 = 0;
int secCount = 0;
int sfaCheck = 0;

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
  
  
  // checking whether initial values written or not.
  char checkInit = EEPROM.read(0);
  if(checkInit != '1'){
    for(int j=0;j<20;j++)
    EEPROM.write(j, '0');
  }
  
  // if not written, write the default values
  if(checkInit == '0'){
    checkInit = '1';
    EEPROM.write(0, checkInit);
    singleExec();
  }
  
  // initializing variables here
  EEPROM.get( 1 , masterKey );
  EEPROM.get( 5 , sideKeyA );
  EEPROM.get( 9 , sideKeyB );
  EEPROM.get( 13 , baitKey );
  
  sei();
}

ISR(TIMER1_COMPA_vect){
  if (secCount > 14){
    digitalWrite(0, HIGH);
    SFA = 0;
    secCount = 0;
  }
  else if (SFA != sfaCheck){
    digitalWrite(1, LOW);
    sfaCheck = SFA;
    digitalWrite(0, LOW);
    secCount = 0;
  }else{
    digitalWrite(1, HIGH);
    secCount++;
  }
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
