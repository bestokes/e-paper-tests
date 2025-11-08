/**
 * ESP32 7.5b e-Paper Display Troubleshooting
 * Enhanced version with debugging to help identify display issues
 */

// SPI pin definitions for ESP32
#define PIN_SPI_SCK  13
#define PIN_SPI_DIN  14
#define PIN_SPI_CS   15
#define PIN_SPI_BUSY 25
#define PIN_SPI_RST  26
#define PIN_SPI_DC   27

// Pin level definitions
#define LOW             0
#define HIGH            1
#define GPIO_PIN_SET   1
#define GPIO_PIN_RESET 0

// Type definitions
typedef unsigned char UBYTE;
typedef unsigned int UWORD;

// Display resolution for 7.5b
#define EPD_WIDTH  800
#define EPD_HEIGHT 480

// Look-up tables for 7.5b V2 display
UBYTE Voltage_Frame_7IN5_V2[] = {
  0x6, 0x3F, 0x3F, 0x11, 0x24, 0x7, 0x17,
};

UBYTE LUT_VCOM_7IN5_V2[] = {  
  0x0,  0xF,  0xF,  0x0,  0x0,  0x1,  
  0x0,  0xF,  0x1,  0xF,  0x1,  0x2,  
  0x0,  0xF,  0xF,  0x0,  0x0,  0x1,  
  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  
  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  
  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  
  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  
};            

UBYTE LUT_WW_7IN5_V2[] = {  
  0x10,  0xF,  0xF,  0x0,  0x0,  0x1,  
  0x84,  0xF,  0x1,  0xF,  0x1,  0x2,  
  0x20,  0xF,  0xF,  0x0,  0x0,  0x1,  
  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  
  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  
  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  
  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  
};

UBYTE LUT_BW_7IN5_V2[] = {  
  0x10,  0xF,  0xF,  0x0,  0x0,  0x1,  
  0x84,  0xF,  0x1,  0xF,  0x1,  0x2,  
  0x20,  0xF,  0xF,  0x0,  0x0,  0x1,  
  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  
  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  
  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  
  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  
};

UBYTE LUT_WB_7IN5_V2[] = {  
  0x80,  0xF,  0xF,  0x0,  0x0,  0x3,  
  0x84,  0xF,  0x1,  0xF,  0x1,  0x4,  
  0x40,  0xF,  0xF,  0x0,  0x0,  0x3,  
  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  
  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  
  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  
  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  
};

UBYTE LUT_BB_7IN5_V2[] = {  
  0x80,  0xF,  0xF,  0x0,  0x0,  0x1,  
  0x84,  0xF,  0x1,  0xF,  0x1,  0x2,  
  0x40,  0xF,  0xF,  0x0,  0x0,  0x1,  
  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  
  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  
  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  
  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  
};

// Function prototypes
void EPD_initSPI();
void GPIO_Mode(unsigned char GPIO_Pin, unsigned char Mode);
void EpdSpiTransferCallback(UBYTE data);
void EPD_SendCommand(UBYTE command);
void EPD_SendData(UBYTE data);
void EPD_WaitUntilIdle();
void EPD_Reset();
static void EPD_7in5_V2_Readbusy();
static void EPD_7IN5_V2_LUT(UBYTE* lut_vcom, UBYTE* lut_ww, UBYTE* lut_bw, UBYTE* lut_wb, UBYTE* lut_bb);
int EPD_7in5B_V2_Init();
static void EPD_7IN5_V2_Show();
void clearDisplay();
void testPins();
void sendTestPattern();

// Initialize SPI pins
void EPD_initSPI() {
    Serial.println("Initializing SPI pins...");
    
    pinMode(PIN_SPI_BUSY, INPUT);
    pinMode(PIN_SPI_RST, OUTPUT);
    pinMode(PIN_SPI_DC, OUTPUT);
    
    pinMode(PIN_SPI_SCK, OUTPUT);
    pinMode(PIN_SPI_DIN, OUTPUT);
    pinMode(PIN_SPI_CS, OUTPUT);

    digitalWrite(PIN_SPI_CS, HIGH);
    digitalWrite(PIN_SPI_SCK, LOW);
    
    Serial.println("SPI pins initialized");
}

// Set GPIO pin mode
void GPIO_Mode(unsigned char GPIO_Pin, unsigned char Mode) {
    if(Mode == 0) {
        pinMode(GPIO_Pin, INPUT);
    } else {
        pinMode(GPIO_Pin, OUTPUT);
    }
}

// SPI transfer function
void EpdSpiTransferCallback(UBYTE data) {
    digitalWrite(PIN_SPI_CS, GPIO_PIN_RESET);

    for (int i = 0; i < 8; i++) {
        if ((data & 0x80) == 0) digitalWrite(PIN_SPI_DIN, GPIO_PIN_RESET); 
        else                    digitalWrite(PIN_SPI_DIN, GPIO_PIN_SET);

        data <<= 1;
        digitalWrite(PIN_SPI_SCK, GPIO_PIN_SET);
        digitalWrite(PIN_SPI_SCK, GPIO_PIN_RESET);
    }
    
    digitalWrite(PIN_SPI_CS, GPIO_PIN_SET);
}

