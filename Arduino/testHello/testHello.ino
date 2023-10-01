void setup() {
  Serial.begin(57600);
  pinMode(LED_BUILTIN, OUTPUT); // Set the onboard LED pin as an OUTPUT
}

void loop() {
  Serial.println("Hello from Arduino!");

  digitalWrite(LED_BUILTIN, HIGH); // Turn on the onboard LED
  delay(1000); // Wait for 1 second

  digitalWrite(LED_BUILTIN, LOW); // Turn off the onboard LED
  delay(1000); // Wait for 1 second
}
