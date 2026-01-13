/*
 * DIY ECG Monitor - Working Version 1
 * Signal inverted, basic filtering, displays waveform and BPM
 * Save this version before experimenting!
 */

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define AD8232_OUTPUT A0
#define LO_PLUS 10
#define LO_MINUS 11

int Signal;
int BPM = 0;
int Threshold = 340;

int xPos = 0;
int lastY = 16;
unsigned long beatTimes[10];
int beatIndex = 0;

int filterBuffer[3];
int filterIndex = 0;

void setup() {
  Serial.begin(9600);
  
  pinMode(LO_PLUS, INPUT);
  pinMode(LO_MINUS, INPUT);
  
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 failed"));
    for(;;);
  }
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("ECG Monitor");
  display.println("Ready...");
  display.display();
  delay(2000);
  
  for(int i = 0; i < 10; i++) {
    beatTimes[i] = 0;
  }
  
  for(int i = 0; i < 3; i++) {
    filterBuffer[i] = 350;
  }
}

void loop() {
  if((digitalRead(LO_PLUS) == 1) || (digitalRead(LO_MINUS) == 1)) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 10);
    display.println("Check electrodes!");
    display.display();
    Serial.println("!");
    xPos = 0;
    return;
  }
  
  int rawSignal = analogRead(AD8232_OUTPUT);
  
  // Light filtering
  filterBuffer[filterIndex] = rawSignal;
  filterIndex = (filterIndex + 1) % 3;
  Signal = (filterBuffer[0] + filterBuffer[1] + filterBuffer[2]) / 3;
  
  // INVERT THE SIGNAL
  int invertedSignal = 670 - Signal;
  
  // Map inverted signal to display
  int yPos = map(invertedSignal, 280, 380, 30, 2);
  yPos = constrain(yPos, 1, 31);
  
  // Detect heartbeat - look for DIPS
  static bool beatDetected = false;
  static int lastSignal = 0;
  
  if (Signal < Threshold && lastSignal >= Threshold && !beatDetected) {
    beatDetected = true;
    unsigned long currentTime = millis();
    
    beatTimes[beatIndex] = currentTime;
    beatIndex = (beatIndex + 1) % 10;
    
    int validBeats = 0;
    unsigned long totalInterval = 0;
    
    for(int i = 0; i < 9; i++) {
      if(beatTimes[i] > 0 && beatTimes[i+1] > 0) {
        unsigned long interval = beatTimes[i+1] - beatTimes[i];
        if(interval > 300 && interval < 2000) {
          totalInterval += interval;
          validBeats++;
        }
      }
    }
    
    if(validBeats > 0) {
      BPM = (60000 * validBeats) / totalInterval;
    }
  }
  
  if (Signal > Threshold) {
    beatDetected = false;
  }
  lastSignal = Signal;
  
  // Draw waveform
  if(xPos == 0) {
    display.clearDisplay();
  }
  
  display.drawLine(xPos - 1, lastY, xPos, yPos, SSD1306_WHITE);
  lastY = yPos;
  
  // Draw BPM
  display.fillRect(0, 0, 50, 8, SSD1306_BLACK);
  display.setTextSize(1);
  display.setCursor(0, 0);
  if(BPM > 40 && BPM < 200) {
    display.print(BPM);
    display.print(" BPM");
  } else {
    display.print("-- BPM");
  }
  
  display.display();
  
  xPos++;
  
  if(xPos >= SCREEN_WIDTH) {
    xPos = 0;
  }
  
  Serial.println(invertedSignal);
  
  delay(10);
}
