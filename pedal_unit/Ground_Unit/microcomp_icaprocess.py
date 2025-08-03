import numpy as np
import mne
from sklearn.decomposition import FastICA
import Jetson.GPIO as GPIO
import time

EEG_PIN_1 = 12  # Example pin number for EEG channel 1
EEG_PIN_2 = 16  # Example pin number for EEG channel 2
EEG_PIN_3 = 18  # Example pin number for EEG channel 3
EEG_PIN_4 = 22  # Example pin number for EEG channel 4

# Pin setup
GPIO.setmode(GPIO.BOARD)  # Use physical pin numbering
GPIO.setup(EEG_PIN_1, GPIO.IN)
GPIO.setup(EEG_PIN_2, GPIO.IN)
GPIO.setup(EEG_PIN_3, GPIO.IN)
GPIO.setup(EEG_PIN_4, GPIO.IN)

# Function to read EEG channel values from GPIO pins
def read_eeg_pins():
    # Read digital values from GPIO pins (assuming a range from 0-2048)
    eeg_1 = GPIO.input(EEG_PIN_1)
    eeg_2 = GPIO.input(EEG_PIN_2)
    eeg_3 = GPIO.input(EEG_PIN_3)
    eeg_4 = GPIO.input(EEG_PIN_4)
    
    # Return as an array of readings (normalize to 0-1)
    eeg_data = np.array([eeg_1, eeg_2, eeg_3, eeg_4], dtype=float)
    return eeg_data

# Perform ICA on the EEG data
def run_ica_on_eeg(eeg_data):
    # Initialize the ICA object from sklearn
    ica = FastICA(n_components=2)  # We want two components for the gain control
    ica_components = ica.fit_transform(eeg_data)
    
    # Return the two independent components
    return ica_components[:, 0], ica_components[:, 1]

# Normalizing the components to control the gain values
def normalize_component(comp, min_value=0.0, max_value=1.0):
    comp_min, comp_max = np.min(comp), np.max(comp)
    normalized = (comp - comp_min) / (comp_max - comp_min) * (max_value - min_value) + min_value
    return normalized

# Loop to constantly read EEG data and apply ICA
try:
    while True:
        # Read EEG data from GPIO pins
        eeg_raw_data = np.array([read_eeg_pins() for _ in range(1000)])  # Collect 1000 samples
        
        # Apply ICA to the EEG data
        component_1, component_2 = run_ica_on_eeg(eeg_raw_data)
        
        # Normalize the ICA components to the gain control range
        gain_1 = normalize_component(component_1, min_value=0.0, max_value=1.0)
        gain_2 = normalize_component(component_2, min_value=0.0, max_value=1.0)
        
        # Print the normalized gain values (replace with actual guitar pedal interface code)
        print(f"Gain for Pedal 1: {gain_1}")
        print(f"Gain for Pedal 2: {gain_2}")
        
        # Sleep for a short while before reading new data
        time.sleep(0.1)

finally:
    GPIO.cleanup()  # Reset GPIO settings on exit
