import numpy as np
import mne
from sklearn.decomposition import FastICA

# Load sample EEG data from MNE
sample_data = mne.io.read_raw_fif(mne.datasets.sample.data_path() + '/MEG/sample/sample_audvis_raw.fif', preload=True)
raw_data = sample_data.get_data()
eeg_data = raw_data[:4, :].T

# Perform ICA on the EEG data
def run_ica_on_eeg(eeg_data):
    # Initialize the ICA object from sklearn
    ica = FastICA(n_components=2)  # We want two components for the gain control
    ica_components = ica.fit_transform(eeg_data)

    # Return the two independent components
    return ica_components[:, 0], ica_components[:, 1]

# Apply ICA to the EEG data
component_1, component_2 = run_ica_on_eeg(eeg_data)

# Normalizing the components to control the gain values
def normalize_component(comp, min_value=0.0, max_value=1.0):
    comp_min, comp_max = np.min(comp), np.max(comp)
    normalized = (comp - comp_min) / (comp_max - comp_min) * (max_value - min_value) + min_value
    return normalized

# Normalize the ICA components to the gain control range
gain_1 = normalize_component(component_1, min_value=0.0, max_value=1.0)
gain_2 = normalize_component(component_2, min_value=0.0, max_value=1.0)

# Displaying the gain values to be applied to the guitar pedal effects
print(f"Gain for Pedal 1: {gain_1}")
print(f"Gain for Pedal 2: {gain_2}")

# Output:
