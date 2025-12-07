#include "usb_cli.h"
#include "app_config.h"
#include "app_usbd.h"
#include "app_usbd_cdc_acm.h"
#include "app_usbd_serial_num.h"
#include "app_usbd_core.h"
#include "hsv.h"

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

static char m_rx_buffer[1];
static char m_line_buffer[128];
static uint8_t m_line_idx = 0;

APP_USBD_CDC_ACM_GLOBAL_DEF(m_app_cdc_acm,
                            cdc_acm_user_ev_handler,
                            CDC_ACM_COMM_INTERFACE,
                            CDC_ACM_DATA_INTERFACE,
                            CDC_ACM_COMM_EPIN,
                            CDC_ACM_DATA_EPIN,
                            CDC_ACM_DATA_EPOUT,
                            APP_USBD_CDC_COMM_PROTOCOL_AT_V250);


static void usb_print(const char *msg) {
    size_t len = strlen(msg);
    app_usbd_cdc_acm_write(&m_app_cdc_acm, msg, len);
}

static void process_command(char *cmd) {
    char response[100];
    char *token = strtok(cmd, " \r\n");

    if (token == NULL) return;

    if (strcasecmp(token, "help") == 0) {
        usb_print("\r\nSupported commands:\r\n");
        usb_print("  RGB <red> <green> <blue>     Set RGB color (0-1000)\r\n");
        usb_print("  HSV <hue> <sat> <val>        Set HSV color (0-360, 0-100, 0-100)\r\n");
        usb_print("  help                         Show this help\r\n");
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
            
            sprintf(response, "\r\nColor set to R=%d G=%d B=%d\r\n", r, g, b);
            usb_print(response);
        } else {
            usb_print("\r\nUnknown command (Invalid RGB arguments)\r\n");
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

            sprintf(response, "\r\nColor set to H=%d S=%d V=%d\r\n", h, s, v);
            usb_print(response);
        } else {
            usb_print("\r\nUnknown command (Invalid HSV arguments)\r\n");
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
            app_usbd_cdc_acm_read(&m_app_cdc_acm, m_rx_buffer, 1);
            usb_print("\r\nHSV CLI Ready. Type 'help'.\r\n> ");
            break;
            
        case APP_USBD_CDC_ACM_USER_EVT_PORT_CLOSE:
            break;
            
        case APP_USBD_CDC_ACM_USER_EVT_RX_DONE:
        {
            char c = m_rx_buffer[0];
            
            app_usbd_cdc_acm_write(&m_app_cdc_acm, m_rx_buffer, 1);

            if (c == '\r') {
                m_line_buffer[m_line_idx] = 0;
                process_command(m_line_buffer);
                m_line_idx = 0;
            } 
            else if (c == 127 || c == 8) {
                if (m_line_idx > 0) m_line_idx--;
            }
            else if (c >= ' ' && c <= '~') {
                if (m_line_idx < sizeof(m_line_buffer) - 1) {
                    m_line_buffer[m_line_idx++] = c;
                }
            }

            app_usbd_cdc_acm_read(&m_app_cdc_acm, m_rx_buffer, 1);
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
}