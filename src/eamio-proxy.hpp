#include <string>
#include <windows.h>

extern "C" {
#include "bemanitools/glue.h"
}

#define NUMBER_OF_EMULATED_READERS 2
#define PROXY_CFG_PATH "eamio-proxy.conf"
#define PROXY_MODULE_NAME "eamio-proxy"

/* bemanitools bt5 api eamio function types */

typedef void (*eam_io_set_loggers_t)(
    log_formatter_t misc,
    log_formatter_t info,
    log_formatter_t warning,
    log_formatter_t fatal);

typedef bool (*eam_io_init_t)(
    thread_create_t thread_create,
    thread_join_t thread_join,
    thread_destroy_t thread_destroy);

typedef void (*eam_io_fini_t)(void);

typedef uint16_t (*eam_io_get_keypad_state_t)(uint8_t unit_no);

typedef uint8_t (*eam_io_get_sensor_state_t)(uint8_t unit_no);

typedef uint8_t (*eam_io_read_card_t)(
    uint8_t unit_no, uint8_t *card_id, uint8_t nbytes);

typedef bool (*eam_io_card_slot_cmd_t)(uint8_t unit_no, uint8_t cmd);

typedef bool (*eam_io_poll_t)(uint8_t unit_no);

typedef const struct eam_io_config_api *(*eam_io_get_config_api_t)(void);

/* Structs for storing module and function references */

typedef struct {
    eam_io_set_loggers_t eam_io_set_loggers;
    eam_io_init_t eam_io_init;
    eam_io_fini_t eam_io_fini;
    eam_io_get_keypad_state_t eam_io_get_keypad_state;
    eam_io_get_sensor_state_t eam_io_get_sensor_state;
    eam_io_read_card_t eam_io_read_card;
    eam_io_card_slot_cmd_t eam_io_card_slot_cmd;
    eam_io_poll_t eam_io_poll;
    eam_io_get_config_api_t eam_io_get_config_api;
} eamio_functions_t;

typedef struct {
    eamio_functions_t functions;
    std::string dll_name;
    HMODULE handle;
} eamio_dll_target_t;
