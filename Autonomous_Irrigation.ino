/*****************************************************************************
* Copyright (C) 2016-2017 by Vasco Ferraz. All Rights Reserved.              *
*                                                                            *
* This program is free software: you can redistribute it and/or modify       *
* it under the terms of the GNU General Public License as published by       *
* the Free Software Foundation, either version 3 of the License, or          *
* (at your option) any later version.                                        *
*                                                                            *
* This program is distributed in the hope that it will be useful,            *
* but WITHOUT ANY WARRANTY; without even the implied warranty of             *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the               *
* GNU General Public License for more details.                               *
*                                                                            *
* You should have received a copy of the GNU General Public License          *
* along with this program. If not, see <http://www.gnu.org/licenses/>.       *
*                                                                            *
*  Author:        Vasco Ferraz                                               *
*  Contact:       http://vascoferraz.com/contact/                            *
*  Description:   http://vascoferraz.com/projects/autonomous-irrigation/     *
*****************************************************************************/

// Import libraries
#include <Wire.h> // I2C and TWI library.
#include <RTClib.h> // RTClib library.
#include <LCD4884.h> // LCD4884 library.
#include <EEPROM.h> // EEPROM library. The microcontroller on the Arduino board has an EEPROM: memory whose values are kept...
                    // ...when the board is turned off (like a tiny hard drive). This library enables you to read and write those bytes.
                    // The microcontrollers on the various Arduino boards have different amounts of EEPROM: 1024 bytes on the ATmega328P,
                    // 512 bytes on the ATmega168 and ATmega8 and 4 kB (4096 bytes) on the ATmega1280 and ATmega2560.

// Define constants
#define DS1307_I2C_ADDRESS 0x68 // Each I2C object has a unique bus address, the DS1307 (Real Time Clock) is 0x68.
#define MENU_X 0 // Set up the position of the cursor in the x axis: 0-83. Each char uses a matrix of 6x8 pixels.
#define MENU_Y 0 // Set up the position of the cursor in the y axis: 0-5. Each char uses a matrix of 6x8 pixels.

// Declare objects
RTC_DS1307 RTC; // Declare variable RTC of type RTC_DS1307.

// Declare global variables
unsigned char upbar[] =        {0x02, 0x01, 0x02, 0x01, 0x02, 0x01}; // Custom char. Each char is composed by a 6x8 pixel matrix.
unsigned char degree[] =       {0x00, 0x0E, 0x11, 0x11, 0x11, 0x0E}; // Custom char. Each char is composed by a 6x8 pixel matrix.
unsigned char valve_open[] =   {0xFF, 0x18, 0x00, 0x00, 0x18, 0xFF}; // Custom char. Each char is composed by a 6x8 pixel matrix.
unsigned char valve_closed[] = {0xFF, 0x18, 0x18, 0x18, 0x18, 0xFF}; // Custom char. Each char is composed by a 6x8 pixel matrix.
unsigned char valve_left[] =   {0xFF, 0x01, 0x01, 0x01, 0x01, 0xF1}; // Custom char. Each char is composed by a 6x8 pixel matrix.
unsigned char valve_center[] = {0x11, 0x10, 0x10, 0x10, 0x10, 0x11}; // Custom char. Each char is composed by a 6x8 pixel matrix.
unsigned char valve_right[] =  {0xF1, 0x01, 0x01, 0x01, 0x01, 0xFF}; // Custom char. Each char is composed by a 6x8 pixel matrix.

int second, minute, hour, dayOfWeek, day, month, year;
// To print in the LDC the following variables they must be stored in a char array.
// If you're experiencing strange issues while printing these variables (or another "string_" variable) try to increase the size of the array.
char string_second[3], string_minute[3], string_hour[3], string_dayOfWeek[3], string_day[3], string_month[3], string_year[3];

const unsigned char joystick = A0; // Input pin where the joystick is connected.
unsigned char ClockModeState = 0; // Current LCD mode state.
unsigned char ClockModeFlag = 0; // Flag used for debouncing the joystick pushbutton.
unsigned int gotomain=0; // If there is no input, automatically jump to main display when gotomain = gotomaincounter ...
const int gotomaincounter=200; // ... Change this time by manipulating the gotomaincouter value.

const unsigned char mainValve=8, leftValve = 9, rightValve = 10; // Output pin where the main/left/right valve is connected.
char mainValveOpenHour, mainValveCloseHour, mainValveOpenMinute, mainValveCloseMinute;     // Hour/Minute when the main valve will open/close.
char leftValveOpenHour, leftValveCloseHour, leftValveOpenMinute, leftValveCloseMinute;     // Hour/Minute when the left valve will open/close.
char rightValveOpenHour, rightValveCloseHour, rightValveOpenMinute, rightValveCloseMinute; // Hour/Minute when the right valve will open/close.
char string_mainValveOpenHour[3], string_mainValveCloseHour[3], string_mainValveOpenMinute[3], string_mainValveCloseMinute[3];
char string_leftValveOpenHour[3], string_leftValveCloseHour[3], string_leftValveOpenMinute[3], string_leftValveCloseMinute[3];
char string_rightValveOpenHour[3], string_rightValveCloseHour[3], string_rightValveOpenMinute[3], string_rightValveCloseMinute[3];
unsigned char mainValveOpenHourMemoryBank = 0;      // This is the position on ATmega328P's EEPROM where the main valve open hour is stored.
unsigned char mainValveCloseHourMemoryBank = 1;     // This is the position on ATmega328P's EEPROM where the main valve close hour is stored.
unsigned char mainValveOpenMinuteMemoryBank = 2;    // This is the position on ATmega328P's EEPROM where the main valve open minute is stored.
unsigned char mainValveCloseMinuteMemoryBank = 3;   // This is the position on ATmega328P's EEPROM where the main valve close minute is stored.
unsigned char leftValveOpenHourMemoryBank = 4;      // This is the position on ATmega328P's EEPROM where the left valve open hour is stored.
unsigned char leftValveCloseHourMemoryBank = 5;     // This is the position on ATmega328P's EEPROM where the left valve open close hour is stored.
unsigned char leftValveOpenMinuteMemoryBank = 6;    // This is the position on ATmega328P's EEPROM where the left valve open open minute is stored.
unsigned char leftValveCloseMinuteMemoryBank = 7;   // This is the position on ATmega328P's EEPROM where the left valve open close minute is stored.
unsigned char rightValveOpenHourMemoryBank = 8;     // This is the position on ATmega328P's EEPROM where the right valve open open hour is stored.
unsigned char rightValveCloseHourMemoryBank = 9;    // This is the position on ATmega328P's EEPROM where the right valve open close hour is stored.
unsigned char rightValveOpenMinuteMemoryBank = 10;  // This is the position on ATmega328P's EEPROM where the right valve open open minute is stored.
unsigned char rightValveCloseMinuteMemoryBank = 11; // This is the position on ATmega328P's EEPROM where the right valve open close minute is stored.

// The higher this value is, the longer it will take to increment day, month, year, hour, minute, second, mainValveOpenHour, mainValveCloseHour... variables.
const unsigned int settingdelay = 200;

byte decToBcd(byte val){return ( (val/10*16) + (val%10) );} // Convert normal decimal numbers to binary coded decimal.
byte bcdToDec(byte val){return ( (val/16*10) + (val%16) );} // Convert binary coded decimal to normal decimal numbers.


