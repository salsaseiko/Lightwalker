#include <QuickPID.h>
#include <ESP32Servo.h>

// var servo
Servo myservo;
Servo myservo2;  

const float THRESHOLD = 0.15; 
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
//int diff = 0;
float diff = 0;
int left = 0;
int right = 0;

// var per PID
float luceInput = 0; // poi aggiornata con la somma delle due letture di ldr
float output; // output del PID, da mappare sul plusStep
float luceObiettivo = 2000; 
float Kp = 0.8, Ki = 0.0, Kd = 0.07;

int lastStop = 1; 

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

  float tot = right + left;
  diff = (float)(right - left) / tot;

  luceInput = tot;

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
if (digitalRead(stopPin) == 0) {

        if (abs(diff) < THRESHOLD) {
            avanti();
        }
        else if (diff < -THRESHOLD) {
            sinistra();
        }
        else if (diff > THRESHOLD) {
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


