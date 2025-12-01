#include "storage.h"
#include "nrfx_nvmc.h"

static uint32_t pack_hsv(const hsv_color_t *hsv) {
    return ((uint32_t)hsv->h << 16) | ((uint32_t)hsv->s << 8) | (uint32_t)hsv->v;
}

static void unpack_hsv(uint32_t data, hsv_color_t *hsv) {
    hsv->h = (uint16_t)((data >> 16) & 0xFFFF);
    hsv->s = (uint8_t)((data >> 8) & 0xFF);
    hsv->v = (uint8_t)(data & 0xFF);
}

bool storage_read_hsv(hsv_color_t *hsv) {
    uint32_t *p_addr = (uint32_t *)FLASH_STORAGE_ADDR;
    uint32_t stored_data = *p_addr;

    if (stored_data == 0xFFFFFFFF) {
        return false;
    }

    unpack_hsv(stored_data, hsv);
    return true;
}

void storage_save_hsv(const hsv_color_t *hsv) {
    uint32_t new_data = pack_hsv(hsv);
    uint32_t *p_addr = (uint32_t *)FLASH_STORAGE_ADDR;
    uint32_t current_data = *p_addr;

    if (current_data == new_data) {
        return;
    }

    if (current_data != 0xFFFFFFFF) {
        nrfx_nvmc_page_erase(FLASH_STORAGE_ADDR);
    }

    nrfx_nvmc_word_write(FLASH_STORAGE_ADDR, new_data);
    
    while (!nrfx_nvmc_write_done_check()) {}
}