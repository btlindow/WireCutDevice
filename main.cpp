#define VERSION "1.1"

//Library Inclusions
#include <Wire.h>
#include <LCDi2cNHD.h>
#include <inttypes.h>

//Set LCD Address
LCDi2cNHD lcd = LCDi2cNHD(4,20,0x50>>1,0);

uint8_t rows = 4;
uint8_t cols = 20;

//Pin Values
int DI = 1;
int microSwitch = 2;
int AI = 3;
int penalty = 4;

int wireA = 5;
int wireB = 6;
int wireC = 7;
int wireD = 8;
int wireE = 9;
int wireF = 10;
int wireG = 11;
int wireH = 12;

//Variable Values
int TIME = 480;
int skips [8];
int order [8];
int submit [8];
int rate = 1;
int wireVal [9];
int AIstate = LOW;
int honest = 0;
int errorCount = 0;

const long interval = 1000;

unsigned long previousMillis = 0;
unsigned long spreviousMillis = 0;

void setup()
{
  pinMode (DI, OUTPUT);
  pinMode (microSwitch, INPUT);
  pinMode (AI, OUTPUT);
  pinMode (penalty, OUTPUT);
  pinMode (wireA, INPUT);
  pinMode (wireB, INPUT);
  pinMode (wireC, INPUT);
  pinMode (wireD, INPUT);
  pinMode (wireE, INPUT);
  pinMode (wireF, INPUT);
  pinMode (wireG, INPUT);
  pinMode (wireH, INPUT);
  
  //Initialize the LCD
  lcd.init();
  lcd.clear();
  
  //Reads value from microswitch
  while (digitalRead (microSwitch) == 1)
  {
    delay (1);
  }
  
  //Initialize skip values to 0
  for (int i = 0; i < 8; i++)
  {
    skips [i] = 0;
    submit [i] = 10;
    order [i] = i;
  }
  
  delay (1000);
}

void loop()
{
  getDigital();
  testDigital();
  penTest();
  
  //Beep and countdown without delay
  unsigned long currentMillis = millis();
  
  if (currentMillis - previousMillis >= (interval / rate))
  {
    previousMillis = currentMillis;
    
    countdown();
    
    if (AIstate == LOW)
    {
      AIstate = HIGH;
    }
    else
    {
      AIstate = LOW;
    }
    
    digitalWrite (AI, AIstate);
  }
}

//Test penalty conditions
void penTest()
{
  int counter = 0;  
  
  //Kills if the microswitch was pressed after lid opens
  if (digitalRead (microSwitch) == 1)
  {
    kill();
  }
  
  //Kills if time runs out
  if (TIME == 0)
  {
    kill();
  }
  
  //Sums up all the values of the wires
  for (int i = 0; i < 8; i++)
  {
    counter = counter + wireVal [i];
  }
  
  
  unsigned long secondMillis = millis();
  
  if (secondMillis - spreviousMillis >= interval)
  {
    spreviousMillis = secondMillis;
   
    if (counter == 1)
    {
      honest = 1;
    }
  }
  
  //Disarms if all the wire vales sum up to zero
  if ((counter == 0) && (honest == 1))
  {
    disarm();
  }
  
  //Kills if all wires are cut at once
  if ((counter == 0) && (honest == 0))
  {
    kill();
  }
}

//Penalty siren
void kill()
{
  digitalWrite (AI, LOW);
  lcd.clear();
  lcd.setCursor(0,2);
  lcd.print ("YOU");
  lcd.setCursor(1,5);
  lcd.print ("BETTER");
  lcd.setCursor(2,11);
  lcd.print ("RUN");
  lcd.setCursor(3,14);
  lcd.print ("FAST");
  
  delay(2000);
  digitalWrite (penalty, HIGH);
  delay (1000);
  digitalWrite (penalty, LOW);
  delay (1000);
  reminder();
}

//Reads and stores wire values  
void getDigital()
{
  for (int i = 0; i < 8; i++)
  {
   wireVal [i] = digitalRead (i+5);
  }
}

//Determines if a wire value is cut out of order  
void testDigital()
{
  int buffer;
  int holder;
  
  for (int i = 0; i < 8; i++)
  {
    if ((wireVal [i] == 0) && skips [i] == 0)
    {
      skips [i] = 1;
      holder = i;
      for (int j = 0; j < 8; j++)
      {
        if (submit [j] > 9)
        {
          submit [j] = i;
          if (submit [j] != order [j])
          {
            rate = 10000;
            errorCount++;
            submit [j] = 10;
            buffer = 8 - i;
            
            for (int k = 1; k < buffer; k++)
            {
              order [holder] = order [holder+1];
              holder++;
            }
          }
          break;
        }
      }
    }
  }
  
  if (errorCount == 2)
  {
    kill();
  }
}
  
//Disarm function  
void disarm()
{
  lcd.clear();
  lcd.setCursor(1,0);
  lcd.print("      DISARMED");
  digitalWrite (AI, LOW);
  digitalWrite (penalty, LOW);
  delay (500);
  digitalWrite (AI, HIGH);
  delay(100);
  digitalWrite (AI, LOW);
  delay(50);
  digitalWrite (AI, HIGH);
  delay(100);
  digitalWrite (AI, LOW);
  delay(50);
  digitalWrite (AI, HIGH);
  delay (100);
  digitalWrite (AI, LOW);
  digitalWrite (DI, HIGH);
  delay(60000);
  reminder();
}

//Displays a countdown to the screen
void countdown()
{
  int minutes = TIME / 60;
  int seconds = TIME % 60;
  
  lcd.setCursor(1,0);
  lcd.print("   Time Remaining");
  lcd.setCursor(2,8);
  lcd.print(minutes);
  lcd.print(":");
  
  if (seconds > 9)
  {
    lcd.print(seconds);
  }
  else
  {
    lcd.print("0");
    lcd.print(seconds);
  }
  TIME--;
}

//Reminder to turn off the device and reset the cable
void reminder()
{
  digitalWrite (AI, LOW);
  digitalWrite (penalty, LOW);
  lcd.clear();
  lcd.print("POWER OFF AND RESET");
  
  while (1)
  {
    digitalWrite (AI, HIGH);
    digitalWrite (DI, HIGH);
    delay(100);
    digitalWrite (DI, LOW);
    digitalWrite (AI, LOW);
    delay (900);
  }
}
