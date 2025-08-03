#include <driver/i2s.h>
#include <Arduino.h>

#define I2S_NUM         I2S_NUM_0
#define SAMPLE_RATE     1000          // 1 kHz sample rate
#define BUFFER_SIZE     128           // Ring buffer size for samples

// I2S configuration for ADC mode
i2s_config_t i2s_config = {
  .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_ADC_BUILT_IN),
  .sample_rate = SAMPLE_RATE,
  .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
  .channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT,
  .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_I2S_MSB),
  .intr_alloc_flags = 0,
  .dma_buf_count = 4,
  .dma_buf_len = 64,
  .use_apll = false
};

// ADC configuration: GPIO35 is ADC1 channel 7
#define ADC_CHANNEL ADC1_CHANNEL_7

// Timer variables
hw_timer_t *timer = NULL;
volatile bool sampleFlag = false;

// Ring buffer for samples
volatile uint16_t sampleBuffer[BUFFER_SIZE];
volatile int bufferHead = 0;
volatile int bufferTail = 0;

void IRAM_ATTR onTimer() {
  sampleFlag = true;
}

void setup() {
  Serial.begin(115200);

  // Setup I2S for ADC mode
  i2s_driver_install(I2S_NUM, &i2s_config, 0, NULL);
  i2s_set_adc_mode(ADC_UNIT_1, ADC_CHANNEL);
  i2s_adc_enable(I2S_NUM);

  // Setup hardware timer (timer 0, 80 divider -> 1 µs tick)
  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, 1000, true);  // 1 kHz: 1000 µs
  timerAlarmEnable(timer);
}

void loop() {
  // Sampling part: triggered by timer interrupt flag
  if (sampleFlag) {
    sampleFlag = false;
    uint16_t sample;
    size_t bytesRead = 0;
    // Read one 16-bit sample; use a short timeout to avoid blocking too long
    if (i2s_read(I2S_NUM, &sample, sizeof(sample), &bytesRead, 0) == ESP_OK && bytesRead == sizeof(sample)) {
      // Add sample to ring buffer if there is space
      int nextHead = (bufferHead + 1) % BUFFER_SIZE;
      if (nextHead != bufferTail) { // buffer not full
        sampleBuffer[bufferHead] = sample;
        bufferHead = nextHead;
      }
    }
  }

  // Processing part: print samples from the buffer
  while (bufferTail != bufferHead) {
    uint16_t s = sampleBuffer[bufferTail];
    bufferTail = (bufferTail + 1) % BUFFER_SIZE;
    Serial.println(s);
  }
}
