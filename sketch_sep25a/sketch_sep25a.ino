void setup() {
  Serial.begin(9600);  // Initialize serial communication at 9600 baud rate
}

void loop() {
  Serial.print("Hello from Arduino!");            // Prints without a new line
  Serial.println("This is a new line message.");  // Prints with a new line
  delay(1000);                                    // Wait for one second
}