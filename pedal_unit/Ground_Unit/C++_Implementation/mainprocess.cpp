#include <Eigen/Dense>
#include <iostream>
#include <thread>
#include <chrono>

// extract Eigen library: https://eigen.tuxfamily.org/dox/GettingStarted.html
// sudo apt-get install libeigen3-dev
// g++ -I /usr/include/eigen3 main.cpp -o main


// Define the pins for gain control (simulated as variables)
int GAIN_PIN_1 = 0;
int GAIN_PIN_2 = 0;

using namespace Eigen;

// Function to center the data (subtract mean from each feature)
MatrixXf centerData(const MatrixXf& data) {
    MatrixXf centered = data.rowwise() - data.colwise().mean();
    return centered;
}

// Function to whiten the data using Singular Value Decomposition (SVD)
MatrixXf whitenData(const MatrixXf& data) {
    JacobiSVD<MatrixXf> svd(data, ComputeThinU | ComputeThinV);
    MatrixXf S_inv = svd.singularValues().asDiagonal().inverse();
    MatrixXf whitened = svd.matrixU() * S_inv * svd.matrixU().transpose() * data;
    return whitened;
}

// Function for the FastICA fixed-point iteration
MatrixXf fastICA(const MatrixXf& data, int num_components, int max_iter = 1000, float tol = 1e-5) {
    int n_samples = data.rows();
    int n_features = data.cols();

    // Step 1: Center and whiten the data
    MatrixXf centered_data = centerData(data);
    MatrixXf whitened_data = whitenData(centered_data);

    // Step 2: Initialize random weights
    MatrixXf W = MatrixXf::Random(num_components, n_features);

    for (int iter = 0; iter < max_iter; iter++) {
        MatrixXf W_last = W;

        // Step 3: Fixed-point iteration for maximizing non-Gaussianity
        // Compute the dot product and apply nonlinearity (g(x) = tanh(x) for FastICA)
        MatrixXf WX = W * whitened_data.transpose();
        MatrixXf gWX = WX.unaryExpr([](float x) { return std::tanh(x); });   // Nonlinear function g(x)
        MatrixXf gWX_prime = WX.unaryExpr([](float x) { return 1.0 - std::tanh(x) * std::tanh(x); }); // g'(x)

        // Update the weights W
        MatrixXf W_new = (gWX * whitened_data / n_samples).transpose() - gWX_prime.rowwise().mean().asDiagonal() * W;

        // Decorrelate the weight matrix (symmetrical decorrelation)
        JacobiSVD<MatrixXf> svd(W_new, ComputeThinU | ComputeThinV);
        W = svd.matrixU() * svd.matrixV().transpose();

        // Check for convergence
        if ((W - W_last).norm() < tol) {
            break;
        }
    }

    return W * whitened_data.transpose();
}

// Placeholder function to simulate analogWrite
void analogWrite(int& pin, int value) {
    pin = value;
    std::cout << "Pin " << &pin << " set to " << value << std::endl;
}

// Task for ICA processing and gain output
void ICAProcessingTask() {
    const int num_samples = 100; // Example value
    const int num_channels = 8;  // Example value
    float eeg_data_buffer[num_samples][num_channels]; // Example buffer

    while (true) {
        // Simulate waiting for new data to be available
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        // Convert the EEG data buffer into an Eigen matrix
        MatrixXf eeg_data(num_samples, num_channels);
        for (int i = 0; i < num_samples; i++) {
            for (int j = 0; j < num_channels; j++) {
                eeg_data(i, j) = eeg_data_buffer[i][j];
            }
        }

        // Perform FastICA on the EEG data
        MatrixXf ica_components = fastICA(eeg_data, 2);

        // Extract and normalize the first two components for gain control
        VectorXf component_1 = ica_components.col(0);
        VectorXf component_2 = ica_components.col(1);

        float gain_1 = (component_1.mean() - component_1.minCoeff()) / 
                       (component_1.maxCoeff() - component_1.minCoeff()) * 255;
        float gain_2 = (component_2.mean() - component_2.minCoeff()) / 
                       (component_2.maxCoeff() - component_2.minCoeff()) * 255;

        // Output the gains to the simulated GPIO pins
        analogWrite(GAIN_PIN_1, (int)gain_1);
        analogWrite(GAIN_PIN_2, (int)gain_2);

        // Short delay for real-time performance
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

int main() {
    // Start the ICA processing task
    ICAProcessingTask();
    return 0;
}
