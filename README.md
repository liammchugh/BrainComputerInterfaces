# Brain-Computer Interfaces (BCI)

This repository contains research and development work on BCI devices for experimentation, entertainment and commercial applications.

### BCI Guitar Pedal

A novel interface that translates brain and muscle signals into guitar effect controls.

#### Overview
This project uses EEG (electroencephalography) and EMG (electromyography) signals to control audio effect parameters in real-time, creating a hands-free experience for musicians.

#### Instrumentation & Communication
- EMG sensors from commercial suppliers
- FastICA optimized for C++ and conventional microprocessors.
- Sub-60μs latency requirement. Implementation in C++ for optimized performance

#### Ground Module / Audio Effects
- Target platforms: 
    - Raspi 4/5, NVIDIA Jetson
    - NXP i.MX RT1170 microcontroller
- Initial Effects: Overdrive and distortion circuits, commercial quality, bench-tested.
- In development: ML feature processing models w/engagement-based reinforcement learning.
- Conventional power circuit design. Signal mapping from BCI data to effect parameters

## Getting Started

Instructions for setting up development environment and hardware configurations will be added soon.

## License

[License information to be added]

## Contributors

[Contributor information to be added]
