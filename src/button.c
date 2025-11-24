#include "button.h"
#include "app_config.h"
#include "nrfx_gpiote.h"

void button_init(button_handler_t handler)
{
    nrfx_err_t err_code;
    
    if (!nrfx_gpiote_is_init()) {
        err_code = nrfx_gpiote_init();
        if (err_code != NRFX_SUCCESS && err_code != NRFX_ERROR_INVALID_STATE) {
            return;
        }
    }
    
    nrf_gpio_cfg_input(BUTTON_PIN, NRF_GPIO_PIN_PULLUP);
    
    nrfx_gpiote_in_config_t button_config = NRFX_GPIOTE_CONFIG_IN_SENSE_HITOLO(true);
    button_config.pull = NRF_GPIO_PIN_PULLUP;
    
    err_code = nrfx_gpiote_in_init(BUTTON_PIN, &button_config, handler);
    
    if (err_code == NRFX_SUCCESS) {
        nrfx_gpiote_in_event_enable(BUTTON_PIN, true);
    }
}