void setup()
{
  //delay(100); // This delay will prevent the random (or last saved on/off state) activation of the LEDs.

  Serial.begin(9600); // Open serial port and sets data rate to 9600 bps.
  Wire.begin(); // Start the Wire (I2C communications).
  RTC.begin(); // Start the RTC Chip.

  while (! Serial); // Wait until Serial is ready (needed for Leonardo only).
  Serial.setTimeout(10); // Sets the maximum milliseconds to wait for serial data.

  pinMode(joystick, INPUT);    // Initialize the joystick pin as an input.
  pinMode(mainValve, OUTPUT);  // Initialize the main valve pin as an output.
  pinMode(leftValve, OUTPUT);  // Initialize the left valve pin as an output.
  pinMode(rightValve, OUTPUT); // Initialize the right valve pin as an output.

  digitalWrite(mainValve, LOW);  // Starts with the main valve closed
  digitalWrite(leftValve, LOW);  // Starts with the left valve closed
  digitalWrite(rightValve, LOW); // Starts with the right valve closed


  // 1A - mainValveOpenHour out of range. If for some reason the stored value (mainValveOpenHour) in ATmega328P's EEPROM is out of range it will be set to a valid hour.
  mainValveOpenHour = EEPROM.read(mainValveOpenHourMemoryBank);
  if (mainValveOpenHour >= 0 && mainValveOpenHour <= 23) {mainValveOpenHour=mainValveOpenHour;}
  else (mainValveOpenHour = 15); // Default dawn value.
  EEPROM.write(mainValveOpenHourMemoryBank,mainValveOpenHour);

  // 1B - mainValveCloseHour out of range. If for some reason the stored value (mainValveCloseHour) in ATmega328P's EEPROM is out of range it will be set to a valid hour.
  mainValveCloseHour = EEPROM.read(mainValveCloseHourMemoryBank);
  if (mainValveCloseHour >= 0 && mainValveCloseHour <= 23) {mainValveCloseHour=mainValveCloseHour;}
  else (mainValveCloseHour = 22); // Default dawn value.
  EEPROM.write(mainValveCloseHourMemoryBank,mainValveCloseHour);

  // 1C - mainValveOpenMinute out of range. If for some reason the stored value (mainValveOpenMinute) in ATmega328P's EEPROM is out of range it will be set to a valid minute.
  mainValveOpenMinute = EEPROM.read(mainValveOpenMinuteMemoryBank);
  if (mainValveOpenMinute >= 0 && mainValveOpenMinute <= 59) {mainValveOpenMinute=mainValveOpenMinute;}
  else (mainValveOpenMinute = 55); // Default dawn value.
  EEPROM.write(mainValveOpenMinuteMemoryBank,mainValveOpenMinute);

  // 1D - mainValveCloseMinute out of range. If for some reason the stored value (mainValveCloseMinute) in ATmega328P's EEPROM is out of range it will be set to a valid minute.
  mainValveCloseMinute = EEPROM.read(mainValveCloseMinuteMemoryBank);
  if (mainValveCloseMinute >= 0 && mainValveCloseMinute <= 59) {mainValveCloseMinute=mainValveCloseMinute;}
  else (mainValveCloseMinute = 5); // Default dawn value.
  EEPROM.write(mainValveCloseMinuteMemoryBank,mainValveCloseMinute);


  // 2A - leftValveOpenHour out of range. If for some reason the stored value (leftValveOpenHour) in ATmega328P's EEPROM is out of range it will be set to a valid hour.
  leftValveOpenHour = EEPROM.read(leftValveOpenHourMemoryBank);
  if (leftValveOpenHour >= 0 && leftValveOpenHour <= 23) {leftValveOpenHour=leftValveOpenHour;}
  else (leftValveOpenHour = 16); // Default dawn value.
  EEPROM.write(leftValveOpenHourMemoryBank,leftValveOpenHour);

  // 2B - leftValveCloseHour out of range. If for some reason the stored value (leftValveCloseHour) in ATmega328P's EEPROM is out of range it will be set to a valid hour.
  leftValveCloseHour = EEPROM.read(leftValveCloseHourMemoryBank);
  if (leftValveCloseHour >= 0 && leftValveCloseHour <= 23) {leftValveCloseHour=leftValveCloseHour;}
  else (leftValveCloseHour = 19); // Default dawn value.
  EEPROM.write(leftValveCloseHourMemoryBank,leftValveCloseHour);

  // 2C - leftValveOpenMinute out of range. If for some reason the stored value (leftValveOpenMinute) in ATmega328P's EEPROM is out of range it will be set to a valid minute.
  leftValveOpenMinute = EEPROM.read(leftValveOpenMinuteMemoryBank);
  if (leftValveOpenMinute >= 0 && leftValveOpenMinute <= 59) {leftValveOpenMinute=leftValveOpenMinute;}
  else (leftValveOpenMinute = 0); // Default dawn value.
  EEPROM.write(leftValveOpenMinuteMemoryBank,leftValveOpenMinute);

  // 2D - leftValveCloseMinute out of range. If for some reason the stored value (leftValveCloseMinute) in ATmega328P's EEPROM is out of range it will be set to a valid minute.
  leftValveCloseMinute = EEPROM.read(leftValveCloseMinuteMemoryBank);
  if (leftValveCloseMinute >= 0 && leftValveCloseMinute <= 59) {leftValveCloseMinute=leftValveCloseMinute;}
  else (leftValveCloseMinute = 0); // Default dawn value.
  EEPROM.write(leftValveCloseMinuteMemoryBank,leftValveCloseMinute);


  // 3A - rightValveOpenHour out of range. If for some reason the stored value (rightValveOpenHour) in ATmega328P's EEPROM is out of range it will be set to a valid hour.
  rightValveOpenHour = EEPROM.read(rightValveOpenHourMemoryBank);
  if (rightValveOpenHour >= 0 && rightValveOpenHour <= 23) {rightValveOpenHour=rightValveOpenHour;}
  else (rightValveOpenHour = 19); // Default dawn value.
  EEPROM.write(rightValveOpenHourMemoryBank,rightValveOpenHour);

  // 3B - rightValveCloseHour out of range. If for some reason the stored value (rightValveCloseHour) in ATmega328P's EEPROM is out of range it will be set to a valid hour.
  rightValveCloseHour = EEPROM.read(rightValveCloseHourMemoryBank);
  if (rightValveCloseHour >= 0 && rightValveCloseHour <= 23) {rightValveCloseHour=rightValveCloseHour;}
  else (rightValveCloseHour = 22); // Default dawn value.
  EEPROM.write(rightValveCloseHourMemoryBank,rightValveCloseHour);

  // 3C - rightValveOpenMinute out of range. If for some reason the stored value (rightValveOpenMinute) in ATmega328P's EEPROM is out of range it will be set to a valid minute.
  rightValveOpenMinute = EEPROM.read(rightValveOpenMinuteMemoryBank);
  if (rightValveOpenMinute >= 0 && rightValveOpenMinute <= 59) {rightValveOpenMinute=rightValveOpenMinute;}
  else (rightValveOpenMinute = 0); // Default dawn value.
  EEPROM.write(rightValveOpenMinuteMemoryBank,rightValveOpenMinute);

  // 3D - rightValveCloseMinute out of range. If for some reason the stored value (rightValveCloseMinute) in ATmega328P's EEPROM is out of range it will be set to a valid minute.
  rightValveCloseMinute = EEPROM.read(rightValveCloseMinuteMemoryBank);
  if (rightValveCloseMinute >= 0 && rightValveCloseMinute <= 59) {rightValveCloseMinute=rightValveCloseMinute;}
  else (rightValveCloseMinute = 0); // Default dawn value.
  EEPROM.write(rightValveCloseMinuteMemoryBank,rightValveCloseMinute);

  
  //Print available (free) SRAM in Serial Monitor.
  Serial.print(F("Available SRAM: "));
  Serial.print(availableSRAM());
  Serial.println(F(" bytes"));

  initSerialCommunication(); // Initialize serial communication.
  initRTC(); // Initialize Real Time Clock.
  lcd.LCD_init(); // Initialize LCD.
  lcd.LCD_clear(); // Clear LCD.
}


void loop()
{
  while (ClockModeState == 0)
  {
   readClock();
   PrintTimeOnLCD();
   PrintDateOnLCD();
   PrintWeekDayOnLCD();
   Valves();
   SwitchClockMode_right(1);
   SwitchClockMode_left(18);
  }
  
  while (ClockModeState == 1)
  {SetHour();}

  while (ClockModeState == 2)
  {SetMinute();}

  while (ClockModeState == 3)
  {SetSecond();}

  while (ClockModeState == 4)
  {SetDay();}

  while (ClockModeState == 5)
  {SetMonth();}

  while (ClockModeState == 6)
  {SetYear();}

  while (ClockModeState == 7)
  {SetMainValveOpenHour();}

  while (ClockModeState == 8)
  {SetMainValveOpenMinute();}

  while (ClockModeState == 9)
  {SetMainValveCloseHour();}
  
  while (ClockModeState == 10)
  {SetMainValveCloseMinute();}

  while (ClockModeState == 11)
  {SetLeftValveOpenHour();}

  while (ClockModeState == 12)
  {SetLeftValveOpenMinute();}

  while (ClockModeState == 13)
  {SetLeftValveCloseHour();}
   
  while (ClockModeState == 14)
  {SetLeftValveCloseMinute();}

  while (ClockModeState == 15)
  {SetRightValveOpenHour();}

  while (ClockModeState == 16)
  {SetRightValveOpenMinute();}

  while (ClockModeState == 17)
  {SetRightValveCloseHour();}
 
  while (ClockModeState == 18)
  {SetRightValveCloseMinute();}
}


// Print time on the LCD
void PrintTimeOnLCD (void)
{
  DateTime now = RTC.now();
  
  if (now.hour() < 10)
  {
   itoa(now.hour(),string_hour,10);
   lcd.LCD_write_string(MENU_X, MENU_Y, "0", MENU_NORMAL);
   lcd.LCD_write_string(MENU_X+6, MENU_Y, string_hour, MENU_NORMAL);
   lcd.LCD_write_string(MENU_X+12, MENU_Y, ":", MENU_NORMAL);
  }
  else
  {
   itoa(now.hour(),string_hour,10);
   lcd.LCD_write_string(MENU_X, MENU_Y, string_hour, MENU_NORMAL);
   lcd.LCD_write_string(MENU_X+12, MENU_Y, ":", MENU_NORMAL);
  }
  
  if (now.minute() < 10)
  {
   itoa(now.minute(),string_minute,10);
   lcd.LCD_write_string(MENU_X+18, MENU_Y, "0", MENU_NORMAL);
   lcd.LCD_write_string(MENU_X+24, MENU_Y, string_minute, MENU_NORMAL);
   lcd.LCD_write_string(MENU_X+30, MENU_Y, ":", MENU_NORMAL);
  }
  else
  {
   itoa(now.minute(),string_minute,10);
   lcd.LCD_write_string(MENU_X+18, MENU_Y, string_minute, MENU_NORMAL);
   lcd.LCD_write_string(MENU_X+30, MENU_Y, ":", MENU_NORMAL);
  }
  
  if (now.second() < 10)
  {
   itoa(now.second(),string_second,10);
   lcd.LCD_write_string(MENU_X+36, MENU_Y, "0", MENU_NORMAL);
   lcd.LCD_write_string(MENU_X+42, MENU_Y, string_second, MENU_NORMAL);
  }
  else
  {
   itoa(now.second(),string_second,10);
   lcd.LCD_write_string(MENU_X+36, MENU_Y, string_second, MENU_NORMAL);
  }
}


