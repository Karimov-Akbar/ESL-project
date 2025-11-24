#include "pwm_leds.h"
#include "nrfx_pwm.h"

static nrfx_pwm_t m_pwm_rgb = NRFX_PWM_INSTANCE(0);
static nrfx_pwm_t m_pwm_indicator = NRFX_PWM_INSTANCE(1);

static nrf_pwm_values_individual_t pwm_rgb_values;
static nrf_pwm_sequence_t const pwm_rgb_seq = {
    .values.p_individual = &pwm_rgb_values,
    .length = NRF_PWM_VALUES_LENGTH(pwm_rgb_values),
    .repeats = 0,
    .end_delay = 0
};

static nrf_pwm_values_individual_t pwm_indicator_values;
static nrf_pwm_sequence_t const pwm_indicator_seq = {
    .values.p_individual = &pwm_indicator_values,
    .length = NRF_PWM_VALUES_LENGTH(pwm_indicator_values),
    .repeats = 0,
    .end_delay = 0
};

void pwm_rgb_init(void)
{
    nrfx_pwm_config_t pwm_config = NRFX_PWM_DEFAULT_CONFIG;
    pwm_config.output_pins[0] = LED_2_RED;
    pwm_config.output_pins[1] = LED_2_BLUE;
    pwm_config.output_pins[2] = LED_2_GREEN;
    pwm_config.output_pins[3] = NRFX_PWM_PIN_NOT_USED;
    pwm_config.base_clock = NRF_PWM_CLK_1MHz;
    pwm_config.count_mode = NRF_PWM_MODE_UP;
    pwm_config.top_value = PWM_TOP_VALUE;
    pwm_config.load_mode = NRF_PWM_LOAD_INDIVIDUAL;
    pwm_config.step_mode = NRF_PWM_STEP_AUTO;
    
    nrfx_pwm_init(&m_pwm_rgb, &pwm_config, NULL);
    
    pwm_rgb_values.channel_0 = 0;
    pwm_rgb_values.channel_1 = 0;
    pwm_rgb_values.channel_2 = 0;
    pwm_rgb_values.channel_3 = 0;
    
    nrfx_pwm_simple_playback(&m_pwm_rgb, &pwm_rgb_seq, 1, NRFX_PWM_FLAG_LOOP);
}

void pwm_indicator_init(void)
{
    nrfx_pwm_config_t pwm_config = NRFX_PWM_DEFAULT_CONFIG;
    pwm_config.output_pins[0] = LED_1_GREEN;
    pwm_config.output_pins[1] = NRFX_PWM_PIN_NOT_USED;
    pwm_config.output_pins[2] = NRFX_PWM_PIN_NOT_USED;
    pwm_config.output_pins[3] = NRFX_PWM_PIN_NOT_USED;
    pwm_config.base_clock = NRF_PWM_CLK_1MHz;
    pwm_config.count_mode = NRF_PWM_MODE_UP;
    pwm_config.top_value = PWM_TOP_VALUE;
    pwm_config.load_mode = NRF_PWM_LOAD_INDIVIDUAL;
    pwm_config.step_mode = NRF_PWM_STEP_AUTO;
    
    nrfx_pwm_init(&m_pwm_indicator, &pwm_config, NULL);
    
    pwm_indicator_values.channel_0 = 0;
    nrfx_pwm_simple_playback(&m_pwm_indicator, &pwm_indicator_seq, 1, NRFX_PWM_FLAG_LOOP);
}

void pwm_set_rgb_values(uint16_t r, uint16_t g, uint16_t b)
{
    pwm_rgb_values.channel_0 = r;
    pwm_rgb_values.channel_1 = b;
    pwm_rgb_values.channel_2 = g;
    pwm_rgb_values.channel_3 = 0;
}

void pwm_set_indicator_value(uint16_t value)
{
    pwm_indicator_values.channel_0 = value;
}