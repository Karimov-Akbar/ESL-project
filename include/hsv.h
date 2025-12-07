#ifndef HSV_H
#define HSV_H

#include <stdint.h>

void hsv_to_rgb_simple(uint16_t h, uint8_t s, uint8_t v, uint16_t *r, uint16_t *g, uint16_t *b);

void rgb_to_hsv_simple(uint16_t r, uint16_t g, uint16_t b, uint16_t *h, uint8_t *s, uint8_t *v);

#endif