// Print date on the LCD
void PrintDateOnLCD(void)
{
  DateTime now = RTC.now();
  
  if (now.day() < 10)
  {
   itoa(now.day(),string_day,10);
   lcd.LCD_write_string(MENU_X, MENU_Y+1, "0", MENU_NORMAL);
   lcd.LCD_write_string(MENU_X+6, MENU_Y+1, string_day, MENU_NORMAL);
   lcd.LCD_write_string(MENU_X+12, MENU_Y+1, "/", MENU_NORMAL);
  }
  else
  {
   itoa(now.day(),string_day,10);
   lcd.LCD_write_string(MENU_X, MENU_Y+1, string_day, MENU_NORMAL);
   lcd.LCD_write_string(MENU_X+12, MENU_Y+1, "/", MENU_NORMAL);
  }

  switch(now.month())
  {
  case 1:  lcd.LCD_write_string(MENU_X+18, MENU_Y+1, "Jan/", MENU_NORMAL); break;     
  case 2:  lcd.LCD_write_string(MENU_X+18, MENU_Y+1, "Feb/", MENU_NORMAL); break; 
  case 3:  lcd.LCD_write_string(MENU_X+18, MENU_Y+1, "Mar/", MENU_NORMAL); break; 
  case 4:  lcd.LCD_write_string(MENU_X+18, MENU_Y+1, "Apr/", MENU_NORMAL); break; 
  case 5:  lcd.LCD_write_string(MENU_X+18, MENU_Y+1, "May/", MENU_NORMAL); break; 
  case 6:  lcd.LCD_write_string(MENU_X+18, MENU_Y+1, "Jun/", MENU_NORMAL); break; 
  case 7:  lcd.LCD_write_string(MENU_X+18, MENU_Y+1, "Jul/", MENU_NORMAL); break; 
  case 8:  lcd.LCD_write_string(MENU_X+18, MENU_Y+1, "Aug/", MENU_NORMAL); break;      
  case 9:  lcd.LCD_write_string(MENU_X+18, MENU_Y+1, "Sep/", MENU_NORMAL); break; 
  case 10: lcd.LCD_write_string(MENU_X+18, MENU_Y+1, "Oct/", MENU_NORMAL); break; 
  case 11: lcd.LCD_write_string(MENU_X+18, MENU_Y+1, "Nov/", MENU_NORMAL); break; 
  case 12: lcd.LCD_write_string(MENU_X+18, MENU_Y+1, "Dec/", MENU_NORMAL); break; 
  default: lcd.LCD_write_string(MENU_X+18, MENU_Y+1, "Err/", MENU_NORMAL);
  }
  
  itoa(now.year(),string_year,10);
  lcd.LCD_write_string(MENU_X+42, MENU_Y+1, string_year, MENU_NORMAL);
}


// Print weekday on the LCD
void PrintWeekDayOnLCD(void)
{
  DateTime now = RTC.now();
  switch(now.dayOfTheWeek())
  {
  case 0: lcd.LCD_write_string(MENU_X+54, MENU_Y, "Sun", MENU_NORMAL); break;
  case 1: lcd.LCD_write_string(MENU_X+54, MENU_Y, "Mon", MENU_NORMAL); break;
  case 2: lcd.LCD_write_string(MENU_X+54, MENU_Y, "Tue", MENU_NORMAL); break;
  case 3: lcd.LCD_write_string(MENU_X+54, MENU_Y, "Wed", MENU_NORMAL); break;
  case 4: lcd.LCD_write_string(MENU_X+54, MENU_Y, "Thu", MENU_NORMAL); break;
  case 5: lcd.LCD_write_string(MENU_X+54, MENU_Y, "Fri", MENU_NORMAL); break;  
  case 6: lcd.LCD_write_string(MENU_X+54, MENU_Y, "Sat", MENU_NORMAL); break; 
  default:lcd.LCD_write_string(MENU_X+54, MENU_Y, "Err", MENU_NORMAL);
  } 
}

// Valves
void Valves(void)
{ 
  DateTime now = RTC.now();
  
  // Convert time into a single 4 digit integer (for each variable).
  int currentTime, mainValveOpenTime, mainValveCloseTime, leftValveOpenTime, leftValveCloseTime, rightValveOpenTime, rightValveCloseTime;

  currentTime = now.hour()*100 + now.minute();
  mainValveOpenTime = mainValveOpenHour*100 + mainValveOpenMinute;
  mainValveCloseTime = mainValveCloseHour*100 + mainValveCloseMinute;
  leftValveOpenTime = leftValveOpenHour*100 + leftValveOpenMinute;
  leftValveCloseTime = leftValveCloseHour*100 + leftValveCloseMinute;
  rightValveOpenTime = rightValveOpenHour*100 + rightValveOpenMinute;
  rightValveCloseTime = rightValveCloseHour*100 + rightValveCloseMinute;
 
  /* Just for debugging
  Serial.print("Current Time: ");
  Serial.println(currentTime);
  Serial.print("Main Valve Open: ");
  Serial.println(mainValveOpenTime);
  Serial.print("Main Valve Close: ");
  Serial.println(mainValveCloseTime);
  Serial.print("Left Valve Open: ");
  Serial.println(leftValveOpenTime);
  Serial.print("Main Valve Close: ");
  Serial.println(leftValveCloseTime);
  Serial.print("Right Valve Open: ");
  Serial.println(rightValveOpenTime);
  Serial.print("Right Valve Close: ");
  Serial.println(rightValveCloseTime);
  */

  lcd.LCD_draw_bmp_pixel(0,4, valve_left, 6,8);
  lcd.LCD_draw_bmp_pixel(6,4, valve_center, 6,8);
  lcd.LCD_draw_bmp_pixel(12,4, valve_right, 6,8); 


  // Main Valve
  if (mainValveOpenTime < mainValveCloseTime)
  {
   if (currentTime >= mainValveOpenTime && currentTime < mainValveCloseTime)
   { digitalWrite(mainValve, HIGH); lcd.LCD_draw_bmp_pixel(6,3, valve_open, 6,8); lcd.LCD_write_string(MENU_X+24, MENU_Y+3, "M:11-Open ", MENU_HIGHLIGHT); Serial.println("M11 open"); }
   else
   { digitalWrite(mainValve, LOW); lcd.LCD_draw_bmp_pixel(6,3, valve_closed, 6,8); lcd.LCD_write_string(MENU_X+24, MENU_Y+3, "M:12-Close", MENU_HIGHLIGHT); Serial.println("M12 close"); }
  }
  
  if (mainValveOpenTime > mainValveCloseTime)
  {
   if (currentTime >= mainValveOpenTime && currentTime <= 2359)
   { digitalWrite(mainValve, HIGH); lcd.LCD_draw_bmp_pixel(6,3, valve_open, 6,8); lcd.LCD_write_string(MENU_X+24, MENU_Y+3, "M:21-Open ", MENU_HIGHLIGHT); Serial.println("M21 open"); }
   if (currentTime >= 0 && currentTime < mainValveCloseTime)
   { digitalWrite(mainValve, HIGH); lcd.LCD_draw_bmp_pixel(6,3, valve_open, 6,8); lcd.LCD_write_string(MENU_X+24, MENU_Y+3, "M:22-Open ", MENU_HIGHLIGHT); Serial.println("M22 open"); }
   if (currentTime >= mainValveCloseTime && currentTime < mainValveOpenTime)
   { digitalWrite(mainValve, LOW); lcd.LCD_draw_bmp_pixel(6,3, valve_closed, 6,8); lcd.LCD_write_string(MENU_X+24, MENU_Y+3, "M:23-Close", MENU_HIGHLIGHT); Serial.println("M23 close"); }   
  }

  if (mainValveOpenTime == mainValveCloseTime)
  { digitalWrite(mainValve, LOW); lcd.LCD_draw_bmp_pixel(6,3, valve_closed, 6,8); lcd.LCD_write_string(MENU_X+24, MENU_Y+3, "M:31-Close", MENU_HIGHLIGHT); Serial.println("M31 close"); }


  // Left Valve
  if (leftValveOpenTime < leftValveCloseTime)
  {
   if (currentTime >= leftValveOpenTime && currentTime < leftValveCloseTime)
   { digitalWrite(leftValve, HIGH); lcd.LCD_draw_bmp_pixel(0,5, valve_open, 6,8); lcd.LCD_write_string(MENU_X+24, MENU_Y+4, "L:11-Open ", MENU_HIGHLIGHT); Serial.println("L11 open"); }
   else
   { digitalWrite(leftValve, LOW); lcd.LCD_draw_bmp_pixel(0,5, valve_closed, 6,8); lcd.LCD_write_string(MENU_X+24, MENU_Y+4, "L:12-Close", MENU_HIGHLIGHT); Serial.println("L12 close"); }
  }
  
  if (leftValveOpenTime > leftValveCloseTime)
  {
   if (currentTime >= leftValveOpenTime && currentTime <= 2359)
   { digitalWrite(leftValve, HIGH); lcd.LCD_draw_bmp_pixel(0,5, valve_open, 6,8); lcd.LCD_write_string(MENU_X+24, MENU_Y+4, "L:21-Open ", MENU_HIGHLIGHT); Serial.println("L21 open"); }
   if (currentTime >= 0 && currentTime < leftValveCloseTime)
   { digitalWrite(leftValve, HIGH); lcd.LCD_draw_bmp_pixel(0,5, valve_open, 6,8); lcd.LCD_write_string(MENU_X+24, MENU_Y+4, "L:22-Open ", MENU_HIGHLIGHT); Serial.println("L22 open"); }
   if (currentTime >= leftValveCloseTime && currentTime < leftValveOpenTime)
   { digitalWrite(leftValve, LOW); lcd.LCD_draw_bmp_pixel(0,5, valve_closed, 6,8); lcd.LCD_write_string(MENU_X+24, MENU_Y+4, "L:23-Close", MENU_HIGHLIGHT); Serial.println("L23 close"); }   
  }

  if (leftValveOpenTime == leftValveCloseTime)
  { digitalWrite(leftValve, LOW); lcd.LCD_draw_bmp_pixel(0,5, valve_closed, 6,8); lcd.LCD_write_string(MENU_X+24, MENU_Y+4, "L:31-Close", MENU_HIGHLIGHT); Serial.println("L31 close"); }


  // Right Valve
  if (rightValveOpenTime < rightValveCloseTime)
  {
   if (currentTime >= rightValveOpenTime && currentTime < rightValveCloseTime)
   { digitalWrite(rightValve, HIGH); lcd.LCD_draw_bmp_pixel(12,5, valve_open, 6,8); lcd.LCD_write_string(MENU_X+24, MENU_Y+5, "R:11-Open ", MENU_HIGHLIGHT); Serial.println("R11 open"); }
   else
   { digitalWrite(rightValve, LOW); lcd.LCD_draw_bmp_pixel(12,5, valve_closed, 6,8); lcd.LCD_write_string(MENU_X+24, MENU_Y+5, "R:12-Close", MENU_HIGHLIGHT); Serial.println("R12 close"); }
  }
  
  if (rightValveOpenTime > rightValveCloseTime)
  {
   if (currentTime >= rightValveOpenTime && currentTime <= 2359)
   { digitalWrite(rightValve, HIGH); lcd.LCD_draw_bmp_pixel(12,5, valve_open, 6,8); lcd.LCD_write_string(MENU_X+24, MENU_Y+5, "R:21-Open ", MENU_HIGHLIGHT); Serial.println("R21 open"); }
   if (currentTime >= 0 && currentTime < rightValveCloseTime)
   { digitalWrite(rightValve, HIGH); lcd.LCD_draw_bmp_pixel(12,5, valve_open, 6,8); lcd.LCD_write_string(MENU_X+24, MENU_Y+5, "R:22-Open ", MENU_HIGHLIGHT); Serial.println("R22 open"); }
   if (currentTime >= rightValveCloseTime && currentTime < rightValveOpenTime)
   { digitalWrite(rightValve, LOW); lcd.LCD_draw_bmp_pixel(12,5, valve_closed, 6,8); lcd.LCD_write_string(MENU_X+24, MENU_Y+5, "R:23-Close", MENU_HIGHLIGHT); Serial.println("R23 close"); }   
  }

  if (rightValveOpenTime == rightValveCloseTime)
  { digitalWrite(rightValve, LOW); lcd.LCD_draw_bmp_pixel(12,5, valve_closed, 6,8); lcd.LCD_write_string(MENU_X+24, MENU_Y+5, "R:31-Close", MENU_HIGHLIGHT); Serial.println("R31 close"); }

}


