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
  char testChar;
  EEPROM.get(21, testChar);
  if(testChar != 1){
    lcd.setCursor(4,3);
    lcd.print("AA");
  }
  
  int j = 0;
  if(state == '0'){
    lcd.clear();
    lcd.setCursor(4,0);
    lcd.print("Main Menu");
    while(1){
      char key = keypad.getKey();
      if (key == '1' || key == '2' || key == '3' || key == '4') {
        state = key;
        key=0;
        delay(100);
        break;
      }
    }
  }
  
  if(state == '1')
  {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Open/Lock Section A");
    lcd.setCursor(0,1);
    while(j<4){
      char key = keypad.getKey();
      if(key){
        tempPass[j++]=key;
        lcd.print('*');
      }
      key=0;
    }
    delay(1000);
    if(!(strncmp(tempPass, masterKey, 4)))  // master key ile check ediyor
    {
      successful++;
      SFA = 0;
      lcd.clear();
      lcd.setCursor(0,0);
        lcd.print("Section A - Master");
      lcd.setCursor(0,1);
        lcd.print("Opening/Locking");
      rotateMotor(0);  // call open/lock func
      state = '0';
      delay(1000);
      lcd.clear();
    }
    else if (!(strncmp(tempPass, sideKeyA, 4)))  // side key A
    {
      successful++;
      SFA = 0;
      lcd.clear();
      lcd.setCursor(0,0);
        lcd.print("Section A - Standard");
      lcd.setCursor(0,1);
        lcd.print("Opening/Locking");
      rotateMotor(0);  // call open/lock func
      state = '0';
      delay(1000);
      lcd.clear();
    }
    else if (!(strncmp(tempPass, baitKey, 4)))  // bait key
    {
      successful++;
      SFA = 0;
      lcd.clear();
      lcd.setCursor(0,0);
        lcd.print("Section A - Standard");
      lcd.setCursor(0,1);
        lcd.print("Opening/Locking");
      rotateMotor(0);  // call open/lock func
      // blink the led
      digitalWrite(2, HIGH);delay(100);
      digitalWrite(2, LOW);delay(100);
      digitalWrite(2, HIGH);delay(100);
      digitalWrite(2, LOW);delay(100);
      digitalWrite(2, HIGH);
      state = '0';
      delay(1000);
      lcd.clear();
    }
    else  // Wrong Input
    {
      unsuccessful++;
      SFA++;
      lcd.clear();
      lcd.setCursor(0,0);
        lcd.print("Wrong Password");
      delay(1000);
      lcd.clear();
      if(!(SFA < 3)){
        state = '5';
      }
    }
  }
  else if(state == '2')
  {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Open/Lock Section B");
    lcd.setCursor(0,1);
    while(j<4){
      char key = keypad.getKey();
      if(key){
        tempPass[j++]=key;
        lcd.print('*');
      }
      key=0;
    }
    delay(1000);
    if(!(strncmp(tempPass, masterKey, 4)))  // master key ile check ediyor
    {
      successful++;
      SFA = 0;
      lcd.clear();
      lcd.setCursor(0,0);
        lcd.print("Section B - Master");
      lcd.setCursor(0,1);
        lcd.print("Opening/Locking");
      rotateMotor(1);  // call open/lock func
      state = '0';
      delay(1000);
      lcd.clear();
    }
    else if (!(strncmp(tempPass, sideKeyB, 4)))  // side key B
    {
      successful++;
      SFA = 0;
      lcd.clear();
      lcd.setCursor(0,0);
        lcd.print("Section B - Standard");
      lcd.setCursor(0,1);
        lcd.print("Opening/Locking");
      rotateMotor(1);  // call open/lock func
      state = '0';
      delay(1000);
      lcd.clear();
    }
    else if (!(strncmp(tempPass, baitKey, 4)))  // bait key
    {
      successful++;
      SFA = 0;
      lcd.clear();
      lcd.setCursor(0,0);
        lcd.print("Section B - Standard");
      lcd.setCursor(0,1);
        lcd.print("Opening/Locking");
      rotateMotor(1);  // call open/lock func
      // blink the led
      digitalWrite(2, HIGH);delay(100);
      digitalWrite(2, LOW);delay(100);
      digitalWrite(2, HIGH);delay(100);
      digitalWrite(2, LOW);delay(100);
      digitalWrite(2, HIGH);
      state = '0';
      delay(1000);
      lcd.clear();
    }
    else  // Wrong Input
    {
      unsuccessful++;
      SFA++;
      lcd.clear();
      lcd.setCursor(0,0);
        lcd.print("Wrong Password");
      delay(1000);
      lcd.clear();
      if(!(SFA < 3)){
        state = '5';
      }
    }
  }
  else if(state == '3')  // Change Password
  {
    int j=0;
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Master Pass. Needed");
    lcd.setCursor(0,1);
    lcd.print("To Change Pass.");
    lcd.setCursor(0,2);
    while(j<4){
      char key = keypad.getKey();
      if(key){
        tempPass[j++]=key;
        lcd.print('*');
      }
      key=0;
    }
    delay(1000);
    if(strncmp(tempPass, masterKey, 4))  // master key değilse
    {
      unsuccessful++;
      lcd.clear();
      lcd.print("Wrong Password");
      delay(1000);
      lcd.clear();
      SFA++;
      if(!(SFA < 3)){
        state = '5';
      }
    }
    else
    {
      successful++;
      lcd.clear();
      lcd.setCursor(0,0);
        lcd.print("Change Password");
      lcd.setCursor(0,1);
        lcd.print("1M 2A 3B 4T");
      lcd.setCursor(0,2);
      while(1)
      {
        char key = keypad.getKey();
        if(key){
          changeKey(key);
          break;
        }
      }
      state = '0';
    }
  }
  else if(state == '4')  // Report Attempts  
  {
    lcd.clear();
    lcd.setCursor(0,0);
    String str = "Suc. Attempts: " + String(successful) + ",";
    lcd.print(str);
    lcd.setCursor(0,1);
    str = "UnSuc. Attempts: " + String(unsuccessful);
    lcd.print(str);
    lcd.setCursor(0,2);
    str = "SFA: " + String(SFA);
    lcd.print(str);
    state = '0';
    delay(5000);
    lcd.clear();
  }
  else if (state == '5')  // Master Lock Screen
  {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("System Locked MP");
    lcd.setCursor(0,1);
    int j = 0;
    while(j<4){
      char key = keypad.getKey();
      if(key){
        tempPass[j++]=key;
        lcd.print('*');
      }
      key=0;
    }
    delay(1000);
    if(!(strncmp(tempPass, masterKey, 4)))  // GOTO Main Screen
    {
      successful++;
      SFA = 0;
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Lock is UnLocked!");
      state = '0';
      delay(1000);
      lcd.clear();
    }
    else  // GOTO Time Lock Screen
    {
      unsuccessful++;
      state = '6';
      lcd.clear();
    }
  }
  else if (state == '6')  // Time Lock Screen
  {
    for(int i = 0; i < 20; i++){
      lcd.clear();
      lcd.setCursor(0,0);
      String str = "No access " + String(i+1) + " sec";
      lcd.print(str);
      delay(1000);
    }
    state = '5';
  }
  else{
    state = '0';
  }
  
}

