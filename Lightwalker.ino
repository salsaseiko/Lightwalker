#include <QuickPID.h>
#include <ESP32Servo.h>

// var servo
Servo myservo;
Servo myservo2;  

int THRESHOLD = 200; 
int speed = 300; 

int zero1 = 90;
int zero2 = 90;
int step = 20; // step base dritto
int stepVira = 40; // step per virare a dx/sx
int plusStep = 0; // step in più per avvicinarsi alla luce

// var fotoresistenze
int stopPin = 34; // 1 = buio, 0 = luce
volatile bool stop = false;

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

int luceAmbiente = 0; // luce percepita all'inizio
int lastStop = 1; 

// var per PID
float luceInput = luceAmbiente; // poi aggiornata con la somma delle due letture di ldr
float output; // output del PID, da mappare sul plusStep
float luceObiettivo = 2000 + luceAmbiente; 
float Kp = 0.8, Ki = 0.0, Kd = 0.07;

QuickPID lucePID(&luceInput, &output, &luceObiettivo);

void IRAM_ATTR ISR() {
  stop = true;
}

void setup() {
// setup dei pin LDR
  Serial.begin(9600);
  pinMode(ldrL, INPUT);
  pinMode(ldrR, INPUT);
  pinMode(stopPin, INPUT);

// setup dei servo
  myservo.attach(32); // servo con pin 32 è davanti
  myservo2.attach(33);
  myservo.write(zero1);  
  myservo2.write(zero2); 

// setup del threshold 
  calibraLuce();
  THRESHOLD += luceAmbiente;
  THRESHOLD = constrain(THRESHOLD, 100, 800);

// setup interrupt = se non c'è luce, interrompi la camminata
  attachInterrupt(digitalPinToInterrupt(stopPin), ISR, RISING); // 0 -> 1 ARRIVA IL BUIO

// setup PID
  lucePID.SetTunings(Kp, Ki, Kd); 
  lucePID.SetOutputLimits(0, 1023); 
  lucePID.SetMode(lucePID.Control::automatic);
}


void loop() {
  
  int currentStop = digitalRead(stopPin);

  left  = mediaMobile(ldrL, misureL, sumL);
  right = mediaMobile(ldrR, misureR, sumR);

  idx = (idx + 1) % N;

  diff = right - left;

  luceInput = right + left;

// NON attuo la correzione allo step se la luce è stata appena accesa (sennò va subito veloce anche se vicina) --> condizione di "FALLING"
 if (currentStop != lastStop && currentStop == 0) { 
   plusStep = 0;
 } else {
  lucePID.Compute();
  plusStep = map(output, 0, 1023, 0, 30);
 }
 // stampa();

  if (!stop) {
    valuta();
  }
  stop = false;
  lastStop = currentStop;

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

/*void stampa() {
  Serial.print("Threshold: "); Serial.println(THRESHOLD);
  Serial.println(right);
  Serial.println(left);
  Serial.print("DIFF: "); Serial.println(diff);
  Serial.print("luceInput: "); Serial.println(luceInput);
  Serial.print("output: "); Serial.println(output);
  Serial.print("plusStep: "); Serial.println(plusStep);


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
  myservo.write(zero1 + step + plusStep);
  // gamba dietro segue
  myservo2.write(zero2 + step + plusStep);
  delay(speed);

  // ritorno al centro
  myservo.write(zero1);
  myservo2.write(zero2);
  delay(speed);

   myservo.write(zero1 - step - plusStep);
  myservo2.write(zero2 - step - plusStep);
  delay(speed);

  myservo.write(zero1);
  myservo2.write(zero2);
  delay(speed);
}

void sinistra() {
  myservo.write(zero1 - stepVira);
  delay(speed);
  myservo2.write(zero2 - stepVira);
  delay(speed);

  myservo.write(zero1);
  myservo2.write(zero2);
  delay(speed);
}

void destra() {
  myservo.write(zero1 + stepVira);
  delay(speed);
  myservo2.write(zero2 + stepVira);
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



