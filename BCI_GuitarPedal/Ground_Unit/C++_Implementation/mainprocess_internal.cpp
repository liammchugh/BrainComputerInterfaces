#include <iostream>
#include <vector>
#include <cmath>
#include <random>
#include <chrono>
#include <thread>

// -----------------------------------------------------------------------------
// Basic Matrix structure and utility functions
// -----------------------------------------------------------------------------
struct Matrix {
    int rows;
    int cols;
    std::vector<float> data; // row-major

    Matrix(int r, int c) : rows(r), cols(c), data(r * c, 0.0f) {}
};

float& at(Matrix& M, int r, int c) {
    return M.data[r * M.cols + c];
}

float  at(const Matrix& M, int r, int c) {
    return M.data[r * M.cols + c];
}

// Create a random matrix of size (rows x cols)
Matrix randomMatrix(int rows, int cols, float min_val = -1.0f, float max_val = 1.0f) {
    static std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<float> dist(min_val, max_val);

    Matrix mat(rows, cols);
    for (int i = 0; i < rows * cols; i++) {
        mat.data[i] = dist(rng);
    }
    return mat;
}

// Print matrix (for debugging)
void printMatrix(const Matrix& M, const std::string& name = "Matrix") {
    std::cout << name << " (" << M.rows << "x" << M.cols << "):\n";
    for (int r = 0; r < M.rows; r++) {
        for (int c = 0; c < M.cols; c++) {
            std::cout << at(M, r, c) << " ";
        }
        std::cout << "\n";
    }
    std::cout << std::endl;
}

// -----------------------------------------------------------------------------
// Matrix Arithmetic
// -----------------------------------------------------------------------------

// Matrix transpose
Matrix transpose(const Matrix& A) {
    Matrix T(A.cols, A.rows);
    for (int r = 0; r < A.rows; r++) {
        for (int c = 0; c < A.cols; c++) {
            at(T, c, r) = at(A, r, c);
        }
    }
    return T;
}

// Matrix multiplication: C = A * B
Matrix matMul(const Matrix& A, const Matrix& B) {
    // A: (m x n), B: (n x p) -> C: (m x p)
    if (A.cols != B.rows) {
        throw std::runtime_error("matMul: dimension mismatch");
    }
    Matrix C(A.rows, B.cols);
    for (int i = 0; i < A.rows; i++) {
        for (int j = 0; j < B.cols; j++) {
            float sum = 0.0f;
            for (int k = 0; k < A.cols; k++) {
                sum += at(A, i, k) * at(B, k, j);
            }
            at(C, i, j) = sum;
        }
    }
    return C;
}

// Scalar divide a matrix (element-wise)
Matrix matDiv(const Matrix& A, float val) {
    Matrix R(A.rows, A.cols);
    for (int i = 0; i < A.rows * A.cols; i++) {
        R.data[i] = A.data[i] / val;
    }
    return R;
}

// Subtract one matrix from another (element-wise)
Matrix matSub(const Matrix& A, const Matrix& B) {
    if (A.rows != B.rows || A.cols != B.cols) {
        throw std::runtime_error("matSub: dimension mismatch");
    }
    Matrix R(A.rows, A.cols);
    for (int i = 0; i < A.rows * A.cols; i++) {
        R.data[i] = A.data[i] - B.data[i];
    }
    return R;
}

// Add one matrix to another (element-wise)
Matrix matAdd(const Matrix& A, const Matrix& B) {
    if (A.rows != B.rows || A.cols != B.cols) {
        throw std::runtime_error("matAdd: dimension mismatch");
    }
    Matrix R(A.rows, A.cols);
    for (int i = 0; i < A.rows * A.cols; i++) {
        R.data[i] = A.data[i] + B.data[i];
    }
    return R;
}

// Create an Identity matrix
Matrix identity(int size) {
    Matrix I(size, size);
    for (int i = 0; i < size; i++) {
        at(I, i, i) = 1.0f;
    }
    return I;
}

