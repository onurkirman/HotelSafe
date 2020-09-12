## - Hotel Safe-
**Introduction**

Hotel Safe is an electronics system with a CPU, a numpad, a 4 - row LCD, 2 motors and an Emergency LED.
It has 2 sections with individual passwords, a master password to open any section and a bait password
that when entered will alert the police. It is a simplified application of an actual safe system.

All passwords are 4 digits between 0-9. Master Password, which is used to open any section, change
passwords), Side Password 1, which is used to open section A, Side Password 2, which is used to open
section B, Bait Password to open any section with blinking LED meaning Police Signal.

**Problem Statement**

There are a lot of problems in the system to begin with. First the user can only press 1,2,3 and 4 numbers
in the keypad. We need to limit the action of users to open section A, open section B, change passwords
and report attempts.

Master Password must be opening and locking all sections and must be able to change other password
values. Bait Password must be able to open safely all the sections with signaling the LED. Side Passwords
must work only desired sections for single sections each.

LCD must be update for every important state changes and must stay the same for a while for the actions
that needs to be stayed on such as reporting successful and unsuccessful attempts. It must not display the
keys the user pressing, must only indicate the pressed buttons with “*”.

Arduino Timer needs to check Successful Failure Attempts every second and must count to 15 second to
reset the SFA value. If a user enters a wrong password within the 15 second count it must stop the counting
and restart again, meaning it must reset and count 15 seconds again.

System must hold total successful and total unsuccessful attempts after power-up. It must be incremented
for every password entry properly and according to its result.

For the Time Lock Screen, it must show the time on LCD. It must work flowless and properly so that the
time passed is exactly 20 seconds.


In Change Password state, if user enters wrong Master Password by making SFA more than 2, system must
go to Master Lock State. Also, it must be stuck until the right Master Password is entered. Unless it goes
into Time Lock Screen and Master Lock Screen back and forth.

EEPROM must be checked for default initialization. It must not reset to default values for every power-up.
So that it works fine whenever we boot the Arduino, it keeps the values according to last assignment.

Number of pins was not enough for bistepper motor installation. I needed to control 2 of them with 8 pins.
They were taking so many pins to control and organize.

**Solution**

For the input part at the Main Screen, I took the input if the pressed button is 1,2,3 or 4 and ignored the
other cases. User can only access to functions, meaning states, by pushing these 4 buttons, other states
are prohibited.

Master Password set to open and lock all the sections, change passwords, and unlock from the Master
Lock Screen. Side Passwords are used according to signed sections and nothing else. Bait Password opens
and locks the sections just like a regular Side Password with blinking LED meaning Police signal.

LCD is refreshed for every unique states and changes. It clears and writes every string according to their
needs. Detailed version is shown in the State Machine Part of the Report.

Arduino Timer 1 is being used to interrupt and execute the code written in the ISR(TIMER1_COMPA_vect).
Timer is configured to interrupt for every second. In this function, it checks the value of SFA and starts
counting for 15 seconds. If the value of SFA is not changed it resets after it reaches to 15 second. But if the
value of SFA changes it restarts the count and waits another 15 seconds from beginning.

The configuration of the Timer 1, we first stop the interrupts with cli() function. Later we set TCCR1A
register to 0, TCCR1B to 0 and later make its WGM12 register equal to 1 which turns on the CTC mode and
later assigned to TCCR1B register with or gate, TCNT1 register to 0 which is counter value. To set the
compare match register for 1Hz which is 1 second increments we set OCR1A to 15624. This value is
calculated using the this formula “compare_match_register=[16MHz/(prescaler*wanted interrupt
frequency )]-1”. This derived from “(timer speed (Hz)) = (Arduino clock speed (16MHz)) / prescaler”.

My Arduino Uno R3’s clock speed is 16MHz and uses 16 bit registers. It has 1, 8, 64, 256, and 1024 prescaler
values which is controlled with CS12, CS11, CS10 registers. I set the prescaler to 1024 by making CS10 and


CS12 bits 1 which is assigned to TCCR1B register later with or gate. At the end we enabled the timer
compare interrupt register by assigning OCIE1A register 1 and or gating TIMSK1 register of Arduino. In
short, I used 1Hz setup to trigger every second and do the SFA checking and resetting. From the Compare
match register calculation, I got 15624 as value and assigned it to proper register with choosing 1024 as
prescaler which has 101 as register bits and done as according to mentioned before.

I added two separate Demultiplexer to control the 4 pins of the bistepper motors only using 2 pins. This
helped me to control 2 of them with only 4 pins by reducing it from 8.

Also, for the EEPROM part I added 2 check points. If the 0. address value of EEPROM is trash while setup
is being done. It overwrites the values as ‘0’. Later, If the 0. address value of the EEPROM is not equal to
‘1’ it goes and writes the default values to addresses specified starting from 1-16 by separating them 4
address each. Because we have 4 Passwords that needs to be stored.

**User Manual**

