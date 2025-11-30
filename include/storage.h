#ifndef STORAGE_H
#define STORAGE_H

#include "app_config.h"

#define FLASH_STORAGE_ADDR 0x000F0000

bool storage_read_hsv(hsv_color_t *hsv);

void storage_save_hsv(const hsv_color_t *hsv);

#endif