// Read clock and store current time and date into the following variables: second, minute, hour, dayOfWeek, day, month and year
void readClock(void)
{
 Wire.beginTransmission(DS1307_I2C_ADDRESS);
 Wire.write(0x00);
 Wire.endTransmission();
 Wire.requestFrom(DS1307_I2C_ADDRESS, 7);
 second = bcdToDec(Wire.read());
 minute = bcdToDec(Wire.read());
 hour = bcdToDec(Wire.read());
 dayOfWeek = bcdToDec(Wire.read());
 day = bcdToDec(Wire.read());
 month = bcdToDec(Wire.read());
 year = bcdToDec(Wire.read());
}


// Debounce ClockMode pushbutton and jump between modes
void SwitchClockMode_right (unsigned char x)
{
 if ((analogRead(joystick) >= 135 && analogRead(joystick) <= 155) || (analogRead(joystick) >= 495 && analogRead(joystick) <= 515))
    { ClockModeFlag = 1; }
 
 if (analogRead(joystick) >= 1003 && analogRead(joystick) <= 1023 && ClockModeFlag == 1)
    { ClockModeFlag=0; gotomain=0; lcd.LCD_clear(); ClockModeState=x; }
}


// Debounce ClockMode pushbutton and jump between modes
void SwitchClockMode_left (unsigned char x)
{
 if (analogRead(joystick) >= 0 && analogRead(joystick) <= 20)
    { ClockModeFlag = 2; }
 
 if (analogRead(joystick) >= 1003 && analogRead(joystick) <= 1023 && ClockModeFlag == 2)
    { ClockModeFlag=0; gotomain=0; lcd.LCD_clear(); ClockModeState=x; }
}


// This function does the following:
// If there is no input, automatically jump to main display when gotomain = gotomaincounter. Change this time by manipulating the gotomaincouter value.
void gotomainfunction(void)
{
 gotomain++; if (gotomain == gotomaincounter) {gotomain=0; lcd.LCD_clear(); ClockModeState=0;}
}


// Set hour
void SetHour(void)
{
  PrintTimeOnLCD();
  lcd.LCD_draw_bmp_pixel(0,1, upbar, 6,8);
  lcd.LCD_draw_bmp_pixel(6,1, upbar, 6,8);
  gotomainfunction();
    
  if(analogRead(joystick) >= 735 && analogRead(joystick) <= 755)
  {  
  gotomain=0;  
  Wire.beginTransmission(DS1307_I2C_ADDRESS);
  Wire.write(0x02);
  Wire.endTransmission();
  Wire.requestFrom(DS1307_I2C_ADDRESS, 1);
  hour = bcdToDec(Wire.read());
  hour++; if (hour == 24){hour=0;}   
  Wire.beginTransmission(DS1307_I2C_ADDRESS);
  Wire.write(0x02);
  Wire.write(decToBcd(hour));
  Wire.endTransmission();
  delay(settingdelay);
  }
  
  if(analogRead(joystick) >= 320 && analogRead(joystick) <= 340)
  {
  gotomain=0;
  Wire.beginTransmission(DS1307_I2C_ADDRESS);
  Wire.write(0x02);
  Wire.endTransmission();
  Wire.requestFrom(DS1307_I2C_ADDRESS, 1);
  hour = bcdToDec(Wire.read());
  hour--; if (hour == -1){hour=23;}   
  Wire.beginTransmission(DS1307_I2C_ADDRESS);
  Wire.write(0x02);
  Wire.write(decToBcd(hour));
  Wire.endTransmission();
  delay(settingdelay);
  } 
  SwitchClockMode_right(2);
  SwitchClockMode_left(0);
}


// Set minute
void SetMinute(void)
{
  PrintTimeOnLCD();
  lcd.LCD_draw_bmp_pixel(18,1, upbar, 6,8);
  lcd.LCD_draw_bmp_pixel(24,1, upbar, 6,8);
  gotomainfunction();

  if(analogRead(joystick) >= 735 && analogRead(joystick) <= 755)
  {   
  gotomain=0;
  Wire.beginTransmission(DS1307_I2C_ADDRESS);
  Wire.write(0x01);
  Wire.endTransmission();
  Wire.requestFrom(DS1307_I2C_ADDRESS, 1);
  minute = bcdToDec(Wire.read());
  minute++; if (minute == 60){minute=0;}   
  Wire.beginTransmission(DS1307_I2C_ADDRESS);
  Wire.write(0x01);
  Wire.write(decToBcd(minute));
  Wire.endTransmission();
  delay(settingdelay);
  }
  
  if(analogRead(joystick) >= 320 && analogRead(joystick) <= 340)
  {   
  gotomain=0; 
  Wire.beginTransmission(DS1307_I2C_ADDRESS);
  Wire.write(0x01);
  Wire.endTransmission();
  Wire.requestFrom(DS1307_I2C_ADDRESS, 1);
  minute = bcdToDec(Wire.read());
  minute--; if (minute == -1){minute=59;}   
  Wire.beginTransmission(DS1307_I2C_ADDRESS);
  Wire.write(0x01);
  Wire.write(decToBcd(minute));
  Wire.endTransmission();
  delay(settingdelay);
  }
  SwitchClockMode_right(3);
  SwitchClockMode_left(1);
}


// Set second
void SetSecond(void)
{
  PrintTimeOnLCD();
  lcd.LCD_draw_bmp_pixel(36,1, upbar, 6,8);
  lcd.LCD_draw_bmp_pixel(42,1, upbar, 6,8);
  gotomainfunction();
  
  if(analogRead(joystick) >= 735 && analogRead(joystick) <= 755)
  {
  gotomain=0;
  Wire.beginTransmission(DS1307_I2C_ADDRESS);
  Wire.write(0x00);
  Wire.endTransmission();
  Wire.requestFrom(DS1307_I2C_ADDRESS, 1);
  second = bcdToDec(Wire.read());
  second++; if (second == 60){second=0;}   
  Wire.beginTransmission(DS1307_I2C_ADDRESS);
  Wire.write(0x00);
  Wire.write(decToBcd(second));
  Wire.endTransmission();
  delay(settingdelay);
  }
  
  if(analogRead(joystick) >= 320 && analogRead(joystick) <= 340)
  {
  gotomain=0;
  Wire.beginTransmission(DS1307_I2C_ADDRESS);
  Wire.write(0x00);
  Wire.endTransmission();
  Wire.requestFrom(DS1307_I2C_ADDRESS, 1);
  second = bcdToDec(Wire.read());
  second--; if (second == -1){second=59;}   
  Wire.beginTransmission(DS1307_I2C_ADDRESS);
  Wire.write(0x00);
  Wire.write(decToBcd(second));
  Wire.endTransmission();
  delay(settingdelay);
  }
  SwitchClockMode_right(4);
  SwitchClockMode_left(2);
}


// Set day
void SetDay(void)
{  
  PrintDateOnLCD();
  lcd.LCD_draw_bmp_pixel(0,2, upbar, 6,8);
  lcd.LCD_draw_bmp_pixel(6,2, upbar, 6,8);
  gotomainfunction();
  
  if(analogRead(joystick) >= 735 && analogRead(joystick) <= 755)
  {
  gotomain=0;  
  Wire.beginTransmission(DS1307_I2C_ADDRESS);
  Wire.write(0x04);
  Wire.endTransmission();
  Wire.requestFrom(DS1307_I2C_ADDRESS, 3);
  day = bcdToDec(Wire.read());
  month = bcdToDec(Wire.read());
  year = bcdToDec(Wire.read());
  day++; 

  if (day == 32 && (month == 1 || month == 3 || month == 5 || month == 7 || month == 8 || month == 10 || month == 12)){day=1;}
  if (day == 31 && (month == 4 || month == 6 || month == 9 || month == 11)){day=1;}
  if (day == 30 && month == 2 && year%4 == 0){day=1;}
  if (day == 29 && month == 2 && year%4 != 0){day=1;}
  
  Wire.beginTransmission(DS1307_I2C_ADDRESS);
  Wire.write(0x04);
  Wire.write(decToBcd(day));
  Wire.endTransmission();
  delay(settingdelay);
  }
 
  if(analogRead(joystick) >= 320 && analogRead(joystick) <= 340)
  {
  gotomain=0;  
  Wire.beginTransmission(DS1307_I2C_ADDRESS);
  Wire.write(0x04);
  Wire.endTransmission();
  Wire.requestFrom(DS1307_I2C_ADDRESS, 3);
  day = bcdToDec(Wire.read());
  month = bcdToDec(Wire.read());
  year = bcdToDec(Wire.read());
  day--;

  if (day == 0 && (month == 1 || month == 3 || month == 5 || month == 7 || month == 8 || month == 10 || month == 12)){day=31;}
  if (day == 0 && (month == 4 || month == 6 || month == 9 || month == 11)){day=30;}
  if (day == 0 && month == 2 && year%4 == 0){day=28;}
  if (day == 0 && month == 2 && year%4 != 0){day=28;}

  Wire.beginTransmission(DS1307_I2C_ADDRESS);
  Wire.write(0x04);
  Wire.write(decToBcd(day));
  Wire.endTransmission();
  delay(settingdelay);
  }
  SwitchClockMode_right(5);
  SwitchClockMode_left(3);
}


