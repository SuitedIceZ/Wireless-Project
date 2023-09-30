void setup() {
  // Initialize the built-in LED pin as an output
  pinMode(13, OUTPUT);
}

void loop() {
  // Turn the built-in LED on
  digitalWrite(13, HIGH);
  delay(1000);  // Wait for 1 second (1000 milliseconds)

  // Turn the built-in LED off
  digitalWrite(13, LOW);
  delay(1000);  // Wait for 1 second (1000 milliseconds)
}
