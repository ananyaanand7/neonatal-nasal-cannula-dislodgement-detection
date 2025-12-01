// Pin assignments
int sensor1Pin = A0;   // analog pin for first temperature sensor
int sensor2Pin = A1;   // analog pin for second temperature sensor
int ledPin = 13;       // digital pin for LED indicator

// Circular buffer to store recent temperature readings
const int bufferSize = 10;       // number of readings to average over
int readings1[bufferSize], readings2[bufferSize];  
int index = 0;                    // current index in the buffer

// Detection parameters
int fluctuationThreshold = 15;     // max allowed temperature variation to consider sensor "still"
unsigned long stillStart = 0;      // timestamp when both sensors became thermally stable
unsigned long requiredStillness = 15000; // time (ms) temperature must remain stable before triggering LED

void setup() {
  Serial.begin(9600);              // initialize serial communication for debugging
  pinMode(ledPin, OUTPUT);         // set LED pin as output

  // Initialize temperature readings buffer with current sensor values
  int init1 = analogRead(sensor1Pin);
  int init2 = analogRead(sensor2Pin);
  for (int i = 0; i < bufferSize; i++) {
    readings1[i] = init1;  
    readings2[i] = init2;
  }
}

void loop() {
  // Read current temperature values into the buffer
  readings1[index] = analogRead(sensor1Pin);
  Serial.println(readings1[index]);   // print first sensor value for debugging
  readings2[index] = analogRead(sensor2Pin);

  // Increment index for circular buffer
  index = (index + 1) % bufferSize;

  // Find min and max readings in the buffer for both sensors
  int max1 = readings1[0], min1 = readings1[0];
  int max2 = readings2[0], min2 = readings2[0];
  for (int i = 1; i < bufferSize; i++) {
    if (readings1[i] > max1) max1 = readings1[i];
    if (readings1[i] < min1) min1 = readings1[i];
    if (readings2[i] > max2) max2 = readings2[i];
    if (readings2[i] < min2) min2 = readings2[i];
  }

  // Compute temperature range to measure fluctuation
  int range1 = max1 - min1;
  int range2 = max2 - min2;

  // Determine if either sensor has low temperature fluctuation
  bool still = (range1 < fluctuationThreshold) || (range2 < fluctuationThreshold);

  // If temperatures are stable, start timing; else, reset timer
  if (still) {
    if (stillStart == 0) stillStart = millis();  // start counting thermal stability
  } else {
    stillStart = 0;  // temperature fluctuating, reset timer
  }

  // Turn LED on if temperature remains stable for required duration
  if (stillStart != 0 && millis() - stillStart >= requiredStillness) {
    Serial.println("Cannula Displaced!");  // alert: temperature stable indicates potential dislodgement
    digitalWrite(ledPin, HIGH);            // turn LED on
  } else {
    digitalWrite(ledPin, LOW);             // keep LED off
  }

  delay(400);  // wait 400ms before next loop iteration
}
