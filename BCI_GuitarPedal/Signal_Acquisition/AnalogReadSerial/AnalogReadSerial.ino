// Define variables
const int emgPin = 35;       // Analog input pin for EMG
const int numReadings = 4;   // Number of readings for moving average
int emg_hist[numReadings] = {0, 0, 0, 0};  // Array to store history of readings

void setup() {
  // Initialize serial communication at 115200 baud.
  Serial.begin(115200);
}

void loop() {
  // Read the analog value from the specified pin.
  int emgVal = analogRead(emgPin);

  // Shift the history array to the left to remove the oldest reading.
  for (int i = 0; i < numReadings - 1; i++) {
    emg_hist[i] = emg_hist[i + 1];
  }
  // Add the newest reading to the end of the array.
  emg_hist[numReadings - 1] = emgVal;

  // Calculate the sum of the values in the history.
  long sum = 0;
  for (int i = 0; i < numReadings; i++) {
    sum += emg_hist[i];
  }
  // Compute the moving average.
  int avg = sum / numReadings;

  // Print the moving average to the Serial Monitor.
  Serial.print(emgVal);
  Serial.print(" ");
  Serial.println(avg);

  // Wait a short period before reading again.
  delay(25);
}