// Matrix Frobenius norm
float frobeniusNorm(const Matrix& A) {
    float sumSq = 0.0f;
    for (int i = 0; i < A.rows * A.cols; i++) {
        sumSq += A.data[i] * A.data[i];
    }
    return std::sqrt(sumSq);
}

// -----------------------------------------------------------------------------
// Mean / Centering / Covariance
// -----------------------------------------------------------------------------

// Compute the mean of each column (returns 1 x cols)
Matrix columnMean(const Matrix& data) {
    Matrix mean(1, data.cols);
    for (int c = 0; c < data.cols; c++) {
        float sum = 0.0f;
        for (int r = 0; r < data.rows; r++) {
            sum += at(data, r, c);
        }
        at(mean, 0, c) = sum / data.rows;
    }
    return mean;
}

// Center the data (subtract column-wise mean)
Matrix centerData(const Matrix& data) {
    Matrix centered(data.rows, data.cols);
    Matrix colMean = columnMean(data);
    for (int r = 0; r < data.rows; r++) {
        for (int c = 0; c < data.cols; c++) {
            at(centered, r, c) = at(data, r, c) - at(colMean, 0, c);
        }
    }
    return centered;
}

// Compute covariance matrix = (1/N) * (X^T * X), assuming X is already centered
Matrix covariance(const Matrix& X) {
    // X: (n_samples x n_features)
    // Cov: (n_features x n_features) = (1/n_samples) * (X^T * X)
    Matrix Xt = transpose(X);
    Matrix XtX = matMul(Xt, X);
    Matrix Cov = matDiv(XtX, static_cast<float>(X.rows));
    return Cov;
}

// -----------------------------------------------------------------------------
// Jacobi EVD for Symmetric Matrices
//   We use it for (symmetric) covariance or (symmetric) W*W^T, etc.
//   This finds A = V * D * V^T
// -----------------------------------------------------------------------------
void jacobiEVD(const Matrix& A, Matrix& V, Matrix& D, int maxIter = 100, float tol = 1e-6f) {
    // Check square
    if (A.rows != A.cols) {
        throw std::runtime_error("jacobiEVD: A must be square");
    }
    int n = A.rows;
    V = identity(n);

    // Copy A into D initially (we'll turn D into the diagonal of eigenvalues).
    D = A; // We'll transform D into the diagonal form via Jacobi rotations.

    for (int iter = 0; iter < maxIter; iter++) {
        // 1. Find the largest off-diagonal element in D
        float maxVal = 0.0f;
        int p = 0, q = 0;
        for (int i = 0; i < n; i++) {
            for (int j = i + 1; j < n; j++) {
                float val = std::fabs(at(D, i, j));
                if (val > maxVal) {
                    maxVal = val;
                    p = i;
                    q = j;
                }
            }
        }
        // Check convergence
        if (maxVal < tol) {
            break;
        }

        float app = at(D, p, p);
        float aqq = at(D, q, q);
        float apq = at(D, p, q);

        // 2. Compute the angle theta
        float theta = 0.0f;
        if (std::fabs(apq) > 1e-12f) {
            theta = 0.5f * std::atan2(2.0f * apq, (app - aqq));
        }

        float c = std::cos(theta);
        float s = std::sin(theta);

        // 3. Update matrix D with rotation
        // We'll rotate p,q rows and columns
        float app_new = c*c*app - 2.0f*c*s*apq + s*s*aqq;
        float aqq_new = s*s*app + 2.0f*c*s*apq + c*c*aqq;
        at(D, p, p) = app_new;
        at(D, q, q) = aqq_new;
        at(D, p, q) = 0.0f;
        at(D, q, p) = 0.0f;

        for (int i = 0; i < n; i++) {
            if (i == p || i == q) continue;
            float aip = at(D, i, p);
            float aiq = at(D, i, q);
            float dip = c*aip - s*aiq;
            float diq = s*aip + c*aiq;
            at(D, i, p) = dip;
            at(D, p, i) = dip;
            at(D, i, q) = diq;
            at(D, q, i) = diq;
        }

        // 4. Update eigenvector matrix V
        for (int i = 0; i < n; i++) {
            float vip = at(V, i, p);
            float viq = at(V, i, q);
            at(V, i, p) = c*vip - s*viq;
            at(V, i, q) = s*vip + c*viq;
        }
    }

    // After convergence, D is symmetrical. The diagonal elements of D are eigenvalues.
    // The columns of V are the eigenvectors. 
    // We usually want D as diagonal only, but it's already mostly diagonal. We'll keep as is.
}

