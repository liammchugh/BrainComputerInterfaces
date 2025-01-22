#include <WiFi.h>
#include <Wire.h>
#include <Arduino.h>

const int GAIN_PIN_1 = 12;
const int GAIN_PIN_2 = 13;

// WiFi credentials
const char* ssid = "your_SSID";
const char* password = "your_PASSWORD";

// Variables for EEG data
const int num_channels = 4;
const int num_samples = 1000;  // Number of samples to store
float eeg_data[num_samples][num_channels];  // Buffer to hold EEG data
int data_index = 0;  // Keep track of the EEG data index

// Task handles
TaskHandle_t Task1;
TaskHandle_t Task2;

// Simulate receiving EEG data over WiFi
void receiveEEGData(void * parameter) {
    WiFi.begin(ssid, password);
    
    // Wait until connected
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi!");

    while (true) {
        // Simulate receiving data (replace this with actual WiFi data receive logic)
        for (int i = 0; i < num_channels; i++) {
            eeg_data[data_index][i] = random(0, 2048);  // Simulating EEG data as random values (0-2048)
        }
        data_index = (data_index + 1) % num_samples;

        delay(1);  // Simulate some delay
    }
}

// Normalize values to control gain (range: 0-255 for GPIO PWM)
float normalize(float value, float min_val, float max_val, float new_min, float new_max) {
    return (value - min_val) / (max_val - min_val) * (new_max - new_min) + new_min;
}

// ICA processing function (simplified for ESP32)
void performICA(void * parameter) {
    // Placeholder for two independent components
    float component_1[num_samples];
    float component_2[num_samples];

    // Simplified ICA processing
    while (true) {
        // Step 1: Mean center the data
        float means[num_channels] = {0};

        for (int i = 0; i < num_samples; i++) {
            for (int j = 0; j < num_channels; j++) {
                means[j] += eeg_data[i][j];
            }
        }

        for (int j = 0; j < num_channels; j++) {
            means[j] /= num_samples;
        }

        for (int i = 0; i < num_samples; i++) {
            for (int j = 0; j < num_channels; j++) {
                eeg_data[i][j] -= means[j];
            }
        }

        // Step 2: Simplified ICA
        // We will use a linear transformation approach for simplicity
        for (int i = 0; i < num_samples; i++) {
            component_1[i] = eeg_data[i][0] - eeg_data[i][1];  // Rough approximation
            component_2[i] = eeg_data[i][2] - eeg_data[i][3];  // Rough approximation
        }

        // Step 3: Normalize the components for gain control
        float min_comp1 = component_1[0], max_comp1 = component_1[0];
        float min_comp2 = component_2[0], max_comp2 = component_2[0];

        for (int i = 1; i < num_samples; i++) {
            if (component_1[i] < min_comp1) min_comp1 = component_1[i];
            if (component_1[i] > max_comp1) max_comp1 = component_1[i];

            if (component_2[i] < min_comp2) min_comp2 = component_2[i];
            if (component_2[i] > max_comp2) max_comp2 = component_2[i];
        }

        float gain_1 = normalize(component_1[num_samples-1], min_comp1, max_comp1, 0, 255);
        float gain_2 = normalize(component_2[num_samples-1], min_comp2, max_comp2, 0, 255);

        // Step 4: Apply the gain values to the GPIO pins
        analogWrite(GAIN_PIN_1, (int)gain_1);
        analogWrite(GAIN_PIN_2, (int)gain_2);

        delay(100);  // Adjust processing frequency as needed
    }
}

void setup() {
    Serial.begin(115200);

    // Initialize pins for gain control
    pinMode(GAIN_PIN_1, OUTPUT);
    pinMode(GAIN_PIN_2, OUTPUT);

    // Initialize WiFi and EEG receiving on Core 0
    xTaskCreatePinnedToCore(receiveEEGData, "Receive EEG Data", 10000, NULL, 1, &Task1, 0);

    // Initialize ICA processing and gain control on Core 1
    xTaskCreatePinnedToCore(performICA, "Perform ICA", 10000, NULL, 1, &Task2, 1);
}

void loop() {
    // Nothing to do here, tasks are running on separate cores
}
