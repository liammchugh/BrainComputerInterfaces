# BrainComputerInterfaces
BCI Devices for Messing around & Potentially Commercial R&D

## BCI Guitar Pedal
Takes principle components of EEG/EMG signal and use them to control an audio effect pedal.
#### Hardware
EEG from Columbia BCI Lab. Developing custom EMG headband for muscle-based control.
C++ to run on commercial microprocessing units (Jetson, NXP i.MX RT1170)
#### Principle Component Analysis
Must be pseudo-realtime (sub-100us delay)... 
FastICA set up to run on C++ with conventional MPs
Ideally will implement a model-based reinforcement-learning PC system for fine-tuning.
#### Effects Modulator
Currently planning simple overdrive & distortion pedal using conventional power circuits.

