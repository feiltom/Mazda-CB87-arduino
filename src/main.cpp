// Jauge air-core (bobines croisees sinus/cosinus) du compteur Mazda, pilotee directement par l'Uno
// Cablage : C+ -> D3, C- -> D9, S+ -> D11, S- -> D10   (toutes des sorties PWM)
//
// L'aiguille s'aligne sur le champ des deux bobines : I(C) ~ cos(angle), I(S) ~ sin(angle).
// Moniteur serie (115200) : un nombre = angle en degres (0-360)
//                           o 30  = decalage du zero (pour caler l'aiguille sur la graduation 0)
//                           s     = balayage lent 0 -> 270 -> 0
//                           m 200 = PWM max (0-255), limite le courant dans les bobines

#include <Arduino.h>

const int PIN_C_POS = 3;
const int PIN_C_NEG = 9;
const int PIN_S_POS = 11;
const int PIN_S_NEG = 10;

int   pwmMax    = 200;   // ~80 % : environ 19 mA par bobine de 205 ohms sous 5 V
float offsetDeg = -38;   // cale le 0 sur la graduation 0 du cadran (reglable avec "o")
float current   = 0;

// Une bobine = un pont en H fait de deux broches : l'une en PWM, l'autre a 0 V
void driveCoil(int pinPos, int pinNeg, float level) {
  int duty = constrain(lround(fabs(level) * pwmMax), 0, 255);
  if (level >= 0) {
    analogWrite(pinNeg, 0);
    analogWrite(pinPos, duty);
  } else {
    analogWrite(pinPos, 0);
    analogWrite(pinNeg, duty);
  }
}

void setAngle(float deg) {
  float rad = (deg + offsetDeg) * PI / 180.0;
  driveCoil(PIN_C_POS, PIN_C_NEG, cos(rad));
  driveCoil(PIN_S_POS, PIN_S_NEG, sin(rad));
  current = deg;
}

// Deplacement progressif : un saut brutal pourrait faire tourner l'aiguille du mauvais cote
void moveTo(float target) {
  float stepDeg = target > current ? 0.5 : -0.5;
  while (fabs(target - current) > 0.5) {
    setAngle(current + stepDeg);
    delay(3);
  }
  setAngle(target);
}

void setup() {
  pinMode(PIN_C_POS, OUTPUT);
  pinMode(PIN_C_NEG, OUTPUT);
  pinMode(PIN_S_POS, OUTPUT);
  pinMode(PIN_S_NEG, OUTPUT);

  // PWM a ~31 kHz au lieu de 490 Hz : plus de sifflement audible dans les bobines
  // (timer1 = D9/D10, timer2 = D3/D11 ; timer0 intact pour delay/millis)
  TCCR1B = (TCCR1B & 0b11111000) | 0x01;
  TCCR2B = (TCCR2B & 0b11111000) | 0x01;

  Serial.begin(115200);
  setAngle(0);
  Serial.println("Jauge air-core prete. Angle (0-360), 'o <deg>', 's' ou 'm <0-255>'");
}

void loop() {
  if (!Serial.available()) return;
  String cmd = Serial.readStringUntil('\n');
  cmd.trim();
  if (cmd.length() == 0) return;

  if (cmd.startsWith("o")) {
    offsetDeg = cmd.substring(1).toFloat();
    setAngle(current);
    Serial.print("Decalage zero = ");
    Serial.println(offsetDeg);
    return;
  }
  if (cmd.startsWith("m")) {
    pwmMax = constrain(cmd.substring(1).toInt(), 0, 255);
    setAngle(current);
    Serial.print("PWM max = ");
    Serial.println(pwmMax);
    return;
  }
  if (cmd == "s") {
    Serial.println("Balayage...");
    moveTo(270);
    moveTo(0);
    Serial.println("Fin");
    return;
  }
  moveTo(cmd.toFloat());
  Serial.print("-> ");
  Serial.print(current, 1);
  Serial.println(" deg");
}