// Extract diagonal as a vector (nx1) from a square matrix
Matrix diagVector(const Matrix& M) {
    Matrix vec(M.rows, 1);
    for (int i = 0; i < M.rows; i++) {
        at(vec, i, 0) = at(M, i, i);
    }
    return vec;
}

// Construct a diagonal matrix from a vector
Matrix diagMatrix(const Matrix& vec) {
    Matrix D(vec.rows, vec.rows);
    for (int i = 0; i < vec.rows; i++) {
        at(D, i, i) = at(vec, i, 0);
    }
    return D;
}

// Take element-wise sqrt of a (column) vector (for eigenvalues)
Matrix sqrtVector(const Matrix& vec) {
    Matrix out(vec.rows, 1);
    for (int i = 0; i < vec.rows; i++) {
        out.data[i] = std::sqrt(vec.data[i]);
    }
    return out;
}

// Invert each element of a (column) vector (for 1 / sqrt(eigs))
Matrix invVector(const Matrix& vec) {
    Matrix out(vec.rows, 1);
    for (int i = 0; i < vec.rows; i++) {
        out.data[i] = (vec.data[i] == 0.0f) ? 0.0f : 1.0f / vec.data[i];
    }
    return out;
}

// -----------------------------------------------------------------------------
// Whitening via EVD of covariance
//   whitened = E * D^{-1/2} * E^T * centered_data
// -----------------------------------------------------------------------------
Matrix whitenData(const Matrix& data_centered) {
    // 1. Covariance
    Matrix Cov = covariance(data_centered);  // shape: (n_features x n_features)

    // 2. EVD on Cov => Cov = V * D * V^T
    Matrix V(0,0), D(0,0);
    jacobiEVD(Cov, V, D);

    // 3. D^{-1/2}
    Matrix diagVals = diagVector(D);    // n_features x 1
    Matrix sqrtVals = sqrtVector(diagVals); // sqrt of eigenvalues
    Matrix invSqrtVals = invVector(sqrtVals); // 1 / sqrt(eigenvalues)
    Matrix D_inv_sqrt = diagMatrix(invSqrtVals); // n_features x n_features

    // 4. Whiten: X_whiten = V * D^{-1/2} * V^T * X_centered^T
    Matrix Vt = transpose(V);
    Matrix temp = matMul(V, D_inv_sqrt);
    Matrix whiteningMat = matMul(temp, Vt); // (n_features x n_features)

    // Now, data_centered is (n_samples x n_features).
    // We want the whitened data in shape (n_features x n_samples) typically for ICA.
    // Let's do: whitened = whiteningMat * data_centered^T
    Matrix dataT = transpose(data_centered);  // (n_features x n_samples)
    Matrix whitened = matMul(whiteningMat, dataT);  // (n_features x n_samples)
    return whitened;
}

