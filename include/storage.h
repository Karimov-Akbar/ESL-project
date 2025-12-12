#ifndef STORAGE_H
#define STORAGE_H

#include "app_config.h"
#include <stdbool.h>

void storage_init(void);

void storage_save_current_hsv(const hsv_color_t *hsv);

bool storage_get_last_hsv(hsv_color_t *hsv);

bool storage_add_color(const char *name, const hsv_color_t *hsv);
bool storage_del_color(const char *name);
bool storage_get_color(const char *name, hsv_color_t *hsv);

void storage_list_colors(void (*print_func)(const char *fmt, ...));

#endif