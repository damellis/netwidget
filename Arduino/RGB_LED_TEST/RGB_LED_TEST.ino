int R = 6, G = 10, B = 5;

void setup() {
  Serial.begin(9600);
}

void loop() {
  Serial.println("Red");
  analogWrite(R, 255);
  delay(3000);
  analogWrite(R, 0);

  Serial.println("Green");
  analogWrite(G, 255);
  delay(3000);
  analogWrite(G, 0);

  Serial.println("Blue");
  analogWrite(B, 255);
  delay(3000);
  analogWrite(B, 0);

  Serial.println("All");
  analogWrite(R, 255);
  analogWrite(G, 255);
  analogWrite(B, 255);
  delay(3000);
  
  Serial.println("None");
  analogWrite(R, 0);
  analogWrite(G, 0);
  analogWrite(B, 0);
  delay(3000);
}
