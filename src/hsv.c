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

void rgb_to_hsv_simple(uint16_t r, uint16_t g, uint16_t b, uint16_t *h, uint8_t *s, uint8_t *v)
{
    if (r > PWM_TOP_VALUE) r = PWM_TOP_VALUE;
    if (g > PWM_TOP_VALUE) g = PWM_TOP_VALUE;
    if (b > PWM_TOP_VALUE) b = PWM_TOP_VALUE;

    uint32_t max_val = MAX(r, MAX(g, b));
    uint32_t min_val = MIN(r, MIN(g, b));
    uint32_t delta = max_val - min_val;

    *v = max_val / 10; 

    if (max_val == 0) {
        *s = 0;
    } else {
        *s = (delta * 100) / max_val;
    }

    if (delta == 0) {
        *h = 0;
    } else {
        int32_t hue_temp;
        if (max_val == r) {
            hue_temp = ((int32_t)(g - b) * 60) / (int32_t)delta;
            if (g < b) hue_temp += 360;
        } else if (max_val == g) {
            hue_temp = ((int32_t)(b - r) * 60) / (int32_t)delta + 120;
        } else {
            hue_temp = ((int32_t)(r - g) * 60) / (int32_t)delta + 240;
        }
        
        if (hue_temp < 0) hue_temp += 360;
        if (hue_temp >= 360) hue_temp -= 360;
        
        *h = (uint16_t)hue_temp;
    }
}