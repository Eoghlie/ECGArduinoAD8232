import processing.serial.*;

Serial myPort;
int xPos = 1;
float height_old = 0;
float height_new = 0;
float inByte = 0;

int PIXEL_SPACING = 7;

// BPM calculation variables
int BPM = 0;
float lastValue = 0;
float threshold = 325;  // Adjust if needed
boolean beatDetected = false;
long lastBeatTime = 0;
long[] beatTimes = new long[10];
int beatIndex = 0;

void setup () {
  size(1000, 400);
  
  println(Serial.list());
  myPort = new Serial(this, Serial.list()[1], 9600);
  
  background(0);
  
  // Initialize beat times
  for(int i = 0; i < 10; i++) {
    beatTimes[i] = 0;
  }
}

void draw () {
  while (myPort.available() > 0) {
    String inString = myPort.readStringUntil('\n');
    
    if (inString != null) {
      inString = trim(inString);
      inByte = float(inString);
      
      // Detect beats for BPM
      if(inByte > threshold && lastValue <= threshold && !beatDetected) {
        beatDetected = true;
        long currentTime = millis();
        
        if(currentTime - lastBeatTime > 300) {  // Debounce
          beatTimes[beatIndex] = currentTime;
          beatIndex = (beatIndex + 1) % 10;
          
          // Calculate BPM from last beats
          int validBeats = 0;
          long totalInterval = 0;
          
          for(int i = 0; i < 9; i++) {
            if(beatTimes[i] > 0 && beatTimes[i+1] > 0) {
              long interval = beatTimes[i+1] - beatTimes[i];
              if(interval > 300 && interval < 2000) {
                totalInterval += interval;
                validBeats++;
              }
            }
          }
          
          if(validBeats > 0) {
            BPM = (int)((60000.0 * validBeats) / totalInterval);
          }
          
          lastBeatTime = currentTime;
        }
      }
      
      if(inByte < threshold) {
        beatDetected = false;
      }
      lastValue = inByte;
      
      // Map signal to display
      height_new = map(inByte, 220, 360, 0, height);
      
      stroke(0, 255, 0);
      strokeWeight(2);
      line(xPos - PIXEL_SPACING, height - height_old, xPos, height - height_new);
      
      height_old = height_new;
      
      xPos += PIXEL_SPACING;
      
      if (xPos >= width) {
        xPos = 0;
        background(0);
      }
    }
  }
  
  // Draw grid lines
  stroke(50);
  strokeWeight(1);
  for(int i = 0; i < height; i += 50) {
    line(0, i, width, i);
  }
  
  // Draw BPM display with background clear
  fill(0);  // Black rectangle to clear old text
  noStroke();
  rect(10, 10, 150, 40);  // Clear the text area
  
  fill(0, 200, 0);  // Green text
  textSize(24);
  textAlign(LEFT);
  text(BPM + " BPM", 20, 40);
}
