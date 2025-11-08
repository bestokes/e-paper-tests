/**
 * ESP32 7.5b e-Paper Direct Test
 * Focused on hardware verification and basic display functionality
 */

// SPI pin definitions for ESP32
#define PIN_SPI_SCK  13
#define PIN_SPI_DIN  14
#define PIN_SPI_CS   15
#define PIN_SPI_BUSY 25
#define PIN_SPI_RST  26
#define PIN_SPI_DC   27

void setup() {
  Serial.begin(115200);
  Serial.println("ESP32 7.5b e-Paper Direct Test");
  delay(1000);
  
  // Initialize pins
  pinMode(PIN_SPI_BUSY, INPUT);
  pinMode(PIN_SPI_RST, OUTPUT);
  pinMode(PIN_SPI_DC, OUTPUT);
  pinMode(PIN_SPI_SCK, OUTPUT);
  pinMode(PIN_SPI_DIN, OUTPUT);
  pinMode(PIN_SPI_CS, OUTPUT);

  digitalWrite(PIN_SPI_CS, HIGH);
  digitalWrite(PIN_SPI_SCK, LOW);
  digitalWrite(PIN_SPI_DC, LOW);
  digitalWrite(PIN_SPI_RST, HIGH);

  Serial.println("=== HARDWARE DIAGNOSTICS ===");
  
  // Test 1: Check BUSY pin with display powered
  Serial.print("BUSY pin state (powered): ");
  Serial.println(digitalRead(PIN_SPI_BUSY));
  
  // Test 2: Perform hard reset
  Serial.println("Performing hard reset...");
  digitalWrite(PIN_SPI_RST, LOW);
  delay(200);
  digitalWrite(PIN_SPI_RST, HIGH);
  delay(500);
  
  Serial.print("BUSY pin state after reset: ");
  Serial.println(digitalRead(PIN_SPI_BUSY));
  
  // Test 3: Try basic SPI communication
  Serial.println("Testing basic SPI...");
  
  // Send a simple command to check if display responds
  digitalWrite(PIN_SPI_CS, LOW);
  digitalWrite(PIN_SPI_DC, LOW); // Command mode
  for (int i = 0; i < 8; i++) {
    digitalWrite(PIN_SPI_SCK, LOW);
    digitalWrite(PIN_SPI_DIN, (0x71 >> (7-i)) & 0x01); // Get Status command
    digitalWrite(PIN_SPI_SCK, HIGH);
  }
  digitalWrite(PIN_SPI_CS, HIGH);
  
  delay(100);
  
  Serial.print("BUSY pin after command: ");
  Serial.println(digitalRead(PIN_SPI_BUSY));
  
  // Test 4: Check if display responds to busy check
  Serial.println("Checking display responsiveness...");
  digitalWrite(PIN_SPI_CS, LOW);
  digitalWrite(PIN_SPI_DC, LOW); // Command mode
  for (int i = 0; i < 8; i++) {
    digitalWrite(PIN_SPI_SCK, LOW);
    digitalWrite(PIN_SPI_DIN, (0x71 >> (7-i)) & 0x01); // Get Status command
    digitalWrite(PIN_SPI_SCK, HIGH);
  }
  digitalWrite(PIN_SPI_CS, HIGH);
  
  // Read response
  delay(10);
  Serial.print("BUSY after status command: ");
  Serial.println(digitalRead(PIN_SPI_BUSY));
  
  Serial.println("=== DIAGNOSTIC SUMMARY ===");
  if (digitalRead(PIN_SPI_BUSY) == 0) {
    Serial.println("❌ DISPLAY NOT RESPONDING - BUSY pin stuck low");
    Serial.println("Possible causes:");
    Serial.println("1. Incorrect wiring");
    Serial.println("2. Display not powered");
    Serial.println("3. Wrong display model");
    Serial.println("4. Hardware failure");
  } else {
    Serial.println("✅ Display appears responsive");
  }
  
  Serial.println("\n=== WIRING CHECKLIST ===");
  Serial.println("ESP32 -> 7.5b Display");
  Serial.println("3.3V  -> VCC");
  Serial.println("GND   -> GND");
  Serial.println("GPIO14-> DIN");
  Serial.println("GPIO13-> CLK");
  Serial.println("GPIO15-> CS");
  Serial.println("GPIO27-> DC");
  Serial.println("GPIO26-> RST");
  Serial.println("GPIO25-> BUSY");
  
  Serial.println("\n=== NEXT STEPS ===");
  Serial.println("1. Verify ALL wiring connections");
  Serial.println("2. Check power supply (3.3V)");
  Serial.println("3. Confirm display model is 7.5b V2");
  Serial.println("4. Try different ESP32 board");
  Serial.println("5. Test with known working display");
}

void loop() {
  // Just indicate program is running
  delay(1000);
  Serial.println("Program running...");
}