// Set month
void SetMonth(void)
{
  PrintDateOnLCD();
  lcd.LCD_draw_bmp_pixel(18,2, upbar, 6,8);
  lcd.LCD_draw_bmp_pixel(24,2, upbar, 6,8);
  lcd.LCD_draw_bmp_pixel(30,2, upbar, 6,8);
  gotomainfunction();
  
  if(analogRead(joystick) >= 735 && analogRead(joystick) <= 755)
  {
  gotomain=0;
  Wire.beginTransmission(DS1307_I2C_ADDRESS);
  Wire.write(0x05);
  Wire.endTransmission();
  Wire.requestFrom(DS1307_I2C_ADDRESS, 1);
  month = bcdToDec(Wire.read()); 

       if (month == 1 && day == 31){month=3;}
  else if (month == 1 && day == 30){month=3;}  
  else if (month == 1 && day == 29 && year%4 != 0){month=3;}
  else if (month == 1 && day == 29 && year%4 == 0){month=2;}  
  else if (month == 1 && day <= 28){month=2;}  
  else if (month == 2){month=3;}
  else if (month == 3 && day == 31){month=5;}
  else if (month == 3 && day <= 30){month=4;}  
  else if (month == 4){month=5;}
  else if (month == 5 && day == 31){month=7;}
  else if (month == 5 && day <= 30){month=6;}  
  else if (month == 6){month=7;}
  else if (month == 7){month=8;}
  else if (month == 8 && day == 31){month=10;}  
  else if (month == 8 && day <= 30){month=9;} 
  else if (month == 9){month=10;}
  else if (month == 10 && day == 31){month=12;}  
  else if (month == 10 && day <= 30){month=11;} 
  else if (month == 11){month=12;}
  else if (month == 12){month=1;}
 
  Wire.beginTransmission(DS1307_I2C_ADDRESS);
  Wire.write(0x05);
  Wire.write(decToBcd(month));
  Wire.endTransmission();
  delay(settingdelay);
  }
  
  if(analogRead(joystick) >= 320 && analogRead(joystick) <= 340)
  {
  gotomain=0;
  Wire.beginTransmission(DS1307_I2C_ADDRESS);
  Wire.write(0x05);
  Wire.endTransmission();
  Wire.requestFrom(DS1307_I2C_ADDRESS, 1);
  month = bcdToDec(Wire.read()); 
       
       if (month == 1){month=12;}
  else if (month == 2){month=1;}  
  else if (month == 3 && day == 31){month=1;}
  else if (month == 3 && day == 30){month=1;}
  else if (month == 3 && day == 29 && year%4 != 0){month=1;}
  else if (month == 3 && day == 29 && year%4 == 0){month=2;}
  else if (month == 3 && day <= 28){month=2;}
  else if (month == 4){month=3;}
  else if (month == 5 && day == 31){month=3;}
  else if (month == 5 && day <= 30){month=4;}  
  else if (month == 6){month=5;}
  else if (month == 7 && day == 31){month=5;}
  else if (month == 7 && day <= 30){month=6;}  
  else if (month == 8){month=7;}  
  else if (month == 9){month=8;}
  else if (month == 10 && day == 31){month=8;}
  else if (month == 10 && day <= 30){month=9;} 
  else if (month == 11){month=10;}
  else if (month == 12 && day == 31){month=10;}
  else if (month == 12 && day <= 30){month=11;} 
  
  Wire.beginTransmission(DS1307_I2C_ADDRESS);
  Wire.write(0x05);
  Wire.write(decToBcd(month));
  Wire.endTransmission();
  delay(settingdelay);
  }
  SwitchClockMode_right(6);
  SwitchClockMode_left(4);
}


// Set year
void SetYear(void)
{
  PrintDateOnLCD();
  lcd.LCD_draw_bmp_pixel(42,2, upbar, 6,8);
  lcd.LCD_draw_bmp_pixel(48,2, upbar, 6,8);
  lcd.LCD_draw_bmp_pixel(54,2, upbar, 6,8);
  lcd.LCD_draw_bmp_pixel(60,2, upbar, 6,8);
  gotomainfunction();
  
  if(analogRead(joystick) >= 735 && analogRead(joystick) <= 755)
  {
  gotomain=0;
  Wire.beginTransmission(DS1307_I2C_ADDRESS);
  Wire.write(0x06);
  Wire.endTransmission();
  Wire.requestFrom(DS1307_I2C_ADDRESS, 1);
  year = bcdToDec(Wire.read());
  
  if (day == 29 && year%4 == 0 && month == 2){year=year+4;}
  else year++; 
  if (year >= 31){year=16;}
  
  Wire.beginTransmission(DS1307_I2C_ADDRESS);
  Wire.write(0x06);
  Wire.write(decToBcd(year));
  Wire.endTransmission();
  delay(settingdelay);
  }
  
  if(analogRead(joystick) >= 320 && analogRead(joystick) <= 340)
  {
  gotomain=0;
  Wire.beginTransmission(DS1307_I2C_ADDRESS);
  Wire.write(0x06);
  Wire.endTransmission();
  Wire.requestFrom(DS1307_I2C_ADDRESS, 1);
  year = bcdToDec(Wire.read());
 
  if (day == 29 && year%4 == 0 && month == 2){year=year-4;}
  else year--;
  if (year <= 15){year=30;}

  Wire.beginTransmission(DS1307_I2C_ADDRESS);
  Wire.write(0x06);
  Wire.write(decToBcd(year));
  Wire.endTransmission();
  delay(settingdelay);
  }
  SwitchClockMode_right(7);
  SwitchClockMode_left(5);
}


// Set Main Valve Open Hour
void SetMainValveOpenHour(void)
{   
  mainValveOpenHour = EEPROM.read(mainValveOpenHourMemoryBank);
  
  lcd.LCD_write_string(MENU_X, MENU_Y, "Main Open", MENU_NORMAL);
  lcd.LCD_write_string(MENU_X+12, MENU_Y+1, ":", MENU_NORMAL);
  lcd.LCD_draw_bmp_pixel(0,2, upbar, 6,8);
  lcd.LCD_draw_bmp_pixel(6,2, upbar, 6,8);
  
  if (mainValveOpenHour >= 10)
  {
  itoa(mainValveOpenHour,string_mainValveOpenHour,10);
  lcd.LCD_write_string(MENU_X, MENU_Y+1, string_mainValveOpenHour, MENU_NORMAL);
  }
  
  if (mainValveOpenHour < 10)
  {
  itoa(mainValveOpenHour,string_mainValveOpenHour,10);
  lcd.LCD_write_string(MENU_X, MENU_Y+1, "0", MENU_NORMAL);
  lcd.LCD_write_string(MENU_X+6, MENU_Y+1, string_mainValveOpenHour, MENU_NORMAL);
  }

  if (mainValveOpenMinute >= 10)
  {
  itoa(mainValveOpenMinute,string_mainValveOpenMinute,10);
  lcd.LCD_write_string(MENU_X+18, MENU_Y+1, string_mainValveOpenMinute, MENU_NORMAL);
  }
  
  if (mainValveOpenMinute < 10)
  {
  itoa(mainValveOpenMinute,string_mainValveOpenMinute,10);
  lcd.LCD_write_string(MENU_X+18, MENU_Y+1, "0", MENU_NORMAL);
  lcd.LCD_write_string(MENU_X+24, MENU_Y+1, string_mainValveOpenMinute, MENU_NORMAL);
  }
  
  gotomainfunction();
  
  if(analogRead(joystick) >= 735 && analogRead(joystick) <= 755)
  {
  gotomain=0;
  mainValveOpenHour++;
  if (mainValveOpenHour >= 24){mainValveOpenHour=0;}
  EEPROM.write(mainValveOpenHourMemoryBank, mainValveOpenHour);
  delay(settingdelay);
  }

  if(analogRead(joystick) >= 320 && analogRead(joystick) <= 340)
  {
  gotomain=0;
  mainValveOpenHour--;
  if (mainValveOpenHour <= -1){mainValveOpenHour=23;}
  EEPROM.write(mainValveOpenHourMemoryBank, mainValveOpenHour);
  delay(settingdelay);
  }
  SwitchClockMode_right(8);
  SwitchClockMode_left(6);
}