// Send command to display
void EPD_SendCommand(UBYTE command) {
    digitalWrite(PIN_SPI_DC, LOW);
    EpdSpiTransferCallback(command);
}

// Send data to display
void EPD_SendData(UBYTE data) {
    digitalWrite(PIN_SPI_DC, HIGH);
    EpdSpiTransferCallback(data);
}

// Wait until display is not busy
void EPD_WaitUntilIdle() {
    Serial.println("Waiting for display to be ready...");
    unsigned long startTime = millis();
    while(digitalRead(PIN_SPI_BUSY) == 0) {
        delay(10);
        if (millis() - startTime > 10000) { // 10 second timeout
            Serial.println("TIMEOUT: Display busy signal stuck low");
            break;
        }
    }
    Serial.println("Display ready");
}

// Reset display
void EPD_Reset() {
    Serial.println("Resetting display...");
    digitalWrite(PIN_SPI_RST, HIGH); 
    delay(200);    
    digitalWrite(PIN_SPI_RST, LOW);    
    delay(10); // Increased from 5ms
    digitalWrite(PIN_SPI_RST, HIGH); 
    delay(200);    
    Serial.println("Display reset complete");
}

// Read busy state for V2 display
static void EPD_7in5_V2_Readbusy() {
    Serial.println("Checking display busy state...");
    unsigned char busy;
    unsigned long startTime = millis();
    do {
        EPD_SendCommand(0x71);
        busy = digitalRead(PIN_SPI_BUSY);
        busy =!(busy & 0x01);
        if (millis() - startTime > 30000) { // 30 second timeout
            Serial.println("TIMEOUT: Display busy check timeout");
            break;
        }
    } while(busy);   
    delay(200);      
    Serial.println("Display busy check complete");
}

// Load LUT data
static void EPD_7IN5_V2_LUT(UBYTE* lut_vcom, UBYTE* lut_ww, UBYTE* lut_bw, UBYTE* lut_wb, UBYTE* lut_bb) {
    Serial.println("Loading LUT data...");
    UBYTE count;

    EPD_SendCommand(0x20); // VCOM    
    for(count = 0; count < 60; count++)
        EPD_SendData(lut_vcom[count]);

    EPD_SendCommand(0x21); // LUTWW
    for(count = 0; count < 60; count++)
        EPD_SendData(lut_ww[count]);

    EPD_SendCommand(0x22); // LUTBW
    for(count = 0; count < 60; count++)
        EPD_SendData(lut_bw[count]);

    EPD_SendCommand(0x23); // LUTWB
    for(count = 0; count < 60; count++)
        EPD_SendData(lut_wb[count]);

    EPD_SendCommand(0x24); // LUTBB
    for(count = 0; count < 60; count++)
        EPD_SendData(lut_bb[count]);
    
    Serial.println("LUT data loaded");
}

// Initialize 7.5b V2 display
int EPD_7in5B_V2_Init() {
    Serial.println("Starting 7.5b V2 display initialization...");
    EPD_Reset();

    EPD_SendCommand(0x01);            // POWER SETTING
    EPD_SendData(0x07);
    EPD_SendData(0x07);               // VGH=20V,VGL=-20V
    EPD_SendData(0x3f);               // VDH=15V
    EPD_SendData(0x3f);               // VDL=-15V

    EPD_SendCommand(0x04);            // POWER ON
    delay(100);
    EPD_7in5_V2_Readbusy();

    EPD_SendCommand(0X00);            // PANNEL SETTING
    EPD_SendData(0x0F);               // KW-3f   KWR-2F BWROTP 0f BWOTP 1f

    EPD_SendCommand(0x61);            // tres
    EPD_SendData(0x03);               // source 800
    EPD_SendData(0x20);
    EPD_SendData(0x01);               // gate 480
    EPD_SendData(0xE0);

    EPD_SendCommand(0X15);
    EPD_SendData(0x00);

    EPD_SendCommand(0X50);            // VCOM AND DATA INTERVAL SETTING
    EPD_SendData(0x11);
    EPD_SendData(0x07);

    EPD_SendCommand(0X60);            // TCON SETTING
    EPD_SendData(0x22);

    EPD_SendCommand(0x65);            // Resolution setting
    EPD_SendData(0x00);
    EPD_SendData(0x00);               // 800*480
    EPD_SendData(0x00);
    EPD_SendData(0x00);
    
    // Load LUT data
    EPD_7IN5_V2_LUT(LUT_VCOM_7IN5_V2, LUT_WW_7IN5_V2, LUT_BW_7IN5_V2, LUT_WB_7IN5_V2, LUT_BB_7IN5_V2);
    
    UWORD i;
    EPD_SendCommand(0x10);            // First buffer (black)
    Serial.println("Clearing black buffer...");
    for(i = 0; i < EPD_WIDTH / 8 * EPD_HEIGHT; i++) {
        EPD_SendData(0x00); // Black
    }
    
    EPD_SendCommand(0x13);            // Second buffer (red)
    Serial.println("Clearing red buffer...");
    for(i = 0; i < EPD_WIDTH / 8 * EPD_HEIGHT; i++) {
        EPD_SendData(0x00); // No red
    }

    EPD_SendCommand(0x10);            // Switch back to first buffer
    Serial.println("7.5b V2 display initialization complete");
    return 0;
}

