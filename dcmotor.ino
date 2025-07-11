// Define motor control pins
const int IN1 = 2;
const int IN2 = 3;
const int ENA = 9; // PWM-capable pin

void setup() {
  Serial.begin(9600);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENA, OUTPUT);
}

void loop() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, 210);
  delay(1000);
}
