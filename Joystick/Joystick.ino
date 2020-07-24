#define SHIFTLED 4
#define SWITCHINPUT 3
#define ANALOG_X A7
#define ANALOG_Y A6
unsigned long time, doubleclick = 1000;
bool shift_lock = false;
bool buttonlast = LOW;

void setup()
{
  Serial.begin(9600);
  pinMode(SHIFTLED, OUTPUT);   //LED
  pinMode(SWITCHINPUT, INPUT); // Switch Input
  pinMode(2, OUTPUT);          //5V to switch
  digitalWrite(2, HIGH);       //5V to switch
}

bool check_shift()
{
  time = millis();
  if (digitalRead(3))
  {
    if ((time - doubleclick > 650) && shift_lock)
    {
      shift_lock = false;
      doubleclick = time;
    }

    else if ((time - doubleclick < 250) && !shift_lock && buttonlast == LOW)
    {
      shift_lock = true;
      Serial.print("SHIFTLOCK\r");
    }
    buttonlast = HIGH;
    return true;
  }

  else if (shift_lock)
    return true;

  else
  {
    if (buttonlast == HIGH)
      doubleclick = time;
    buttonlast = LOW;
    return false;
  }
}

void loop()
{
  int y = analogRead(A7) / 10;
  int x = analogRead(A6) / 10;
  bool shift = check_shift();
  digitalWrite(SHIFTLED, shift);

  int X = (50 - x) / 3;
  int Y = (50 - y) / 3;

  Serial.print("X:");
  Serial.print(X);
  Serial.print("\t");

  Serial.print("Y:");
  Serial.print(Y);
  Serial.print('\r');
  delay(50);
}