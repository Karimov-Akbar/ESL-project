#include "boards.h"
#include "nrf_delay.h"
#include <stdbool.h>

#define LED_PINS_COUNT 4

/**
 * @brief
 * @param led_index
 * @param count
 * @param blink_delay_ms
 */
void blink_led(uint32_t led_index, int count, uint32_t blink_delay_ms)
{
    for (int i = 0; i < count; i++)
    {
        bsp_board_led_on(led_index);
        nrf_delay_ms(blink_delay_ms);

        bsp_board_led_off(led_index);
        nrf_delay_ms(blink_delay_ms);
    }
}

/**
 * @brief
 */
int main(void)
{
    uint8_t id_sequence[] = {7, 2, 0, 5};
    
    uint32_t inter_sequence_delay = 1000;
    uint32_t blink_time_ms = 150;

    bsp_board_init(BSP_INIT_LEDS);

    while (true)
    {

        for (size_t i = 0; i < LED_PINS_COUNT; i++)
        {
            uint32_t count = id_sequence[i]; 
            
            if (count > 0)
            {
                blink_led(i, count, blink_time_ms);
            }
            
            nrf_delay_ms(inter_sequence_delay);
        }
    }
}