// -----------------------------------------------------------------------------
// Symmetric Decorrelation for W (like the "symmetric decorrelation" in FastICA)
//   W -> W * (W^T W)^{-1/2}
// -----------------------------------------------------------------------------
Matrix symmetricDecorrelation(const Matrix& W_in) {
    // M = W_in * W_in^T (symmetric)
    Matrix Wt = transpose(W_in);
    Matrix M  = matMul(W_in, Wt);

    // EVD: M = V * D * V^T
    Matrix V(0,0), D(0,0);
    jacobiEVD(M, V, D);

    // M^{-1/2} = V * D^{-1/2} * V^T
    Matrix diagVals = diagVector(D);
    Matrix sqrtVals = sqrtVector(diagVals);
    Matrix invSqrtVals = invVector(sqrtVals);
    Matrix D_inv_sqrt = diagMatrix(invSqrtVals);

    Matrix Vt = transpose(V);
    Matrix temp = matMul(V, D_inv_sqrt);
    Matrix M_inv_sqrt = matMul(temp, Vt);

    // W_out = W_in * M^{-1/2}
    Matrix W_out = matMul(W_in, M_inv_sqrt);
    return W_out;
}

// -----------------------------------------------------------------------------
// FastICA (tanh non-linearity)
//   data: (n_samples x n_features)
//   Returns: (num_components x n_samples) => the independent components
// -----------------------------------------------------------------------------
Matrix fastICA(const Matrix& data, int num_components, int max_iter = 1000, float tol = 1e-5) {
    int n_samples  = data.rows;
    int n_features = data.cols;

    // 1. Center and whiten
    Matrix centered_data = centerData(data);              // (n_samples x n_features)
    Matrix whitened_data = whitenData(centered_data);     // (n_features x n_samples)

    // 2. Initialize random W: shape (num_components x n_features)
    Matrix W = randomMatrix(num_components, n_features);

    // 3. Iteration
    for (int iter = 0; iter < max_iter; iter++) {
        // Save old W
        Matrix W_last = W;

        // WX = W * whitened_data ( shape: (num_components x n_samples) )
        Matrix WX = matMul(W, whitened_data);

        // Apply g(x) = tanh(x) element-wise
        Matrix gWX(WX.rows, WX.cols);
        Matrix gWXprime(WX.rows, WX.cols);
        for (int r = 0; r < WX.rows; r++) {
            for (int c = 0; c < WX.cols; c++) {
                float val = at(WX, r, c);
                float t   = std::tanh(val);
                at(gWX, r, c)      = t;
                at(gWXprime, r, c) = 1.0f - t*t; // derivative of tanh
            }
        }

        // Compute W_new:
        //   W_new = (gWX * whitened_data^T)/n_samples - diag(mean(g'(WX), axis=1)) * W
        // First part: gWX * whitened_data^T
        Matrix whitened_data_T = transpose(whitened_data); // (n_samples x n_features)
        Matrix gWX_whitened = matMul(gWX, whitened_data_T); // (num_components x n_features)
        Matrix firstPart = matDiv(gWX_whitened, (float)n_samples);

        // We also need mean of g'(WX) row-wise => shape (num_components)
        std::vector<float> mean_gWXprime(gWX.rows, 0.0f);
        for (int r = 0; r < gWX.rows; r++) {
            float sum = 0.0f;
            for (int c = 0; c < gWX.cols; c++) {
                sum += at(gWXprime, r, c);
            }
            mean_gWXprime[r] = sum / gWX.cols;
        }

        // Subtract diag(...) * W
        Matrix secondPart = W; // we'll scale row i by mean_gWXprime[i]
        for (int r = 0; r < secondPart.rows; r++) {
            for (int c = 0; c < secondPart.cols; c++) {
                at(secondPart, r, c) *= mean_gWXprime[r];
            }
        }

        Matrix W_new = matSub(firstPart, secondPart);

        // Symmetric decorrelation
        W = symmetricDecorrelation(W_new);

        // Check for convergence
        // We look at ||W - W_last||
        Matrix diff = matSub(W, W_last);
        float dist = frobeniusNorm(diff);
        if (dist < tol) {
            break;
        }
    }

    // The independent components are in W * whitened_data, shape: (num_components x n_samples)
    Matrix S = matMul(W, whitened_data);
    return S; // shape => (num_components x n_samples)
}

