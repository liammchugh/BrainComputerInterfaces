#include "emg_adc.hpp"
#include <zephyr/device.h>
#include <zephyr/drivers/clock_control.h>
#include <zephyr/drivers/timer/nrf_rtc_timer.h>   // STM32 uses generic timer; choose TIM2
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(emg_adc, LOG_LEVEL_INF);

static const struct device *adc = DEVICE_DT_GET_ONE(st_stm32_adc);
static int16_t dma_buf[2][EMG_CH];               // ping-pong
static uint8_t buf_idx;

static void dma_callback(const struct device*, void*, uint32_t, int)
{
    auto *frame = (emg_frame_t*)k_malloc(sizeof(emg_frame_t));
    frame->tick_us = k_cycle_get_32() / (CONFIG_SYS_CLOCK_HW_CYCLES_PER_SEC / 1000000);
    memcpy(frame->sample, dma_buf[buf_idx], sizeof(frame->sample));
    k_fifo_put(&adc_fifo, frame);
    buf_idx ^= 1;
}

void emg_adc_init()
{
    /* Timer trigger @1 kHz */
    const struct device *tim = DEVICE_DT_GET(DT_ALIAS(timer2));
    timer_configure(tim, 1000);  // helper util you’d write for TIM & Zephyr counter API

    /* ADC sequence */
    struct adc_channel_cfg ch = {
        .gain             = ADC_GAIN_1,
        .reference        = ADC_REF_INTERNAL,
        .acquisition_time = ADC_ACQ_TIME_DEFAULT,
    };
    for (uint8_t i = 0; i < EMG_CH; ++i) {
        ch.channel_id = i;
        adc_channel_setup(adc, &ch);
    }

    struct adc_sequence seq = {
        .channels    = BIT_MASK(EMG_CH),
        .buffer      = dma_buf[0],
        .buffer_size = sizeof(dma_buf[0]),
        .resolution  = 12,
        .oversampling = 4,                 // optional
        .calibrate = true,
    };

    adc_dma_start(adc, &seq, dma_callback);       // Zephyr v4.2 helper
    LOG_INF("ADC started @1 kHz x %d ch", EMG_CH);
}
