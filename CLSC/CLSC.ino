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
*************************************************************/

#include <Wire.h>
#include <RTClib.h>
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 4
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
float temp1, temp2;
DeviceAddress insensor = {0x28, 0xB4, 0x63, 0x05, 0x05, 0x00, 0x00, 0x2E};
DeviceAddress outsensor;

RTC_DS1307 rtc;

LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);
const int lcdledpin = 3;

char* currentversion = "v0.3";

//output pin to control the blinker relay
const int signalpin = 2;

//output pin to control the cls
const int clspin = 5;

//input signal from ignition switch
const int ignitionpin = 6;

//four input pins from the remote control unit
const int remotepin1 = 9;
const int remotepin2 = 10;
const int remotepin3 = 11;
const int remotepin4 = 12;

//determine if central locking is controlled using one or two buttons from the remote
int lockmode = 1;

//are the doors locked?
boolean doorslocked = false;

//locking/unlocking command states
boolean locksignal1 = false;
boolean locksignal2 = false;

//custom display characters
byte degree[8] = {
  B00110,
  B01001,
  B01001,
  B00110
};

void setup()
{
  Serial.begin(9600);
  Wire.begin();
  rtc.begin();
  lcd.begin(16,2);
  sensors.begin();
  
  if (! rtc.isrunning()) {
    lcd.setCursor(3,0); lcd.print("RTC is");
    lcd.setCursor(2,1); lcd.print("NOT running!");
  }

  // following line sets the RTC to the date & time this sketch was compiled
  rtc.adjust(DateTime(__DATE__, __TIME__));
  
  pinMode(signalpin, OUTPUT);
  pinMode(lcdledpin, OUTPUT);
  pinMode(clspin, OUTPUT);
  pinMode(ignitionpin, INPUT);
  pinMode(remotepin1, INPUT);
  pinMode(remotepin2, INPUT);
  pinMode(remotepin3, INPUT);
  pinMode(remotepin4, INPUT);

  lcd.createChar(0, degree);

  //show startup message
  lcd.setCursor(3,0); lcd.print("Starduino");
  lcd.setCursor(6,1); lcd.print(currentversion);
  if (digitalRead(ignitionpin) == HIGH)
  {
    for(int i=0;i<80;i=i+5)
    {
      analogWrite(lcdledpin, i);
      delay(50);
    }
  }
  delay(1000);
  lcd.clear();
}

void loop()
{
  if (digitalRead(ignitionpin) == LOW && analogRead(lcdledpin) > 0)
  {
    for(int i=80;i>0;i=i-5)
    {
      analogWrite(lcdledpin, i);
      delay(50);
    }
    lcd.setBacklight(0);
  }
  
  if (digitalRead(ignitionpin) == HIGH && analogRead(lcdledpin) < 80)
  {
    lcd.setBacklight(1);
    for(int i=0;i<80;i=i+5)
    {
      analogWrite(lcdledpin, i);
      delay(50);
    }
  }
  DateTime now = rtc.now();
  locksignal1 = digitalRead(remotepin1);
  locksignal2 = digitalRead(remotepin2);
  
  sensors.requestTemperatures();  //check temperature from the sensors
  temp1 = sensors.getTempC(insensor);  //inside temperature
  temp2 = sensors.getTempC(outsensor);  //outside temperature

  //show time and date
  lcd.setCursor(2,0); if(now.hour()<10){lcd.print(" ");} lcd.print(now.hour(), DEC); lcd.print(":"); if(now.minute()<10){lcd.print("0");} lcd.print(now.minute(), DEC);
  lcd.setCursor(9,0); lcd.print(now.day(), DEC); lcd.print("."); lcd.print(now.month(), DEC); lcd.print(".");
  
  //show temperatures
  lcd.setCursor(0,1); lcd.print(temp1); if(temp1 < 10){lcd.setCursor(3,1);} else {lcd.setCursor(4,1);} lcd.print(" "); lcd.write(byte(0)); lcd.print("C");
  lcd.setCursor(9,1); lcd.print(temp2); if(temp2 < 10){lcd.setCursor(12,1);} else {lcd.setCursor(13,1);} lcd.print(" "); lcd.write(byte(0)); lcd.print("C ");
  
  if((locksignal1 || locksignal2) && !(locksignal1 && locksignal2))
  {
    centrallocking();
  }
}

void centrallocking()
{
  //clear lcd for message and start blinking sequence by setting blinkers on
  lcd.clear();
  digitalWrite(signalpin, HIGH);
    
  //unlock doors
  if (doorslocked && locksignal1)
  {
    digitalWrite(clspin, LOW);
    lcd.on();
    /*for(int i=0;i<80;i=i+5)
    {
      analogWrite(lcdledpin, i);
      delay(50);
    }*/
    lcd.setCursor(6,0); lcd.print("Ovet");
    lcd.setCursor(4,1); lcd.print("avataan!");
    delay(1500);
    digitalWrite(signalpin, LOW);
    lcdscroll();
  }
  
  //lock doors
  if (!doorslocked && ((lockmode == 1 && locksignal1) || (lockmode == 2 && locksignal2)))
  {
    digitalWrite(clspin, HIGH);
    lcd.setCursor(6,0); lcd.print("Ovet");
    lcd.setCursor(4,1); lcd.print("lukitaan!");
    delay(500);
    digitalWrite(signalpin, LOW);
    delay(500);
    digitalWrite(signalpin, HIGH);
    delay(500);
    digitalWrite(signalpin, LOW);
    lcdscroll();
    /*for(int i=80;i>0;i=i-5)
    {
      analogWrite(lcdledpin, i);
      delay(50);
    }*/
    lcd.off();
  }

  doorslocked = !doorslocked;

  //make sure the blinker goes off
  digitalWrite(signalpin, LOW);
}

void lcdscroll()
{
  for (int positionCounter = 0; positionCounter < 16; positionCounter++)
  {
    lcd.scrollDisplayLeft();
    delay(150);
  }
  lcd.clear();
}

