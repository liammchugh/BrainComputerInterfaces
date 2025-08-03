#pragma once
#include <zephyr/kernel.h>
#include <zephyr/drivers/adc.h>

constexpr uint8_t  EMG_CH   = 5;          // 3-5 sensors; pick 5 max
constexpr uint16_t EMG_SPS  = 1000;       // target sample rate

struct emg_frame_t {
    uint32_t tick_us;                     // µs timestamp
    int16_t  sample[EMG_CH];
};

// double-buffer for DMA ping-pong
inline k_fifo adc_fifo;
void emg_adc_init();
