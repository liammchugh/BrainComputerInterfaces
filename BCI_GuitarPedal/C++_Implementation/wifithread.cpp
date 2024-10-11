#include <WiFi.h>
#include <Eigen/Dense>
#include <FreeRTOS.h>

// WiFi credentials
const char* ssid = "your_SSID";
const char* password = "your_PASSWORD";

// Variables to store EEG data
const int num_channels = 4;
const int num_samples = 1000;
float eeg_data_buffer[num_samples][num_channels];

// FreeRTOS handle for the processing task
TaskHandle_t processingTaskHandle;

// Wi-Fi EEG Data Receiving Task
void WiFiReceiveTask(void* parameter) {
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi!");

    while (true) {
        // Simulate receiving EEG data via Wi-Fi
        for (int i = 0; i < num_samples; i++) {
            for (int j = 0; j < num_channels; j++) {
                eeg_data_buffer[i][j] = random(0, 2048);  // Replace with actual Wi-Fi receive code
            }
        }

        // Notify processing task that new data is available
        xTaskNotifyGive(processingTaskHandle);
        delay(1);  // Simulate a short delay between data reception
    }
}
