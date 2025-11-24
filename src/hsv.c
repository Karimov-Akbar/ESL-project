#include "hsv.h"
#include "app_config.h"

void hsv_to_rgb_simple(uint16_t h, uint8_t s, uint8_t v, uint16_t *r, uint16_t *g, uint16_t *b)
{
    h = h % 360;
    
    if (s == 0) {
        *r = *g = *b = (v * PWM_TOP_VALUE) / 100;
        return;
    }
    
    uint8_t sector = h / 60;
    uint16_t remainder = (h % 60) * 6;
    
    uint32_t p = ((uint32_t)v * (100 - s)) / 100;
    uint32_t q = ((uint32_t)v * (100 - ((s * remainder) / 360))) / 100;
    uint32_t t = ((uint32_t)v * (100 - ((s * (360 - remainder)) / 360))) / 100;
    
    uint16_t v_pwm = (v * PWM_TOP_VALUE) / 100;
    uint16_t p_pwm = (p * PWM_TOP_VALUE) / 100;
    uint16_t q_pwm = (q * PWM_TOP_VALUE) / 100;
    uint16_t t_pwm = (t * PWM_TOP_VALUE) / 100;
    
    switch(sector) {
        case 0:
            *r = v_pwm; *g = t_pwm; *b = p_pwm;
            break;
        case 1:
            *r = q_pwm; *g = v_pwm; *b = p_pwm;
            break;
        case 2:
            *r = p_pwm; *g = v_pwm; *b = t_pwm;
            break;
        case 3:
            *r = p_pwm; *g = q_pwm; *b = v_pwm;
            break;
        case 4:
            *r = t_pwm; *g = p_pwm; *b = v_pwm;
            break;
        default:
            *r = v_pwm; *g = p_pwm; *b = q_pwm;
            break;
    }
}