// Set Main Valve Open Minute
void SetMainValveOpenMinute(void)
{
  mainValveOpenMinute = EEPROM.read(mainValveOpenMinuteMemoryBank);
  
  lcd.LCD_write_string(MENU_X, MENU_Y, "Main Open", MENU_NORMAL);
  lcd.LCD_write_string(MENU_X+12, MENU_Y+1, ":", MENU_NORMAL);
  lcd.LCD_draw_bmp_pixel(18,2, upbar, 6,8);
  lcd.LCD_draw_bmp_pixel(24,2, upbar, 6,8);

  if (mainValveOpenHour >= 10)
  {
  itoa(mainValveOpenHour,string_mainValveOpenHour,10);
  lcd.LCD_write_string(MENU_X, MENU_Y+1, string_mainValveOpenHour, MENU_NORMAL);
  }
  
  if (mainValveOpenHour < 10)
  {
  itoa(mainValveOpenHour,string_mainValveOpenHour,10);
  lcd.LCD_write_string(MENU_X, MENU_Y+1, "0", MENU_NORMAL);
  lcd.LCD_write_string(MENU_X+6, MENU_Y+1, string_mainValveOpenHour, MENU_NORMAL);
  }
  
  if (mainValveOpenMinute >= 10)
  {
  itoa(mainValveOpenMinute,string_mainValveOpenMinute,10);
  lcd.LCD_write_string(MENU_X+18, MENU_Y+1, string_mainValveOpenMinute, MENU_NORMAL);
  }
  
  if (mainValveOpenMinute < 10)
  {
  itoa(mainValveOpenMinute,string_mainValveOpenMinute,10);
  lcd.LCD_write_string(MENU_X+18, MENU_Y+1, "0", MENU_NORMAL);
  lcd.LCD_write_string(MENU_X+24, MENU_Y+1, string_mainValveOpenMinute, MENU_NORMAL);
  }
  
  gotomainfunction();
  
  if(analogRead(joystick) >= 735 && analogRead(joystick) <= 755)
  {
  gotomain=0;
  mainValveOpenMinute++;
  if (mainValveOpenMinute >= 60){mainValveOpenMinute=0;}
  EEPROM.write(mainValveOpenMinuteMemoryBank, mainValveOpenMinute);
  delay(settingdelay);
  }

  if(analogRead(joystick) >= 320 && analogRead(joystick) <= 340)
  {
  gotomain=0;
  mainValveOpenMinute--;
  if (mainValveOpenMinute <= -1){mainValveOpenMinute=59;}
  EEPROM.write(mainValveOpenMinuteMemoryBank, mainValveOpenMinute);
  delay(settingdelay);
  }
  SwitchClockMode_right(9);
  SwitchClockMode_left(7);
}


// Set Main Valve Close Hour
void SetMainValveCloseHour(void)
{   
  mainValveCloseHour = EEPROM.read(mainValveCloseHourMemoryBank);
  
  lcd.LCD_write_string(MENU_X, MENU_Y, "Main Close", MENU_NORMAL);
  lcd.LCD_write_string(MENU_X+12, MENU_Y+1, ":", MENU_NORMAL);
  lcd.LCD_draw_bmp_pixel(0,2, upbar, 6,8);
  lcd.LCD_draw_bmp_pixel(6,2, upbar, 6,8);
  
  if (mainValveCloseHour >= 10)
  {
  itoa(mainValveCloseHour,string_mainValveCloseHour,10);
  lcd.LCD_write_string(MENU_X, MENU_Y+1, string_mainValveCloseHour, MENU_NORMAL);
  }
  
  if (mainValveCloseHour < 10)
  {
  itoa(mainValveCloseHour,string_mainValveCloseHour,10);
  lcd.LCD_write_string(MENU_X, MENU_Y+1, "0", MENU_NORMAL);
  lcd.LCD_write_string(MENU_X+6, MENU_Y+1, string_mainValveCloseHour, MENU_NORMAL);
  }

  if (mainValveCloseMinute >= 10)
  {
  itoa(mainValveCloseMinute,string_mainValveCloseMinute,10);
  lcd.LCD_write_string(MENU_X+18, MENU_Y+1, string_mainValveCloseMinute, MENU_NORMAL);
  }
  
  if (mainValveCloseMinute < 10)
  {
  itoa(mainValveCloseMinute,string_mainValveCloseMinute,10);
  lcd.LCD_write_string(MENU_X+18, MENU_Y+1, "0", MENU_NORMAL);
  lcd.LCD_write_string(MENU_X+24, MENU_Y+1, string_mainValveCloseMinute, MENU_NORMAL);
  }
  
  gotomainfunction();
  
  if(analogRead(joystick) >= 735 && analogRead(joystick) <= 755)
  {
  gotomain=0;
  mainValveCloseHour++;
  if (mainValveCloseHour >= 24){mainValveCloseHour=0;}
  EEPROM.write(mainValveCloseHourMemoryBank, mainValveCloseHour);
  delay(settingdelay);
  }

  if(analogRead(joystick) >= 320 && analogRead(joystick) <= 340)
  {
  gotomain=0;
  mainValveCloseHour--;
  if (mainValveCloseHour <= -1){mainValveCloseHour=23;}
  EEPROM.write(mainValveCloseHourMemoryBank, mainValveCloseHour);
  delay(settingdelay);
  }
  SwitchClockMode_right(10);
  SwitchClockMode_left(8);
}


// Set Main Valve Close Minute
void SetMainValveCloseMinute(void)
{
  mainValveCloseMinute = EEPROM.read(mainValveCloseMinuteMemoryBank);
  
  lcd.LCD_write_string(MENU_X, MENU_Y, "Main Close", MENU_NORMAL);
  lcd.LCD_write_string(MENU_X+12, MENU_Y+1, ":", MENU_NORMAL);
  lcd.LCD_draw_bmp_pixel(18,2, upbar, 6,8);
  lcd.LCD_draw_bmp_pixel(24,2, upbar, 6,8);

  if (mainValveCloseHour >= 10)
  {
  itoa(mainValveCloseHour,string_mainValveCloseHour,10);
  lcd.LCD_write_string(MENU_X, MENU_Y+1, string_mainValveCloseHour, MENU_NORMAL);
  }
  
  if (mainValveCloseHour < 10)
  {
  itoa(mainValveCloseHour,string_mainValveCloseHour,10);
  lcd.LCD_write_string(MENU_X, MENU_Y+1, "0", MENU_NORMAL);
  lcd.LCD_write_string(MENU_X+6, MENU_Y+1, string_mainValveCloseHour, MENU_NORMAL);
  }
  
  if (mainValveCloseMinute >= 10)
  {
  itoa(mainValveCloseMinute,string_mainValveCloseMinute,10);
  lcd.LCD_write_string(MENU_X+18, MENU_Y+1, string_mainValveCloseMinute, MENU_NORMAL);
  }
  
  if (mainValveCloseMinute < 10)
  {
  itoa(mainValveCloseMinute,string_mainValveCloseMinute,10);
  lcd.LCD_write_string(MENU_X+18, MENU_Y+1, "0", MENU_NORMAL);
  lcd.LCD_write_string(MENU_X+24, MENU_Y+1, string_mainValveCloseMinute, MENU_NORMAL);
  }
  
  gotomainfunction();
  
  if(analogRead(joystick) >= 735 && analogRead(joystick) <= 755)
  {
  gotomain=0;
  mainValveCloseMinute++;
  if (mainValveCloseMinute >= 60){mainValveCloseMinute=0;}
  EEPROM.write(mainValveCloseMinuteMemoryBank, mainValveCloseMinute);
  delay(settingdelay);
  }

  if(analogRead(joystick) >= 320 && analogRead(joystick) <= 340)
  {
  gotomain=0;
  mainValveCloseMinute--;
  if (mainValveCloseMinute <= -1){mainValveCloseMinute=59;}
  EEPROM.write(mainValveCloseMinuteMemoryBank, mainValveCloseMinute);
  delay(settingdelay);
  }
  SwitchClockMode_right(11);
  SwitchClockMode_left(9);
}


// Set Left Valve Open Hour
void SetLeftValveOpenHour(void)
{   
  leftValveOpenHour = EEPROM.read(leftValveOpenHourMemoryBank);
  
  lcd.LCD_write_string(MENU_X, MENU_Y, "Left Open", MENU_NORMAL);
  lcd.LCD_write_string(MENU_X+12, MENU_Y+1, ":", MENU_NORMAL);
  lcd.LCD_draw_bmp_pixel(0,2, upbar, 6,8);
  lcd.LCD_draw_bmp_pixel(6,2, upbar, 6,8);
  
  if (leftValveOpenHour >= 10)
  {
  itoa(leftValveOpenHour,string_leftValveOpenHour,10);
  lcd.LCD_write_string(MENU_X, MENU_Y+1, string_leftValveOpenHour, MENU_NORMAL);
  }
  
  if (leftValveOpenHour < 10)
  {
  itoa(leftValveOpenHour,string_leftValveOpenHour,10);
  lcd.LCD_write_string(MENU_X, MENU_Y+1, "0", MENU_NORMAL);
  lcd.LCD_write_string(MENU_X+6, MENU_Y+1, string_leftValveOpenHour, MENU_NORMAL);
  }

  if (leftValveOpenMinute >= 10)
  {
  itoa(leftValveOpenMinute,string_leftValveOpenMinute,10);
  lcd.LCD_write_string(MENU_X+18, MENU_Y+1, string_leftValveOpenMinute, MENU_NORMAL);
  }
  
  if (leftValveOpenMinute < 10)
  {
  itoa(leftValveOpenMinute,string_leftValveOpenMinute,10);
  lcd.LCD_write_string(MENU_X+18, MENU_Y+1, "0", MENU_NORMAL);
  lcd.LCD_write_string(MENU_X+24, MENU_Y+1, string_leftValveOpenMinute, MENU_NORMAL);
  }
  
  gotomainfunction();
  
  if(analogRead(joystick) >= 735 && analogRead(joystick) <= 755)
  {
  gotomain=0;
  leftValveOpenHour++;
  if (leftValveOpenHour >= 24){leftValveOpenHour=0;}
  EEPROM.write(leftValveOpenHourMemoryBank, leftValveOpenHour);
  delay(settingdelay);
  }

  if(analogRead(joystick) >= 320 && analogRead(joystick) <= 340)
  {
  gotomain=0;
  leftValveOpenHour--;
  if (leftValveOpenHour <= -1){leftValveOpenHour=23;}
  EEPROM.write(leftValveOpenHourMemoryBank, leftValveOpenHour);
  delay(settingdelay);
  }
  SwitchClockMode_right(12);
  SwitchClockMode_left(10);
}


