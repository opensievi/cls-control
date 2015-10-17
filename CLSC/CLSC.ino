/*************************************************************
*  project: Central Locking System control
*  author: Simo Hakala
*  description: A sketch to control various tasks in my 1990
*  Mercedes Benz 230TE, mainly the central locking system.
*
*  v0.1: radio controlled central locking system (CLS)
*  v0.2: temperature display both inside and outside of the
*        vehicle, option to use two remote buttons
*        to control the cls
*  v0.3: display backlight control from ignition signal
*  v0.3.1: -added an input for checking the current door
*          locking state
*          -set the locksignals to use interrupts
*          -removed the two button option added in v0.2
*          -bug fixing
*************************************************************/

#include <Wire.h>
#include <RTClib.h>
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#include <DallasTemperature.h>

char* currentversion = "v0.3.1";

#define ONE_WIRE_BUS 4
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
float temp1, temp2;
DeviceAddress sensor_int = {0x28, 0xB4, 0x63, 0x05, 0x05, 0x00, 0x00, 0x2E};
DeviceAddress sensor_ext;

RTC_DS1307 rtc;

LiquidCrystal_I2C lcd(0x27,16,2);
const int pin_lcdled = 6; //PWM output pin to control lcd brightness
int brightness = 80; //lcd background brightness

const int pin_signal = 7; //output pin to control the blinker relay
const int pin_cls = 5; //output pin to control the cls

const int pin_ignition = 8; //input signal from ignition switch
const int pin_lockstate = 3; //input pin to determine lockstate (D3 = INT1)
boolean doorslocked;

//four input pins from the remote control unit
const int pin_remote1 = 2; //(D2 = INT0)
const int pin_remote2 = 10;
const int pin_remote3 = 11;
const int pin_remote4 = 12;

//reset locksignals
volatile boolean signal_remote;
volatile boolean signal_local;

//custom display characters
byte degree[8] = {
  B00110,
  B01001,
  B01001,
  B00110
};

void lock_signal() //interrupt service routine (ISR)
{
  if (digitalRead(pin_remote1) == LOW) //remote locksignal
  {
    signal_remote = true;
  } else {
    signal_local = true;
  }
}

void setup()
{
  Wire.begin();
  sensors.begin();

  pinMode(pin_signal, OUTPUT);
  pinMode(pin_lcdled, OUTPUT);
  pinMode(pin_cls, OUTPUT);
  pinMode(pin_lockstate, INPUT_PULLUP);
  pinMode(pin_ignition, INPUT_PULLUP);
  pinMode(pin_remote1, INPUT_PULLUP);
  pinMode(pin_remote2, INPUT_PULLUP);
  pinMode(pin_remote4, INPUT_PULLUP);
  pinMode(pin_remote3, INPUT_PULLUP);
  
  if (pin_lockstate == LOW)
  {
    doorslocked = true;
  } else {
    doorslocked = false;
  }
  
  attachInterrupt (0,lock_signal,LOW);
  attachInterrupt (1,lock_signal,CHANGE);

  //show startup message
  lcd.init();
  lcd.backlight();
  lcd.setBacklight(80);
  lcd.createChar(0, degree);
  lcd.setCursor(6, 0); lcd.print("CLSC");
  lcd.setCursor(5, 1); lcd.print(currentversion);
  if (digitalRead(pin_ignition) == LOW)
  {
    for (int i = 0; i < brightness; i = i + 5)
    {
      analogWrite(pin_lcdled, i);
      delay(50);
    }
    brightness = 80;
  } else {
    brightness = 0;
  }
  
  rtc.begin();
  if (! rtc.isrunning()) {
    lcd.setCursor(3, 0); lcd.print("RTC is");
    lcd.setCursor(2, 1); lcd.print("NOT running!");
  }
  // following line sets the RTC to the date & time this sketch was compiled
  rtc.adjust(DateTime(__DATE__, __TIME__));

  delay(1000);
  lcd.clear();
  signal_remote = false;
  signal_local = false;
}

void loop()
{
  //ignition pin state
  if (digitalRead(pin_ignition) == HIGH)
  {
    if (brightness > 0)
    {
      //fade off
      for (int i = 80; i > 0; i = i - 5)
      {
        analogWrite(pin_lcdled, i);
        delay(50);
      }
      analogWrite(pin_lcdled, 0);
      brightness = 0;
    }
  }
  
  if (digitalRead(pin_ignition) == LOW)
  {
    if (brightness < 80)
    {
      //fade on
      for (int i = 0; i < 80; i = i + 5)
      {
        analogWrite(pin_lcdled, i);
        delay(50);
      }
      analogWrite(pin_lcdled, 80);
      brightness = 80;
    }
  }

  DateTime now = rtc.now();
  sensors.requestTemperatures();  //check temperature from the sensors
  temp1 = sensors.getTempC(sensor_int);  //inside temperature
  temp2 = sensors.getTempC(sensor_ext);  //outside temperature
  
  //show time and date
  //lcd.clear();
  lcd.setCursor(0, 0); if (now.hour() < 10) {lcd.print("   ");} else {lcd.print("  ");} lcd.print(now.hour(), DEC); lcd.print(":"); if (now.minute() < 10) {lcd.print("0");} lcd.print(now.minute(), DEC);
  lcd.setCursor(9, 0); if (now.day() < 10) {lcd.print(" ");} lcd.print(now.day(), DEC); lcd.print("."); lcd.print(now.month(), DEC); lcd.print("."); lcd.print("  ");

  //show temperatures
  lcd.setCursor(0, 1); lcd.print(temp1); if (temp1 < 10) {lcd.setCursor(3, 1);} else {lcd.setCursor(4, 1);} lcd.print(" "); lcd.write(byte(0)); lcd.print("C");
  lcd.setCursor(9, 1); lcd.print(temp2); if (temp2 < 10) {lcd.setCursor(12, 1);} else {lcd.setCursor(13, 1);} lcd.print(" "); lcd.write(byte(0)); lcd.print("C ");
  //lcd.setCursor(0, 1); lcd.print(analogRead(pin_lcdled));
  //lcd.setCursor(9, 1); lcd.print(digitalRead(pin_ignition));

  if (signal_remote || signal_local)
  {
    centrallocking();
  }
}

void centrallocking()
{
  //clear lcd for message and start blinking sequence by setting blinkers on
  lcd.clear();
  digitalWrite(pin_signal, HIGH);

  if (doorslocked)
  {
    digitalWrite(pin_cls, LOW);
    lcd.setCursor(6, 0); lcd.print("Ovet"); //message about unlocking doors
    lcd.setCursor(4, 1); lcd.print("avataan!");
    delay(1500);
    digitalWrite(pin_signal, LOW); //one long blink
    doorslocked = false;
  }
  else
  {
    digitalWrite(pin_cls, HIGH);
    lcd.setCursor(6, 0); lcd.print("Ovet"); //message about locking doors
    lcd.setCursor(4, 1); lcd.print("lukitaan!");
    delay(200);
    digitalWrite(pin_signal, LOW);
    delay(200);
    digitalWrite(pin_signal, HIGH);
    delay(200);
    digitalWrite(pin_signal, LOW); //two short blinks
    delay(500);
    doorslocked = true;
  }

  //reset blinker and locksignal states
  digitalWrite(pin_signal, LOW);
  signal_remote = false;
  signal_local = false;
  lcd.clear();
}