The system comes locks opened and after entering the correct passwords it lock according to their sections
by rotating clockwise twice. Later opens the locks by rotating counter-clockwise twice. My program starts
in Main Menu by taking input between 1-4. It holds the state of 6 with additional start input state meaning
in total 7 states. It assigns the first menu input to state and goes to that state function. It waits for 4 - digit
Password to be entered and checks automatically without any other input. It takes the action as it is shown
in the state machine. Opens/ Locks the section desired or Changes the desired password after taking
Master Password. If the user enters consecutive wrong password state goes to Master Lock and waits for
Master Password to be given. Later, if user still enters the wrong key it puts itself a 20 second loop until
the right Master Password is entered. While all these are happening, it resets the SFA for every 15 second
if it is not increased during that period.

The program and the whole system work fine. It displays clearly as wanted and does the functionality as
wanted with bonus of holding passwords in EEPROM which enables us to hold the passwords even if the
Arduino restarted. Also, default Passwords are set according to shown in the list down below.

- Master Password: “ 1234 ”
- Section A Password: “ 2222 ”
- Section B Password: “ 3333 ”
- Bait Password: “ 9999 ”


**State Machine**

**A. Main Screen** (LCD: “Main Menu”) -> Enter 1, 2, 3 or 4

**1. Open/Lock Section A** (LCD: “Open/Lock Section A”)
    
    a) **Enter Master Password** - > open/lock section A with (LCD: Row1: “Section A - Master”), (LCD: Row2: “Opening/Locking”) - > return Main Screen
    
    b) **Enter Side Password 1** - > open/lock section A with (LCD: Row1: “Section A - Standard”), (LCD: Row2: “Opening/Locking”) -> return Main Screen
    
    c) **Enter Bait Password** - > open/lock section A with (LCD: Row1: “Section A - Standard”), (LCD: Row2: “Opening/Locking”) and blink Emergency LED -> return Main Screen
    
    d) **Enter Any Other Password** - > (LCD: “Wrong Password”) -> Increase Successive Failure Attempts ( **SFA** ) counter -> if SFA < 3 return Open/Lock Section A, else GOTO Master Lock Screen.
    
**2. Open/Lock Section B** (LCD: “Open/Lock Section B”)
    
    a) Enter Master Password - > open/lock section B with (LCD: Row1: “Section B - Master”), (LCD: Row2: “Opening/Locking”) - > return Main Screen
    
    b) Side Password 1 - > open/lock section B with (LCD: Row1: “Section B - Standard”), (LCD: Row2: “Opening/Locking”) -> return Main Screen
    
    c) Enter Bait Password - > open/lock section B with (LCD: Row1: “Section B - Standard”), (LCD: Row2: “Opening/Locking”) and blink Emergency LED -> return Main Screen
    
    d) Enter Any Other Password -> (LCD: “Wrong Password”) -> Increase Successive Failure Attempts (SFA) counter -> if SFA < 3 return Open/Lock Section B, else GOTO Master Lock Screen.
    
**3. Change Password** (LCD: Row1: “Master Pass. Needed”), (LCD: Row2: “To Change Pass.”)
    
    a) **Enter Master Password** - > Enter 1, 2, 3 or 4 (LCD: Row 1 :” Change Password”), (LCD: Row2: “1M 2A 3B 4T)
          - **Change Master Password** (LCD: “Enter New Pass M”) -> Enter New Password - > Change the password (LCD: “Pass is changed”) -> return Main Screen
          - **Change Section A Password** (LCD: “Enter New Pass A”) -> Enter New Password - > Change the password (LCD: “Pass is changed”) -> return Main Screen
          - **Change Section B Password** (LCD: “Enter New Pass B”) -> Enter New Password - > Change the password (LCD: “Pass is changed”) -> return Main Screen
          - **Change Bait Password** (LCD: “Enter New Pass T”) -> Enter New Password -> Change the password (LCD: “Pass is changed”) -> return Main Screen
    
    b) **Enter Any Other Password** - > (LCD: “Wrong Password”) -> Increase Successive Failure Attempts (SFA) counter -> if SFA less than 3 return Change Password, else GOTO Master Lock Screen.

**4. Report Previous Attempts** (LCD: Row1: “Suc. Attempts: XX”), (LCD: Row2: “UnSuc. Attempts: YY”), (LCD: Row3: “ SFA: ZZ”) after 5 seconds -> return Main Screen

**5. Master Lock Screen** (LCD: “System Locked MP”)
    
    a) **Enter Master Password** - > (LCD: “Lock is UnLocked!”) GOTO Main Screen
    
    b) **Enter Any Other Password** - > GOTO Time Lock Screen

**6. Time Lock Screen** (LCD: “No access %d sec”, countdown from 20)) -> wait 20 seconds -> return Master Lock Screen

**Components Used:**

- 1 x Arduino Uno R
- 1 x Keypad-Phone – Interactive matrix keypad for phone
- 1 x LM044 L - 20x4 Alphanumeric LCD
- 2 x Motor Bistepper – Bipolar Stepper Motor
- 2 x 4555 - Dual 1 to 4 Line Decoder / Demultiplexer
- 1 x LED – Red
- 1 x Resistor – 330 Ohm

**Environment Needed:**

- Proteus Design Suite
- Arduino IDE with EEPROM, LiquidCrystal and Keypad libraries installed.
- Proteus Arduino Library


For more information please contact via e-mail onur.kirman@ozu.edu.tr

**_Onur Kirman - S009958_**

**_Electrical and Electronics Engineering – Özyeğin University_**