// -----------------------------------------------------------------------------
// Example "analogWrite" simulators
// -----------------------------------------------------------------------------
void analogWrite(int& pin, int value) {
    pin = value;
    std::cout << "Pin " << &pin << " set to " << value << std::endl;
}

// -----------------------------------------------------------------------------
// Example ICAProcessingTask
// -----------------------------------------------------------------------------
void ICAProcessingTask() {
    // Simulated input (replace with real data streams)
    const int num_samples = 100; // Example
    const int num_channels = 8;  // Example
    static float eeg_data_buffer[100][8]; 
    // Fill with dummy data for demonstration
    for (int i = 0; i < num_samples; i++) {
        for (int j = 0; j < num_channels; j++) {
            eeg_data_buffer[i][j] = std::sin(0.01f * i * (j+1)); // arbitrary wave
        }
    }

    // Example "pins"
    int GAIN_PIN_1 = 0;
    int GAIN_PIN_2 = 0;

    while (true) {
        // Simulate waiting for new data
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        // Convert buffer into our Matrix: shape (num_samples x num_channels)
        Matrix eeg_data(num_samples, num_channels);
        for (int i = 0; i < num_samples; i++) {
            for (int j = 0; j < num_channels; j++) {
                at(eeg_data, i, j) = eeg_data_buffer[i][j];
            }
        }

        // Perform FastICA => 2 components
        Matrix ica_components = fastICA(eeg_data, 2);

        // ica_components shape = (2 x num_samples)
        // We'll interpret col(0) as the first sample, so let's turn them into 2 vectors:
        // but in the original code, they took the first 2 columns from a (num_samples x 2) matrix.
        // Our S is (2 x num_samples). We want each row as a component across samples.
        // We'll take row 0 => component 1, row 1 => component 2.
        // Then compute min, max, mean, etc.
        // For consistency with original code's approach:
        // "component_1 = ica_components.col(0)" => old code used column 0 as the first component
        // Now our first component is row(0).
        // We'll define a helper to get row stats.

        auto getRow = [&](const Matrix& M, int rowIndex) {
            std::vector<float> rowVals(M.cols);
            for (int c = 0; c < M.cols; c++) {
                rowVals[c] = at(M, rowIndex, c);
            }
            return rowVals;
        };

        auto minVal = [](const std::vector<float>& v){
            float m = v[0];
            for (auto x : v) if (x < m) m = x;
            return m;
        };
        auto maxVal = [](const std::vector<float>& v){
            float m = v[0];
            for (auto x : v) if (x > m) m = x;
            return m;
        };
        auto meanVal = [](const std::vector<float>& v){
            float s = 0.0f;
            for (auto x : v) s += x;
            return s / v.size();
        };

        std::vector<float> comp1 = getRow(ica_components, 0);
        std::vector<float> comp2 = getRow(ica_components, 1);

        float c1_min = minVal(comp1);
        float c1_max = maxVal(comp1);
        float c1_mean = meanVal(comp1);

        float c2_min = minVal(comp2);
        float c2_max = maxVal(comp2);
        float c2_mean = meanVal(comp2);

        // Normalize [mean] to [0..255] using min,max
        float gain_1 = 0.f;
        if (c1_max > c1_min) {
            gain_1 = (c1_mean - c1_min) / (c1_max - c1_min) * 255.f;
        }
        float gain_2 = 0.f;
        if (c2_max > c2_min) {
            gain_2 = (c2_mean - c2_min) / (c2_max - c2_min) * 255.f;
        }

        // Output to simulated pins
        analogWrite(GAIN_PIN_1, (int)gain_1);
        analogWrite(GAIN_PIN_2, (int)gain_2);

        // Short delay
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

int main() {
    ICAProcessingTask();
    return 0;
}
