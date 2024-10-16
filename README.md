# ECBM4090repo_BrainComputerInterfaces
BCI Devices for Messing around & Potentially Commercial R&D

## Current Devices

### BCI Guitar Pedal
Take principle components of EEG signal and use them to control an audio effect pedal.
#### Hardware
EEG from Columbia BCI Lab. Eventually use custom EEG/EMG headband
C++ to run on commercial microprocessing units (Jetson, NXP i.MX RT1170)
#### Principle Component Analysis
Must be pseudo-realtime (sub-100us delay)... 
FastICA set up to run on C++ with conventional MPs
Ideally will implement a model-based reinforcement-learning PC system for fine-tuning.
#### Effects Modulator
Currently planning simple overdrive & distortion pedal using conventional power circuits.

