#include "usb_cli.h"
#include "app_config.h"
#include "app_usbd.h"
#include "app_usbd_cdc_acm.h"
#include "app_usbd_serial_num.h"
#include "app_usbd_core.h"
#include "hsv.h"
#include "pwm_leds.h"
#include "nrf_delay.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <strings.h>
#include <stdbool.h>

bool app_usbd_event_queue_process(void);

extern void update_hsv_state(hsv_color_t new_hsv);

static void cdc_acm_user_ev_handler(app_usbd_class_inst_t const * p_inst,
                                    app_usbd_cdc_acm_user_event_t event);

#define CDC_ACM_COMM_INTERFACE  0
#define CDC_ACM_COMM_EPIN       NRF_DRV_USBD_EPIN2
#define CDC_ACM_DATA_INTERFACE  1
#define CDC_ACM_DATA_EPIN       NRF_DRV_USBD_EPIN1
#define CDC_ACM_DATA_EPOUT      NRF_DRV_USBD_EPOUT1

APP_USBD_CDC_ACM_GLOBAL_DEF(m_app_cdc_acm,
                            cdc_acm_user_ev_handler,
                            CDC_ACM_COMM_INTERFACE,
                            CDC_ACM_DATA_INTERFACE,
                            CDC_ACM_COMM_EPIN,
                            CDC_ACM_DATA_EPIN,
                            CDC_ACM_DATA_EPOUT,
                            APP_USBD_CDC_COMM_PROTOCOL_AT_V250);

#define RX_BUF_SIZE 256
static char m_fifo_buffer[RX_BUF_SIZE];
static volatile uint16_t m_fifo_head = 0;
static volatile uint16_t m_fifo_tail = 0;

static char m_usb_rx_char;
static char m_line_buffer[128];
static uint8_t m_line_idx = 0;
static char m_tx_buffer[512];

static void fifo_put(char c) {
    uint16_t next_head = (m_fifo_head + 1) % RX_BUF_SIZE;
    if (next_head != m_fifo_tail) {
        m_fifo_buffer[m_fifo_head] = c;
        m_fifo_head = next_head;
    }
}

static bool fifo_get(char *c) {
    if (m_fifo_head == m_fifo_tail) {
        return false;
    }
    *c = m_fifo_buffer[m_fifo_tail];
    m_fifo_tail = (m_fifo_tail + 1) % RX_BUF_SIZE;
    return true;
}

static void usb_send_blocking(const char *data, size_t len) {
    if (len == 0) return;
    if (len > sizeof(m_tx_buffer)) len = sizeof(m_tx_buffer);
    
    memcpy(m_tx_buffer, data, len);
    
    ret_code_t ret;
    int timeout = 100000;
    
    do {
        ret = app_usbd_cdc_acm_write(&m_app_cdc_acm, m_tx_buffer, len);
        if (ret == NRF_ERROR_BUSY) {
            nrf_delay_us(50); 
            timeout--;
        }
    } while (ret == NRF_ERROR_BUSY && timeout > 0);
}

static void usb_print(const char *msg) {
    usb_send_blocking(msg, strlen(msg));
}