// Set Left Valve Open Minute
void SetLeftValveOpenMinute(void)
{
  leftValveOpenMinute = EEPROM.read(leftValveOpenMinuteMemoryBank);
  
  lcd.LCD_write_string(MENU_X, MENU_Y, "Left Open", MENU_NORMAL);
  lcd.LCD_write_string(MENU_X+12, MENU_Y+1, ":", MENU_NORMAL);
  lcd.LCD_draw_bmp_pixel(18,2, upbar, 6,8);
  lcd.LCD_draw_bmp_pixel(24,2, upbar, 6,8);

  if (leftValveOpenHour >= 10)
  {
  itoa(leftValveOpenHour,string_leftValveOpenHour,10);
  lcd.LCD_write_string(MENU_X, MENU_Y+1, string_leftValveOpenHour, MENU_NORMAL);
  }
  
  if (leftValveOpenHour < 10)
  {
  itoa(leftValveOpenHour,string_leftValveOpenHour,10);
  lcd.LCD_write_string(MENU_X, MENU_Y+1, "0", MENU_NORMAL);
  lcd.LCD_write_string(MENU_X+6, MENU_Y+1, string_leftValveOpenHour, MENU_NORMAL);
  }
  
  if (leftValveOpenMinute >= 10)
  {
  itoa(leftValveOpenMinute,string_leftValveOpenMinute,10);
  lcd.LCD_write_string(MENU_X+18, MENU_Y+1, string_leftValveOpenMinute, MENU_NORMAL);
  }
  
  if (leftValveOpenMinute < 10)
  {
  itoa(leftValveOpenMinute,string_leftValveOpenMinute,10);
  lcd.LCD_write_string(MENU_X+18, MENU_Y+1, "0", MENU_NORMAL);
  lcd.LCD_write_string(MENU_X+24, MENU_Y+1, string_leftValveOpenMinute, MENU_NORMAL);
  }
  
  gotomainfunction();
  
  if(analogRead(joystick) >= 735 && analogRead(joystick) <= 755)
  {
  gotomain=0;
  leftValveOpenMinute++;
  if (leftValveOpenMinute >= 60){leftValveOpenMinute=0;}
  EEPROM.write(leftValveOpenMinuteMemoryBank, leftValveOpenMinute);
  delay(settingdelay);
  }

  if(analogRead(joystick) >= 320 && analogRead(joystick) <= 340)
  {
  gotomain=0;
  leftValveOpenMinute--;
  if (leftValveOpenMinute <= -1){leftValveOpenMinute=59;}
  EEPROM.write(leftValveOpenMinuteMemoryBank, leftValveOpenMinute);
  delay(settingdelay);
  }
  SwitchClockMode_right(13);
  SwitchClockMode_left(11);
}


// Set Left Valve Close Hour
void SetLeftValveCloseHour(void)
{   
  leftValveCloseHour = EEPROM.read(leftValveCloseHourMemoryBank);
  
  lcd.LCD_write_string(MENU_X, MENU_Y, "Left Close", MENU_NORMAL);
  lcd.LCD_write_string(MENU_X+12, MENU_Y+1, ":", MENU_NORMAL);
  lcd.LCD_draw_bmp_pixel(0,2, upbar, 6,8);
  lcd.LCD_draw_bmp_pixel(6,2, upbar, 6,8);
  
  if (leftValveCloseHour >= 10)
  {
  itoa(leftValveCloseHour,string_leftValveCloseHour,10);
  lcd.LCD_write_string(MENU_X, MENU_Y+1, string_leftValveCloseHour, MENU_NORMAL);
  }
  
  if (leftValveCloseHour < 10)
  {
  itoa(leftValveCloseHour,string_leftValveCloseHour,10);
  lcd.LCD_write_string(MENU_X, MENU_Y+1, "0", MENU_NORMAL);
  lcd.LCD_write_string(MENU_X+6, MENU_Y+1, string_leftValveCloseHour, MENU_NORMAL);
  }

  if (leftValveCloseMinute >= 10)
  {
  itoa(leftValveCloseMinute,string_leftValveCloseMinute,10);
  lcd.LCD_write_string(MENU_X+18, MENU_Y+1, string_leftValveCloseMinute, MENU_NORMAL);
  }
  
  if (leftValveCloseMinute < 10)
  {
  itoa(leftValveCloseMinute,string_leftValveCloseMinute,10);
  lcd.LCD_write_string(MENU_X+18, MENU_Y+1, "0", MENU_NORMAL);
  lcd.LCD_write_string(MENU_X+24, MENU_Y+1, string_leftValveCloseMinute, MENU_NORMAL);
  }
  
  gotomainfunction();
  
  if(analogRead(joystick) >= 735 && analogRead(joystick) <= 755)
  {
  gotomain=0;
  leftValveCloseHour++;
  if (leftValveCloseHour >= 24){leftValveCloseHour=0;}
  EEPROM.write(leftValveCloseHourMemoryBank, leftValveCloseHour);
  delay(settingdelay);
  }

  if(analogRead(joystick) >= 320 && analogRead(joystick) <= 340)
  {
  gotomain=0;
  leftValveCloseHour--;
  if (leftValveCloseHour <= -1){leftValveCloseHour=23;}
  EEPROM.write(leftValveCloseHourMemoryBank, leftValveCloseHour);
  delay(settingdelay);
  }
  SwitchClockMode_right(14);
  SwitchClockMode_left(12);
}


// Set Left Valve Close Minute
void SetLeftValveCloseMinute(void)
{
  leftValveCloseMinute = EEPROM.read(leftValveCloseMinuteMemoryBank);
  
  lcd.LCD_write_string(MENU_X, MENU_Y, "Left Close", MENU_NORMAL);
  lcd.LCD_write_string(MENU_X+12, MENU_Y+1, ":", MENU_NORMAL);
  lcd.LCD_draw_bmp_pixel(18,2, upbar, 6,8);
  lcd.LCD_draw_bmp_pixel(24,2, upbar, 6,8);

  if (leftValveCloseHour >= 10)
  {
  itoa(leftValveCloseHour,string_leftValveCloseHour,10);
  lcd.LCD_write_string(MENU_X, MENU_Y+1, string_leftValveCloseHour, MENU_NORMAL);
  }
  
  if (leftValveCloseHour < 10)
  {
  itoa(leftValveCloseHour,string_leftValveCloseHour,10);
  lcd.LCD_write_string(MENU_X, MENU_Y+1, "0", MENU_NORMAL);
  lcd.LCD_write_string(MENU_X+6, MENU_Y+1, string_leftValveCloseHour, MENU_NORMAL);
  }
  
  if (leftValveCloseMinute >= 10)
  {
  itoa(leftValveCloseMinute,string_leftValveCloseMinute,10);
  lcd.LCD_write_string(MENU_X+18, MENU_Y+1, string_leftValveCloseMinute, MENU_NORMAL);
  }
  
  if (leftValveCloseMinute < 10)
  {
  itoa(leftValveCloseMinute,string_leftValveCloseMinute,10);
  lcd.LCD_write_string(MENU_X+18, MENU_Y+1, "0", MENU_NORMAL);
  lcd.LCD_write_string(MENU_X+24, MENU_Y+1, string_leftValveCloseMinute, MENU_NORMAL);
  }
  
  gotomainfunction();
  
  if(analogRead(joystick) >= 735 && analogRead(joystick) <= 755)
  {
  gotomain=0;
  leftValveCloseMinute++;
  if (leftValveCloseMinute >= 60){leftValveCloseMinute=0;}
  EEPROM.write(leftValveCloseMinuteMemoryBank, leftValveCloseMinute);
  delay(settingdelay);
  }

  if(analogRead(joystick) >= 320 && analogRead(joystick) <= 340)
  {
  gotomain=0;
  leftValveCloseMinute--;
  if (leftValveCloseMinute <= -1){leftValveCloseMinute=59;}
  EEPROM.write(leftValveCloseMinuteMemoryBank, leftValveCloseMinute);
  delay(settingdelay);
  }
  SwitchClockMode_right(15);
  SwitchClockMode_left(13);
}


// Set Right Valve Open Hour
void SetRightValveOpenHour(void)
{   
  rightValveOpenHour = EEPROM.read(rightValveOpenHourMemoryBank);
  
  lcd.LCD_write_string(MENU_X, MENU_Y, "Right Open", MENU_NORMAL);
  lcd.LCD_write_string(MENU_X+12, MENU_Y+1, ":", MENU_NORMAL);
  lcd.LCD_draw_bmp_pixel(0,2, upbar, 6,8);
  lcd.LCD_draw_bmp_pixel(6,2, upbar, 6,8);
  
  if (rightValveOpenHour >= 10)
  {
  itoa(rightValveOpenHour,string_rightValveOpenHour,10);
  lcd.LCD_write_string(MENU_X, MENU_Y+1, string_rightValveOpenHour, MENU_NORMAL);
  }
  
  if (rightValveOpenHour < 10)
  {
  itoa(rightValveOpenHour,string_rightValveOpenHour,10);
  lcd.LCD_write_string(MENU_X, MENU_Y+1, "0", MENU_NORMAL);
  lcd.LCD_write_string(MENU_X+6, MENU_Y+1, string_rightValveOpenHour, MENU_NORMAL);
  }

  if (rightValveOpenMinute >= 10)
  {
  itoa(rightValveOpenMinute,string_rightValveOpenMinute,10);
  lcd.LCD_write_string(MENU_X+18, MENU_Y+1, string_rightValveOpenMinute, MENU_NORMAL);
  }
  
  if (rightValveOpenMinute < 10)
  {
  itoa(rightValveOpenMinute,string_rightValveOpenMinute,10);
  lcd.LCD_write_string(MENU_X+18, MENU_Y+1, "0", MENU_NORMAL);
  lcd.LCD_write_string(MENU_X+24, MENU_Y+1, string_rightValveOpenMinute, MENU_NORMAL);
  }
  
  gotomainfunction();
  
  if(analogRead(joystick) >= 735 && analogRead(joystick) <= 755)
  {
  gotomain=0;
  rightValveOpenHour++;
  if (rightValveOpenHour >= 24){rightValveOpenHour=0;}
  EEPROM.write(rightValveOpenHourMemoryBank, rightValveOpenHour);
  delay(settingdelay);
  }

  if(analogRead(joystick) >= 320 && analogRead(joystick) <= 340)
  {
  gotomain=0;
  rightValveOpenHour--;
  if (rightValveOpenHour <= -1){rightValveOpenHour=23;}
  EEPROM.write(rightValveOpenHourMemoryBank, rightValveOpenHour);
  delay(settingdelay);
  }
  SwitchClockMode_right(16);
  SwitchClockMode_left(14);
}


