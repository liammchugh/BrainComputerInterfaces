#include "emg_adc.hpp"
extern void ble_service_init();

int main()
{
    k_fifo_init(&adc_fifo);
    emg_adc_init();
    ble_service_init();
    return 0;
}