static void process_command(char *cmd) {
    char temp_resp[128]; 
    char *token = strtok(cmd, " \r\n");

    if (token == NULL) {
        usb_print("\r\n> ");
        return;
    }

    if (strcasecmp(token, "help") == 0) {
        usb_print("\r\nSupported commands:\r\n"
                  "  RGB <r> <g> <b>    Set RGB (0-1000)\r\n"
                  "  HSV <h> <s> <v>    Set HSV (0-360, 0-100, 0-100)\r\n"
                  "  help               Show help\r\n");
    } 
    else if (strcasecmp(token, "RGB") == 0) {
        int r = -1, g = -1, b = -1;
        char *arg1 = strtok(NULL, " ");
        char *arg2 = strtok(NULL, " ");
        char *arg3 = strtok(NULL, " ");

        if (arg1) r = atoi(arg1);
        if (arg2) g = atoi(arg2);
        if (arg3) b = atoi(arg3);

        if (r >= 0 && r <= 1000 && g >= 0 && g <= 1000 && b >= 0 && b <= 1000) {
            hsv_color_t new_hsv;
            rgb_to_hsv_simple(r, g, b, &new_hsv.h, &new_hsv.s, &new_hsv.v);
            update_hsv_state(new_hsv);
            
            sprintf(temp_resp, "\r\nColor set to R=%d G=%d B=%d\r\n", r, g, b);
            usb_print(temp_resp);
        } else {
            usb_print("\r\nError: RGB values must be 0-1000\r\n");
        }
    }
    else if (strcasecmp(token, "HSV") == 0) {
        int h = -1, s = -1, v = -1;
        char *arg1 = strtok(NULL, " ");
        char *arg2 = strtok(NULL, " ");
        char *arg3 = strtok(NULL, " ");

        if (arg1) h = atoi(arg1);
        if (arg2) s = atoi(arg2);
        if (arg3) v = atoi(arg3);

        if (h >= 0 && h <= 360 && s >= 0 && s <= 100 && v >= 0 && v <= 100) {
            hsv_color_t new_hsv = { (uint16_t)h, (uint8_t)s, (uint8_t)v };
            update_hsv_state(new_hsv);

            sprintf(temp_resp, "\r\nColor set to H=%d S=%d V=%d\r\n", h, s, v);
            usb_print(temp_resp);
        } else {
            usb_print("\r\nError: Use H:0-360, S:0-100, V:0-100\r\n");
        }
    }
    else {
        usb_print("\r\nUnknown command\r\n");
    }
    
    usb_print("> ");
}

static void cdc_acm_user_ev_handler(app_usbd_class_inst_t const * p_inst,
                                    app_usbd_cdc_acm_user_event_t event)
{
    switch (event) {
        case APP_USBD_CDC_ACM_USER_EVT_PORT_OPEN:
            app_usbd_cdc_acm_read(&m_app_cdc_acm, &m_usb_rx_char, 1);
            pwm_set_rgb_values(0, 0, 1000); 
            break;
            
        case APP_USBD_CDC_ACM_USER_EVT_PORT_CLOSE:
            pwm_set_rgb_values(0, 0, 0);
            break;
            
        case APP_USBD_CDC_ACM_USER_EVT_RX_DONE:
        {
            fifo_put(m_usb_rx_char);
            app_usbd_cdc_acm_read(&m_app_cdc_acm, &m_usb_rx_char, 1);
            break;
        }
        default:
            break;
    }
}

static void usbd_user_ev_handler(app_usbd_event_type_t event)
{
    switch (event) {
        case APP_USBD_EVT_STOPPED:
            app_usbd_disable();
            break;
        case APP_USBD_EVT_POWER_DETECTED:
            if (!nrf_drv_usbd_is_enabled()) {
                app_usbd_enable();
            }
            break;
        case APP_USBD_EVT_POWER_REMOVED:
            app_usbd_stop();
            break;
        case APP_USBD_EVT_POWER_READY:
            app_usbd_start();
            break;
        default:
            break;
    }
}

void cli_init(void)
{
    static const app_usbd_config_t usbd_config = {
        .ev_state_proc = usbd_user_ev_handler
    };

    app_usbd_serial_num_generate();
    app_usbd_init(&usbd_config);
    app_usbd_class_append(app_usbd_cdc_acm_class_inst_get(&m_app_cdc_acm));
    
    app_usbd_enable();
    app_usbd_start();
}

void cli_process(void)
{
    while (app_usbd_event_queue_process()) {
    }

    char c;
    while (fifo_get(&c)) {
        
        if (c != '\r' && c != '\n') {
             usb_send_blocking(&c, 1);
        }

        if (c == '\r' || c == '\n') {
            m_line_buffer[m_line_idx] = 0; 
            if (m_line_idx > 0) {
                process_command(m_line_buffer);
            } else {
                usb_print("\r\n> ");
            }
            m_line_idx = 0;
        } 
        else if (c == 127 || c == 8) {
            if (m_line_idx > 0) {
                m_line_idx--;
                usb_print("\b \b");
            }
        }
        else if (c >= ' ' && c <= '~') {
            if (m_line_idx < sizeof(m_line_buffer) - 1) {
                m_line_buffer[m_line_idx++] = c;
            }
        }
    }
}