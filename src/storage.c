#include "storage.h"
#include "nrfx_nvmc.h"
#include <string.h>
#include <stdio.h>

#define FLASH_STORAGE_ADDR 0x00060000 
#define STORAGE_MAGIC      0xCAFEBABE

typedef struct {
    char name[COLOR_NAME_MAX_LEN];
    hsv_color_t color;
    uint8_t valid;
    uint8_t padding[3];
} color_entry_t; 

typedef struct {
    uint32_t magic;
    hsv_color_t last_state;
    uint8_t padding[2]; 
    color_entry_t saved_colors[MAX_SAVED_COLORS];
} flash_data_t;

static flash_data_t m_ram_data __attribute__((aligned(4)));

static void flash_sync(void) {
    nrfx_nvmc_page_erase(FLASH_STORAGE_ADDR);
    
    uint32_t *p_src = (uint32_t *)&m_ram_data;
    uint32_t words_to_write = sizeof(flash_data_t) / 4;
    if (sizeof(flash_data_t) % 4 != 0) words_to_write++;

    nrfx_nvmc_words_write(FLASH_STORAGE_ADDR, p_src, words_to_write);
    
    while (!nrfx_nvmc_write_done_check()) {}
}

void storage_init(void) {
    flash_data_t *p_flash = (flash_data_t *)FLASH_STORAGE_ADDR;
    
    if (p_flash->magic == STORAGE_MAGIC) {
        memcpy(&m_ram_data, p_flash, sizeof(flash_data_t));
    } else {
        memset(&m_ram_data, 0, sizeof(flash_data_t));
        m_ram_data.magic = STORAGE_MAGIC;
    }
}

void storage_save_current_hsv(const hsv_color_t *hsv) {
    if (memcmp(&m_ram_data.last_state, hsv, sizeof(hsv_color_t)) != 0) {
        m_ram_data.last_state = *hsv;
        flash_sync();
    }
}

bool storage_get_last_hsv(hsv_color_t *hsv) {
    flash_data_t *p_flash = (flash_data_t *)FLASH_STORAGE_ADDR;
    if (p_flash->magic != STORAGE_MAGIC) {
        return false;
    }
    *hsv = m_ram_data.last_state;
    return true;
}


bool storage_add_color(const char *name, const hsv_color_t *hsv) {
    int empty_idx = -1;
    
    for (int i = 0; i < MAX_SAVED_COLORS; i++) {
        if (m_ram_data.saved_colors[i].valid) {
            if (strcmp(m_ram_data.saved_colors[i].name, name) == 0) {
                m_ram_data.saved_colors[i].color = *hsv;
                flash_sync();
                return true;
            }
        } else if (empty_idx == -1) {
            empty_idx = i;
        }
    }

    if (empty_idx != -1) {
        strncpy(m_ram_data.saved_colors[empty_idx].name, name, COLOR_NAME_MAX_LEN - 1);
        m_ram_data.saved_colors[empty_idx].name[COLOR_NAME_MAX_LEN - 1] = '\0'; 
        m_ram_data.saved_colors[empty_idx].color = *hsv;
        m_ram_data.saved_colors[empty_idx].valid = 1;
        flash_sync();
        return true;
    }

    return false;
}

bool storage_del_color(const char *name) {
    for (int i = 0; i < MAX_SAVED_COLORS; i++) {
        if (m_ram_data.saved_colors[i].valid && 
            strcmp(m_ram_data.saved_colors[i].name, name) == 0) {
            
            m_ram_data.saved_colors[i].valid = 0;
            flash_sync();
            return true;
        }
    }
    return false;
}

bool storage_get_color(const char *name, hsv_color_t *hsv) {
    for (int i = 0; i < MAX_SAVED_COLORS; i++) {
        if (m_ram_data.saved_colors[i].valid && 
            strcmp(m_ram_data.saved_colors[i].name, name) == 0) {
            
            *hsv = m_ram_data.saved_colors[i].color;
            return true;
        }
    }
    return false;
}

void storage_list_colors(void (*print_func)(const char *fmt, ...)) {
    bool found = false;
    print_func("\r\nSaved Colors:\r\n");
    
    for (int i = 0; i < MAX_SAVED_COLORS; i++) {
        if (m_ram_data.saved_colors[i].valid) {
            hsv_color_t *c = &m_ram_data.saved_colors[i].color;
            print_func("  [%d] %s: H=%d S=%d V=%d\r\n", 
                    i, m_ram_data.saved_colors[i].name, c->h, c->s, c->v);
            found = true;
        }
    }
    
    if (!found) {
        print_func("  (Empty)\r\n");
    }
}