#define KNOB_1 A1
#define KNOB_2 A2
#define KNOB_3 A3

int v1, v2, v3;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  v1 = analogRead(KNOB_1);
  v2 = analogRead(KNOB_2);
  v3 = analogRead(KNOB_3);
  Serial.print(v1); Serial.print(" ; ");
  Serial.print(v2); Serial.print(" ; ");
  Serial.println(v3);
  delay(10);
}