// Show display and enter sleep mode
static void EPD_7IN5_V2_Show() {
    Serial.println("Refreshing display...");
    EPD_SendCommand(0x12);            // DISPLAY REFRESH
    delay(100);                       // Delay necessary, 200uS at least
    EPD_7in5_V2_Readbusy();

    Serial.println("Entering sleep mode...");
    // Enter sleep mode
    EPD_SendCommand(0X02);            // power off
    EPD_7in5_V2_Readbusy();
    EPD_SendCommand(0X07);            // deep sleep
    EPD_SendData(0xA5);
    Serial.println("Display in sleep mode");
}

// Clear display (fill with white)
void clearDisplay() {
    Serial.println("Clearing display to white...");
    EPD_SendCommand(0x10);  // Black buffer
    for(UWORD i = 0; i < EPD_WIDTH / 8 * EPD_HEIGHT; i++) {
        EPD_SendData(0xFF);  // All white
    }
    
    EPD_SendCommand(0x13);  // Red buffer  
    for(UWORD i = 0; i < EPD_WIDTH / 8 * EPD_HEIGHT; i++) {
        EPD_SendData(0x00);  // All white (no red)
    }
    
    EPD_SendCommand(0x10);  // Switch back to black buffer
    Serial.println("Display cleared to white");
}

// Send test pattern
void sendTestPattern() {
    Serial.println("Sending test pattern...");
    EPD_SendCommand(0x10);  // Switch to black buffer
    
    // Create a checkerboard pattern
    for(UWORD i = 0; i < EPD_WIDTH / 8 * EPD_HEIGHT; i++) {
        // Alternate between black and white every 8 pixels
        if ((i / (EPD_WIDTH / 8)) % 2 == 0) {
            EPD_SendData((i % 2 == 0) ? 0x00 : 0xFF);  // Black/white checkerboard
        } else {
            EPD_SendData((i % 2 == 0) ? 0xFF : 0x00);  // White/black checkerboard
        }
    }
    Serial.println("Test pattern sent");
}

// Test GPIO pins
void testPins() {
    Serial.println("Testing GPIO pins...");
    
    // Test RST pin
    digitalWrite(PIN_SPI_RST, HIGH);
    Serial.println("RST pin set HIGH");
    delay(1000);
    digitalWrite(PIN_SPI_RST, LOW);
    Serial.println("RST pin set LOW");
    delay(1000);
    digitalWrite(PIN_SPI_RST, HIGH);
    Serial.println("RST pin set HIGH again");
    
    // Test DC pin
    digitalWrite(PIN_SPI_DC, HIGH);
    Serial.println("DC pin set HIGH");
    delay(500);
    digitalWrite(PIN_SPI_DC, LOW);
    Serial.println("DC pin set LOW");
    
    // Test CS pin
    digitalWrite(PIN_SPI_CS, LOW);
    Serial.println("CS pin set LOW");
    delay(500);
    digitalWrite(PIN_SPI_CS, HIGH);
    Serial.println("CS pin set HIGH");
    
    // Read BUSY pin
    int busyState = digitalRead(PIN_SPI_BUSY);
    Serial.print("BUSY pin state: ");
    Serial.println(busyState);
    
    Serial.println("GPIO pin test complete");
}

// Arduino setup function
void setup() {
    Serial.begin(115200);
    Serial.println("ESP32 7.5b e-Paper Troubleshooting Starting...");
    delay(1000);
    
    // Test GPIO pins first
    testPins();
    delay(2000);
    
    // Initialize SPI and display
    EPD_initSPI();
    
    // Initialize the 7.5b display
    Serial.println("Attempting display initialization...");
    if (EPD_7in5B_V2_Init() == 0) {
        Serial.println("Display initialized successfully");
    } else {
        Serial.println("Display initialization failed");
        return;
    }
    
    // Clear display (white background)
    clearDisplay();
    
    // Send test pattern
    sendTestPattern();
    
    // Refresh display to show the pattern
    EPD_7IN5_V2_Show();
    
    Serial.println("Troubleshooting demo completed");
    Serial.println("Check Serial Monitor for any error messages");
}

// Arduino loop function
void loop() {
    // Nothing to do here - display maintains image without power
    delay(5000);
    Serial.println("Still running...");
}
