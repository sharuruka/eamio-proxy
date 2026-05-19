#include <windows.h>

extern "C" {
#include "bemanitools/eamio.h"
#include "bemanitools/glue.h"
}

static log_formatter_t misc_logger;
static log_formatter_t info_logger;
static log_formatter_t warning_logger;
static log_formatter_t fatal_logger;

extern "C" void __cdecl eam_io_set_loggers(
    log_formatter_t misc,
    log_formatter_t info,
    log_formatter_t warning,
    log_formatter_t fatal)
{
    misc_logger = misc;
    info_logger = info;
    warning_logger = warning;
    fatal_logger = fatal;
}

extern "C" bool __cdecl eam_io_init(
    thread_create_t thread_create,
    thread_join_t thread_join,
    thread_destroy_t thread_destroy)
{
    info_logger("eamio-proxy", "Initializing card reader emulation proxy DLL");
    return true;
}

extern "C" void __cdecl eam_io_fini(void)
{
    info_logger("eamio-proxy", "Shutting down card reader emulation proxy DLL");
}

extern "C" uint16_t __cdecl eam_io_get_keypad_state(uint8_t unit_no)
{
    return 0;
}

extern "C" uint8_t __cdecl eam_io_get_sensor_state(uint8_t unit_no)
{
    return 0;
}

extern "C" uint8_t __cdecl eam_io_read_card(
    uint8_t unit_no, uint8_t *card_id, uint8_t nbytes)
{
    return 0;
}

extern "C" bool __cdecl eam_io_card_slot_cmd(uint8_t unit_no, uint8_t cmd)
{
    return false;
}

extern "C" bool __cdecl eam_io_poll(uint8_t unit_no)
{
    return true;
}

extern "C" const struct eam_io_config_api *__cdecl eam_io_get_config_api(void)
{
    return NULL;
}
