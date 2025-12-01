#ifndef STORAGE_H
#define STORAGE_H

#include <stdbool.h>
#include <stdint.h>
#include "app_config.h"

#define FLASH_STORAGE_ADDR 0x00070000

bool storage_read_hsv(hsv_color_t *hsv);

void storage_save_hsv(const hsv_color_t *hsv);

#endif