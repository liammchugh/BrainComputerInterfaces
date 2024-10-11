#include <Eigen/Dense>
#include <FreeRTOS.h>
#include <Arduino.h>

// Define the pins for gain control
const int GAIN_PIN_1 = 12;
const int GAIN_PIN_2 = 13;

// FastICA function using Eigen
Eigen::MatrixXf performFastICA(Eigen::MatrixXf eeg_data) {
    // Simplified FastICA algorithm using Eigen
    // Initialize ICA matrices, whiten data, and iterate to find independent components
    // Eigen supports matrix operations which can be used to handle the ICA calculations.

    // Here you would implement the full ICA algorithm or use a library that supports FastICA.
    // This is a placeholder. You'll need to handle whitening, decorrelation, and convergence.

    Eigen::MatrixXf ica_components = eeg_data;  // Placeholder
    return ica_components;
}

// Task for ICA processing and gain output
void ICAProcessingTask(void* parameter) {
    while (true) {
        // Wait for new data to be available
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        // Convert the EEG data buffer into an Eigen matrix
        Eigen::MatrixXf eeg_data(num_samples, num_channels);
        for (int i = 0; i < num_samples; i++) {
            for (int j = 0; j < num_channels; j++) {
                eeg_data(i, j) = eeg_data_buffer[i][j];
            }
        }

        // Perform FastICA on the EEG data
        Eigen::MatrixXf ica_components = performFastICA(eeg_data);

        // Extract and normalize the first two components for gain control
        Eigen::VectorXf component_1 = ica_components.col(0);
        Eigen::VectorXf component_2 = ica_components.col(1);

        float gain_1 = (component_1.mean() - component_1.minCoeff()) / 
                       (component_1.maxCoeff() - component_1.minCoeff()) * 255;
        float gain_2 = (component_2.mean() - component_2.minCoeff()) / 
                       (component_2.maxCoeff() - component_2.minCoeff()) * 255;

        // Output the gains to the GPIO pins
        analogWrite(GAIN_PIN_1, (int)gain_1);
        analogWrite(GAIN_PIN_2, (int)gain_2);

        // Short delay for real-time performance
        delay(100);
    }
}
