#define DIR 10 //so that it is also compatible with PWM
#define PUL 3
#define STEPS 1600
#define DELAY 

void setup() {
  // put your setup code here, to run once:
  pinMode(DIR, OUTPUT);
  pinMode(PUL, OUTPUT);
  digitalWrite(DIR, HIGH);
  for (int i = 0; i <STEPS; i++) {
    digitalWrite(PUL, HIGH);
    delay(1);
    digitalWrite(PUL, LOW);
    delay(1);  
  }
  digitalWrite(DIR, LOW);
  for (int i = 0; i <STEPS; i++) {
    digitalWrite(PUL, HIGH);
    delay(1);
    digitalWrite(PUL, LOW);
    delay(1);  
  }
}

void loop() {
  // put your main code here, to run repeatedly:
    
}