void changeKey(char st)
{
  if(st == '1') //Change Master Password
  {
    int j = 0;
    lcd.clear();
    lcd.print("Enter New Pass M");
    lcd.setCursor(0,1);
    while(j<4){
      char key = keypad.getKey();
      if(key){
        masterKey[j++]=key;
        lcd.print('*');
      }
      key=0;
    }
    EEPROM.put(1, masterKey);
  }
  else if (st == '2') // Change Section A Password
  {
    int j = 0;
    lcd.clear();
    lcd.print("Enter New Pass A");
    lcd.setCursor(0,1);
    while(j<4){
      char key = keypad.getKey();
      if(key){
        sideKeyA[j++]=key;
        lcd.print('*');
      }
      key=0;
    }
    EEPROM.put(5, sideKeyA);
  }
  else if (st == '3') // Change Section B Password
  {
    int j = 0;
    lcd.clear();
    lcd.print("Enter New Pass B");
    lcd.setCursor(0,1);
    while(j<4){
      char key = keypad.getKey();
      if(key){
        sideKeyB[j++]=key;
        lcd.print('*');
      }
      key=0;
    }
    EEPROM.put(9, sideKeyB);
  }
  else if (st == '4') // Change Bait Password
  {
    int j = 0;
    lcd.clear();
    lcd.print("Enter New Pass T");
    lcd.setCursor(0,1);
    while(j<4){
      char key = keypad.getKey();
      if(key){
        baitKey[j++]=key;
        lcd.print('*');
      }
      key=0;
    }
    EEPROM.put(13, baitKey);
  }
  delay(1000);
  lcd.clear();
  lcd.print("Pass is changed");
  delay(1000);
  lcd.clear();
}