// Set Right Valve Open Minute
void SetRightValveOpenMinute(void)
{
  rightValveOpenMinute = EEPROM.read(rightValveOpenMinuteMemoryBank);
  
  lcd.LCD_write_string(MENU_X, MENU_Y, "Right Open", MENU_NORMAL);
  lcd.LCD_write_string(MENU_X+12, MENU_Y+1, ":", MENU_NORMAL);
  lcd.LCD_draw_bmp_pixel(18,2, upbar, 6,8);
  lcd.LCD_draw_bmp_pixel(24,2, upbar, 6,8);

  if (rightValveOpenHour >= 10)
  {
  itoa(rightValveOpenHour,string_rightValveOpenHour,10);
  lcd.LCD_write_string(MENU_X, MENU_Y+1, string_rightValveOpenHour, MENU_NORMAL);
  }
  
  if (rightValveOpenHour < 10)
  {
  itoa(rightValveOpenHour,string_rightValveOpenHour,10);
  lcd.LCD_write_string(MENU_X, MENU_Y+1, "0", MENU_NORMAL);
  lcd.LCD_write_string(MENU_X+6, MENU_Y+1, string_rightValveOpenHour, MENU_NORMAL);
  }
  
  if (rightValveOpenMinute >= 10)
  {
  itoa(rightValveOpenMinute,string_rightValveOpenMinute,10);
  lcd.LCD_write_string(MENU_X+18, MENU_Y+1, string_rightValveOpenMinute, MENU_NORMAL);
  }
  
  if (rightValveOpenMinute < 10)
  {
  itoa(rightValveOpenMinute,string_rightValveOpenMinute,10);
  lcd.LCD_write_string(MENU_X+18, MENU_Y+1, "0", MENU_NORMAL);
  lcd.LCD_write_string(MENU_X+24, MENU_Y+1, string_rightValveOpenMinute, MENU_NORMAL);
  }
  
  gotomainfunction();
  
  if(analogRead(joystick) >= 735 && analogRead(joystick) <= 755)
  {
  gotomain=0;
  rightValveOpenMinute++;
  if (rightValveOpenMinute >= 60){rightValveOpenMinute=0;}
  EEPROM.write(rightValveOpenMinuteMemoryBank, rightValveOpenMinute);
  delay(settingdelay);
  }

  if(analogRead(joystick) >= 320 && analogRead(joystick) <= 340)
  {
  gotomain=0;
  rightValveOpenMinute--;
  if (rightValveOpenMinute <= -1){rightValveOpenMinute=59;}
  EEPROM.write(rightValveOpenMinuteMemoryBank, rightValveOpenMinute);
  delay(settingdelay);
  }
  SwitchClockMode_right(17);
  SwitchClockMode_left(15);
}


// Set Right Valve Close Hour
void SetRightValveCloseHour(void)
{   
  rightValveCloseHour = EEPROM.read(rightValveCloseHourMemoryBank);
  
  lcd.LCD_write_string(MENU_X, MENU_Y, "Right Close", MENU_NORMAL);
  lcd.LCD_write_string(MENU_X+12, MENU_Y+1, ":", MENU_NORMAL);
  lcd.LCD_draw_bmp_pixel(0,2, upbar, 6,8);
  lcd.LCD_draw_bmp_pixel(6,2, upbar, 6,8);
  
  if (rightValveCloseHour >= 10)
  {
  itoa(rightValveCloseHour,string_rightValveCloseHour,10);
  lcd.LCD_write_string(MENU_X, MENU_Y+1, string_rightValveCloseHour, MENU_NORMAL);
  }
  
  if (rightValveCloseHour < 10)
  {
  itoa(rightValveCloseHour,string_rightValveCloseHour,10);
  lcd.LCD_write_string(MENU_X, MENU_Y+1, "0", MENU_NORMAL);
  lcd.LCD_write_string(MENU_X+6, MENU_Y+1, string_rightValveCloseHour, MENU_NORMAL);
  }

  if (rightValveCloseMinute >= 10)
  {
  itoa(rightValveCloseMinute,string_rightValveCloseMinute,10);
  lcd.LCD_write_string(MENU_X+18, MENU_Y+1, string_rightValveCloseMinute, MENU_NORMAL);
  }
  
  if (rightValveCloseMinute < 10)
  {
  itoa(rightValveCloseMinute,string_rightValveCloseMinute,10);
  lcd.LCD_write_string(MENU_X+18, MENU_Y+1, "0", MENU_NORMAL);
  lcd.LCD_write_string(MENU_X+24, MENU_Y+1, string_rightValveCloseMinute, MENU_NORMAL);
  }
  
  gotomainfunction();
  
  if(analogRead(joystick) >= 735 && analogRead(joystick) <= 755)
  {
  gotomain=0;
  rightValveCloseHour++;
  if (rightValveCloseHour >= 24){rightValveCloseHour=0;}
  EEPROM.write(rightValveCloseHourMemoryBank, rightValveCloseHour);
  delay(settingdelay);
  }

  if(analogRead(joystick) >= 320 && analogRead(joystick) <= 340)
  {
  gotomain=0;
  rightValveCloseHour--;
  if (rightValveCloseHour <= -1){rightValveCloseHour=23;}
  EEPROM.write(rightValveCloseHourMemoryBank, rightValveCloseHour);
  delay(settingdelay);
  }
  SwitchClockMode_right(18);
  SwitchClockMode_left(16);
}


// Set Right Valve Close Minute
void SetRightValveCloseMinute(void)
{
  rightValveCloseMinute = EEPROM.read(rightValveCloseMinuteMemoryBank);
  
  lcd.LCD_write_string(MENU_X, MENU_Y, "Right Close", MENU_NORMAL);
  lcd.LCD_write_string(MENU_X+12, MENU_Y+1, ":", MENU_NORMAL);
  lcd.LCD_draw_bmp_pixel(18,2, upbar, 6,8);
  lcd.LCD_draw_bmp_pixel(24,2, upbar, 6,8);

  if (rightValveCloseHour >= 10)
  {
  itoa(rightValveCloseHour,string_rightValveCloseHour,10);
  lcd.LCD_write_string(MENU_X, MENU_Y+1, string_rightValveCloseHour, MENU_NORMAL);
  }
  
  if (rightValveCloseHour < 10)
  {
  itoa(rightValveCloseHour,string_rightValveCloseHour,10);
  lcd.LCD_write_string(MENU_X, MENU_Y+1, "0", MENU_NORMAL);
  lcd.LCD_write_string(MENU_X+6, MENU_Y+1, string_rightValveCloseHour, MENU_NORMAL);
  }
  
  if (rightValveCloseMinute >= 10)
  {
  itoa(rightValveCloseMinute,string_rightValveCloseMinute,10);
  lcd.LCD_write_string(MENU_X+18, MENU_Y+1, string_rightValveCloseMinute, MENU_NORMAL);
  }
  
  if (rightValveCloseMinute < 10)
  {
  itoa(rightValveCloseMinute,string_rightValveCloseMinute,10);
  lcd.LCD_write_string(MENU_X+18, MENU_Y+1, "0", MENU_NORMAL);
  lcd.LCD_write_string(MENU_X+24, MENU_Y+1, string_rightValveCloseMinute, MENU_NORMAL);
  }
  
  gotomainfunction();
  
  if(analogRead(joystick) >= 735 && analogRead(joystick) <= 755)
  {
  gotomain=0;
  rightValveCloseMinute++;
  if (rightValveCloseMinute >= 60){rightValveCloseMinute=0;}
  EEPROM.write(rightValveCloseMinuteMemoryBank, rightValveCloseMinute);
  delay(settingdelay);
  }

  if(analogRead(joystick) >= 320 && analogRead(joystick) <= 340)
  {
  gotomain=0;
  rightValveCloseMinute--;
  if (rightValveCloseMinute <= -1){rightValveCloseMinute=59;}
  EEPROM.write(rightValveCloseMinuteMemoryBank, rightValveCloseMinute);
  delay(settingdelay);
  }
  SwitchClockMode_right(0);
  SwitchClockMode_left(17);
}


// Initialize serial communication
void initSerialCommunication(void)
{
 Serial.print(F("Serial communication: "));
  if (!Serial)
   {Serial.println(F("Failed")); delay(1000); initSerialCommunication();}
  else Serial.println(F("OK"));
}


// Initialize Real Time Clock
void initRTC(void)
{
 //RTC.adjust(DateTime(__DATE__, __TIME__)); // This line sets the RTC to the date and time this sketch was compiled.
 Serial.print(F("RTC: "));
  if (!RTC.isrunning()) // If the clock is not running, execute the following code.
   {
    Serial.print(F("Not Running! Restarting the RTC. "));
    // Starts ticking the clock
    Wire.beginTransmission(DS1307_I2C_ADDRESS);
    Wire.write(0x00); // Move pointer to 0x00 byte address.
    Wire.write(0x00); // Sends 0x00. The whole byte is set to zero (0x00). This also means seconds will reset!! Unless you use a mask -> homework :)
    Wire.endTransmission();
    
    // Following line sets the RTC to the date & time to: 2015 January 01 - 00:00:00
    RTC.adjust(DateTime(2015,1,1, 0,0,0)); // Sequence: year, month, day,  hour, minute, second.
   }
  if (RTC.isrunning())
   {Serial.println(F("OK"));}
  if (!RTC.isrunning())
   {Serial.println(F("Failed")); delay(7000); initRTC();}
}


// Print available (free) SRAM in Serial Monitor.
int availableSRAM()
{
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}
