#define DO 523.25
#define RE 587.33
#define MI 659.26
#define FA 698.46
#define SOL 783.99
#define LA 880
#define SI 987.77
#define RE2 1174.66
#define DO2 1062
#define DoS 554.37
#define RES 622.25
#define FAS 739.99
#define SOLS 830.61
#define FAS2 1479.98
#define a 1000
#define b 500

#define PIN_LED_RED     38
#define PIN_LED_GREEN   39
#define PIN_LED_BUTTON  36
#define PIN_LED_ALARM   41

uint32_t servoTimer = 2000;
uint32_t ledGreenTimer = 10;
uint32_t ledRedTimer = 1000;

uint32_t servoLapsed = 0;
uint32_t ledGreenLapsed = 0;
uint32_t ledRedLapsed = 0;

uint8_t ledGreenValue = 0;

bool servoStatus = false;
bool ledRedStatus = false;

void setup() 
{
  pinMode(PIN_LED_RED, OUTPUT);
  pinMode(PIN_LED_GREEN, OUTPUT);
  pinMode(PIN_LED_BUTTON, INPUT);
  pinMode(PIN_LED_ALARM, OUTPUT);
}

void loop() 
{  
  if (millis() > ledRedTimer + ledRedLapsed)
  {
    ledRedLapsed = millis();
    digitalWrite(PIN_LED_RED, ledRedStatus);
    ledRedStatus = !ledRedStatus;
  }

  if (millis() > ledGreenTimer + ledGreenLapsed)
  {
    ledGreenLapsed = millis();
    analogWrite(PIN_LED_GREEN, ++ledGreenValue);
    if (ledGreenValue == 255) 
    {
      ledGreenValue = 0;
    }
  }
  
  if (digitalRead(PIN_LED_BUTTON)) 
  {
    himno();
  }
}

void himno()
{
  tone(PIN_LED_ALARM, SOL , a);
  delay(500);
  noTone(PIN_LED_ALARM);
  if (!digitalRead(PIN_LED_BUTTON)) {return;}

  tone(PIN_LED_ALARM, RE , a);
  delay(500);
  noTone(PIN_LED_ALARM);
  if (!digitalRead(PIN_LED_BUTTON)) {return;}

  tone(PIN_LED_ALARM, SI , a);
  delay(500);
  noTone(PIN_LED_ALARM);
  if (!digitalRead(PIN_LED_BUTTON)) {return;}

  tone(PIN_LED_ALARM, SOL , b);
  delay(500);
  noTone(PIN_LED_ALARM);
  if (!digitalRead(PIN_LED_BUTTON)) {return;}
  
  tone(PIN_LED_ALARM, RE2 , b);
  delay(400);
  noTone(PIN_LED_ALARM);
  if (!digitalRead(PIN_LED_BUTTON)) {return;}

  tone(PIN_LED_ALARM, DO2 , b);
  delay(400);
  noTone(PIN_LED_ALARM);
  if (!digitalRead(PIN_LED_BUTTON)) {return;}

  tone(PIN_LED_ALARM, SI , b);
  delay(400);
  noTone(PIN_LED_ALARM); 
  if (!digitalRead(PIN_LED_BUTTON)) {return;}

  tone(PIN_LED_ALARM, LA , b);
  delay(400);
  noTone(PIN_LED_ALARM);
  if (!digitalRead(PIN_LED_BUTTON)) {return;}

  tone(PIN_LED_ALARM, SOL , b);
  delay(400);
  noTone(PIN_LED_ALARM);
  if (!digitalRead(PIN_LED_BUTTON)) {return;}

  tone(PIN_LED_ALARM, SOL , b);
  delay(400);
  noTone(PIN_LED_ALARM);
  if (!digitalRead(PIN_LED_BUTTON)) {return;}

  tone(PIN_LED_ALARM, FAS , b);
  delay(400);
  noTone(PIN_LED_ALARM);
  if (!digitalRead(PIN_LED_BUTTON)) {return;}

  tone(PIN_LED_ALARM, MI , b);
  delay(400);
  noTone(PIN_LED_ALARM);
  if (!digitalRead(PIN_LED_BUTTON)) {return;}

  tone(PIN_LED_ALARM, RE , b);
  delay(400);
  noTone(PIN_LED_ALARM);
  if (!digitalRead(PIN_LED_BUTTON)) {return;}

  tone(PIN_LED_ALARM, SOL , a);
  delay(500);
  noTone(PIN_LED_ALARM);
  if (!digitalRead(PIN_LED_BUTTON)) {return;}

  tone(PIN_LED_ALARM, LA , a);
  delay(500);
  noTone(PIN_LED_ALARM); 
  if (!digitalRead(PIN_LED_BUTTON)) {return;}

  tone(PIN_LED_ALARM, SI , b + b);
  delay(1000);
  noTone(PIN_LED_ALARM);
  if (!digitalRead(PIN_LED_BUTTON)) {return;}

  tone(PIN_LED_ALARM, RE2 , b);
  delay(400);
  if (!digitalRead(PIN_LED_BUTTON)) {return;}

  tone(PIN_LED_ALARM, DO2 , b);
  delay(400);
  if (!digitalRead(PIN_LED_BUTTON)) {return;}

  tone(PIN_LED_ALARM, SI , b);
  delay(400);
  if (!digitalRead(PIN_LED_BUTTON)) {return;}
  
  tone(PIN_LED_ALARM, LA , b);
  delay(400);
  if (!digitalRead(PIN_LED_BUTTON)) {return;}
  
  tone(PIN_LED_ALARM, SOL , b);
  delay(400);
  if (!digitalRead(PIN_LED_BUTTON)) {return;}

  tone(PIN_LED_ALARM, RE2 , b);
}