void rotateMotor(boolean type)
{
  if(type){
    if(lockstateB == 0){  // locked -> opened
      lockstateB = 1;
      //rotate twice left
      delay(1000);
      digitalWrite(4, HIGH);digitalWrite(3, HIGH);
      delay(1000);
      digitalWrite(4, LOW);digitalWrite(3, HIGH);
      delay(1000);
      digitalWrite(4, HIGH);digitalWrite(3, LOW);
      delay(1000);
      digitalWrite(4, LOW);digitalWrite(3, LOW);
      delay(1000);
      digitalWrite(4, HIGH);digitalWrite(3, HIGH);
      delay(1000);
      digitalWrite(4, LOW);digitalWrite(3, HIGH);
      delay(1000);
      digitalWrite(4, HIGH);digitalWrite(3, LOW);
      delay(1000);
      digitalWrite(4, LOW);digitalWrite(3, LOW);
      delay(1000);
    }else if (lockstateB == 1)  // opened -> locked
    {
      lockstateB = 0;
      //rotate twice right
      delay(1000);
      digitalWrite(4, HIGH);digitalWrite(3, LOW);
      delay(1000);   
      digitalWrite(4, LOW);digitalWrite(3, HIGH);
      delay(1000);   
      digitalWrite(4, HIGH);digitalWrite(3, HIGH);
      delay(1000);   
      digitalWrite(4, LOW);digitalWrite(3, LOW);
      delay(1000);  
      digitalWrite(4, HIGH);digitalWrite(3, LOW);
      delay(1000);   
      digitalWrite(4, LOW);digitalWrite(3, HIGH);
      delay(1000);   
      digitalWrite(4, HIGH);digitalWrite(3, HIGH);
      delay(1000);   
      digitalWrite(4, LOW);digitalWrite(3, LOW);
      delay(1000);
    }
  }else{
    if(lockstateA == 0){  // locked -> opened
      lockstateA = 1;
      //rotate twice left
      delay(1000);
      digitalWrite(6, HIGH);digitalWrite(5, HIGH);
      delay(1000);
      digitalWrite(6, LOW);digitalWrite(5, HIGH);
      delay(1000);
      digitalWrite(6, HIGH);digitalWrite(5, LOW);
      delay(1000);
      digitalWrite(6, LOW);digitalWrite(5, LOW);
      delay(1000);
      digitalWrite(6, HIGH);digitalWrite(5, HIGH);
      delay(1000);
      digitalWrite(6, LOW);digitalWrite(5, HIGH);
      delay(1000);
      digitalWrite(6, HIGH);digitalWrite(5, LOW);
      delay(1000);
      digitalWrite(6, LOW);digitalWrite(5, LOW);
      delay(1000);
    }else if (lockstateA == 1)  // opened -> locked
    {
      lockstateA = 0;
      //rotate twice right
      delay(1000);
      digitalWrite(6, HIGH);digitalWrite(5, LOW);
      delay(1000);   
      digitalWrite(6, LOW);digitalWrite(5, HIGH);
      delay(1000);   
      digitalWrite(6, HIGH);digitalWrite(5, HIGH);
      delay(1000);   
      digitalWrite(6, LOW);digitalWrite(5, LOW);
      delay(1000);  
      digitalWrite(6, HIGH);digitalWrite(5, LOW);
      delay(1000);   
      digitalWrite(6, LOW);digitalWrite(5, HIGH);
      delay(1000);   
      digitalWrite(6, HIGH);digitalWrite(5, HIGH);
      delay(1000);   
      digitalWrite(6, LOW);digitalWrite(5, LOW);
      delay(1000);
    }
  }
}
