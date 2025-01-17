import numpy as np
import time
import zlib
import Jetson.GPIO as GPIO
import bluetooth

# If using TensorFlow/Keras for the LSTM model:
from tensorflow.keras.models import load_model

# --------------------------------------------------------------------
# Bluetooth Configuration
# --------------------------------------------------------------------
# Adjust these constants as needed
BT_PORT = 1               # RFCOMM port
BACKLOG = 1               # Number of unaccepted connections before refusing new ones
BUFFER_SIZE = 4096        # How many bytes to read at once from the socket

# --------------------------------------------------------------------
# GPIO Setup (Optional)
# --------------------------------------------------------------------
# If you still want to drive any GPIO pins (e.g., toggling pedal parameters),
# configure them here. Otherwise, you can remove or comment out this section.
PEDAL_PIN = 12  # Example pin to indicate classification to a hardware line

GPIO.setmode(GPIO.BOARD)  
GPIO.setup(PEDAL_PIN, GPIO.OUT, initial=GPIO.LOW)

# --------------------------------------------------------------------
# Load Pretrained LSTM Model
# --------------------------------------------------------------------
# Make sure you have your model file in the same directory or provide the full path
model = load_model('my_lstm_model.h5')

# --------------------------------------------------------------------
# Function to Receive and Decompress Data via Bluetooth
# --------------------------------------------------------------------
def receive_bluetooth_data():
    """
    Listens for a single Bluetooth connection, receives compressed data,
    decompresses it, and returns as a NumPy array.
    """
    server_socket = bluetooth.BluetoothSocket(bluetooth.RFCOMM)
    server_socket.bind(("", BT_PORT))
    server_socket.listen(BACKLOG)
    
    print(f"[*] Waiting for Bluetooth connection on RFCOMM port {BT_PORT}...")
    client_socket, address = server_socket.accept()
    print(f"[+] Accepted connection from {address}")

    # Collect data in a loop or a single read if you know the exact size
    chunks = []
    while True:
        data = client_socket.recv(BUFFER_SIZE)
        if not data:
            break
        chunks.append(data)
    
    # Close sockets once done
    client_socket.close()
    server_socket.close()
    
    # Combine all chunks
    compressed_data = b''.join(chunks)
    
    # Decompress the received data
    decompressed_data = zlib.decompress(compressed_data)
    
    # Convert to NumPy array
    # Adjust dtype and shape as needed to match your LSTM input
    # For example, if your EMG/EEG data is float32
    arr = np.frombuffer(decompressed_data, dtype=np.float32)

    return arr

# --------------------------------------------------------------------
# Main Loop
# --------------------------------------------------------------------
try:
    while True:
        # 1. Receive data from Bluetooth
        input_data = receive_bluetooth_data()
        
        # 2. Reshape data for LSTM input
        #    Adjust (1, timesteps, features) to match your model's expected shape.
        #    This is just an example shape of (1, 100, 4).
        #    e.g., 100 timesteps, 4 features
        #    If your data length doesn't match exactly, adjust accordingly.
        try:
            input_data = input_data.reshape((1, 100, 4))  
        except ValueError:
            print("[!] Received data cannot be reshaped to (1, 100, 4). Check your data shape.")
            continue
        
        # 3. Run inference with the pretrained LSTM model
        prediction = model.predict(input_data)
        
        # 4. Get classification (for multi-class)
        predicted_class = np.argmax(prediction, axis=1)[0]
        
        # 5. Use classification output
        # For demonstration, we simply print the classification
        print(f"Predicted Class: {predicted_class}")
        
        # Example: toggle a GPIO pin based on classification
        if predicted_class == 0:
            GPIO.output(PEDAL_PIN, GPIO.LOW)
        else:
            GPIO.output(PEDAL_PIN, GPIO.HIGH)
        
        # Delay before next read
        time.sleep(0.2)

except KeyboardInterrupt:
    print("[*] Exiting...")

finally:
    GPIO.cleanup()
    print("[*] GPIO cleaned up. Application closed.")
