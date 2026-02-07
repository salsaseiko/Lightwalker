#include <ESP32Servo.h>

// var servo
Servo myservo;
Servo myservo2;  

int THRESHOLD = 300; 
int speed = 300; 

int zero1 = 90;
int zero2 = 90;
int step = 50;
int step2 = 40;

// var fotoresistenze
int stopPin = 34; // 1 = buio, 0 = luce
bool stop = false;

int ldrL = 14;
int ldrR = 27;
const int N = 3; 
int misureL[N];
int misureR[N];
int idx = 0;
long sumL = 0;
long sumR = 0;
int diff = 0;
int left = 0;
int right = 0;

int luceAmbiente = 0;

void IRAM_ATTR ISR() {
  stop = true;
}

void setup() {

  Serial.begin(9600);
  pinMode(ldrL, INPUT);
  pinMode(ldrR, INPUT);
  pinMode(stopPin, INPUT);

  myservo.attach(32); // servo con pin32 è davanti, ha i piedi più alti
  myservo2.attach(33);

  myservo.write(zero1);  // servo1 col meno vira a destra
  myservo2.write(zero2); // servo2 col meno vira a sinistra

  calibraLuce();

  THRESHOLD += luceAmbiente;
  THRESHOLD = constrain(THRESHOLD, 100, 800);

  // interrupt = se non c'è luce, interrompi la camminata.
  attachInterrupt(digitalPinToInterrupt(stopPin), ISR, RISING); // 0 -> 1 ARRIVA IL BUIO
}


void loop() {
  
  left  = mediaMobile(ldrL, misureL, sumL);
  right = mediaMobile(ldrR, misureR, sumR);

  idx = (idx + 1) % N;

  diff = right - left;

//  stampa();

  if (!stop) {
    valuta();
  }
  stop = false;

}


void valuta() {
   bool dritto = abs(diff) < THRESHOLD;
  if (digitalRead(stopPin) == 0) {
    if (dritto) {
      avanti();
      } else if (diff < -THRESHOLD) {
        sinistra();
      } else if (diff > THRESHOLD) {
        destra();
      }
  }
}
/*
void stampa() {
  Serial.print("Threshold: "); Serial.println(THRESHOLD);
  Serial.println(right);
  Serial.println(left);
  Serial.println(diff);

  Serial.println(digitalRead(stopPin));
  Serial.println("---");
  delay(500);
}*/

int mediaMobile(int pin, int *misure, long &sum) {
  int x;
  x = misure[idx];
  sum = sum - x;
  misure[idx] = analogRead(pin);
  x = misure[idx];
  sum = sum + misure[idx];

  int valoreSmussato = sum / N;
  return valoreSmussato;
}

void avanti() {
  // gamba davanti spinge
  myservo.write(zero1 + step +10);
  // gamba dietro segue
  myservo2.write(zero2 - step2);
  delay(speed);

  // ritorno al centro
  myservo.write(zero1);
  myservo2.write(zero2);
  delay(speed);

   myservo.write(zero1 - step -10);
  myservo2.write(zero2 + step2);
  delay(speed);

  myservo.write(zero1);
  myservo2.write(zero2);
  delay(speed);
}

void sinistra() {
  myservo.write(zero1 - step);
  delay(speed);
  myservo2.write(zero2 - step);
  delay(speed);

  myservo.write(zero1);
  myservo2.write(zero2);
  delay(speed);
}

void destra() {
  myservo.write(zero1 + step);
  delay(speed);
  myservo2.write(zero2 + step);
  delay(speed);

  myservo.write(zero1);
  myservo2.write(zero2);
  delay(speed);

}

void calibraLuce() {
  const int campioni = 30;
  long totL = 0;
  long totR = 0;

  for (int i = 0; i < campioni; i++) {
    totL += analogRead(ldrL);
    totR += analogRead(ldrR);
    delay(10);
  }

  int baseL = totL / campioni;
  int baseR = totR / campioni;
  luceAmbiente = (baseL + baseR) / 2;

}



