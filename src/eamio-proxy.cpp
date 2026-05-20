#include <fstream>
#include <string>
#include <vector>
#include <windows.h>

extern "C" {
#include "bemanitools/eamio.h"
#include "bemanitools/glue.h"
}

#include "eamio-proxy.hpp"

static log_formatter_t misc_logger;
static log_formatter_t info_logger;
static log_formatter_t warning_logger;
static log_formatter_t fatal_logger;

static bool proxy_initialized = false;

static std::vector<eamio_dll_target_t> eamio_dll_targets;

static eamio_dll_target_t
    *inserted_eamio_dll_targets[NUMBER_OF_EMULATED_READERS];

/* Initialize eamio-proxy by dynamically loading all DLL modules listed in the
 * config file. */

bool proxy_init()
{
    std::ifstream config_file(PROXY_CFG_PATH);

    if (!config_file.is_open()) {
        fatal_logger(
            PROXY_MODULE_NAME, "Failed to open config file: " PROXY_CFG_PATH);
        return false;
    }

    std::string dll_name;

    while (std::getline(config_file, dll_name)) {

        /* Ignore malformed lines. */
        if (!dll_name.ends_with(".dll")) {
            warning_logger(
                PROXY_MODULE_NAME,
                "Ignoring malformed line in config file: %s",
                dll_name.c_str());
            continue;
        }

        info_logger(
            PROXY_MODULE_NAME,
            "Listing EAMIO DLL target: %s",
            dll_name.c_str());

        HMODULE dll_handle = LoadLibraryA(dll_name.c_str());

        if (dll_handle == NULL) {
            fatal_logger(
                PROXY_MODULE_NAME,
                "Failed to load EAMIO DLL: %s",
                dll_name.c_str());
            return false;
        }

        info_logger(
            PROXY_MODULE_NAME, "Loaded EAMIO DLL target: %s", dll_name.c_str());

        eamio_functions_t functions;

        functions.eam_io_set_loggers = (eam_io_set_loggers_t) GetProcAddress(
            dll_handle, "eam_io_set_loggers");

        functions.eam_io_init =
            (eam_io_init_t) GetProcAddress(dll_handle, "eam_io_init");

        functions.eam_io_fini =
            (eam_io_fini_t) GetProcAddress(dll_handle, "eam_io_fini");

        functions.eam_io_get_keypad_state =
            (eam_io_get_keypad_state_t) GetProcAddress(
                dll_handle, "eam_io_get_keypad_state");

        functions.eam_io_get_sensor_state =
            (eam_io_get_sensor_state_t) GetProcAddress(
                dll_handle, "eam_io_get_sensor_state");

        functions.eam_io_read_card =
            (eam_io_read_card_t) GetProcAddress(dll_handle, "eam_io_read_card");

        functions.eam_io_card_slot_cmd =
            (eam_io_card_slot_cmd_t) GetProcAddress(
                dll_handle, "eam_io_card_slot_cmd");

        functions.eam_io_poll =
            (eam_io_poll_t) GetProcAddress(dll_handle, "eam_io_poll");

        functions.eam_io_get_config_api =
            (eam_io_get_config_api_t) GetProcAddress(
                dll_handle, "eam_io_get_config_api");

        if (!functions.eam_io_set_loggers || !functions.eam_io_init ||
            !functions.eam_io_fini || !functions.eam_io_get_keypad_state ||
            !functions.eam_io_get_sensor_state || !functions.eam_io_read_card ||
            !functions.eam_io_card_slot_cmd || !functions.eam_io_poll ||
            !functions.eam_io_get_config_api) {

            fatal_logger(
                PROXY_MODULE_NAME,
                "At least one missing eamio function in the target DLL: %s",
                dll_name.c_str());

            return false;
        }

        eamio_dll_targets.push_back({functions, dll_name, dll_handle});
    }

    config_file.close();
    return true;
}

/* eam_io_set_loggers is always the first function being called so it's treated
 * as the entrypoint to load all modules listed in the config file. */

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

    proxy_initialized = proxy_init();

    if (!proxy_initialized) {
        return;
    }

    for (const auto &eamio_dll_target : eamio_dll_targets) {
        eamio_dll_target.functions.eam_io_set_loggers(
            misc, info, warning, fatal);
    }
}

extern "C" bool __cdecl eam_io_init(
    thread_create_t thread_create,
    thread_join_t thread_join,
    thread_destroy_t thread_destroy)
{
    if (!proxy_initialized) {
        return false;
    }

    for (const auto &eamio_dll_target : eamio_dll_targets) {
        info_logger(
            PROXY_MODULE_NAME,
            "Initializing card reader emulation DLL: %s",
            eamio_dll_target.dll_name.c_str());
        if (!eamio_dll_target.functions.eam_io_init(
                thread_create, thread_join, thread_destroy)) {
            warning_logger(
                PROXY_MODULE_NAME,
                "Failed to initialize card reader emulation DLL: %s",
                eamio_dll_target.dll_name.c_str());
            return false;
        }
    }

    return true;
}

extern "C" void __cdecl eam_io_fini(void)
{
    info_logger(
        PROXY_MODULE_NAME, "Shutting down card reader emulation proxy DLL");

    for (const auto &eamio_dll_target : eamio_dll_targets) {
        eamio_dll_target.functions.eam_io_fini();
        FreeLibrary(eamio_dll_target.handle);
    }

    eamio_dll_targets.clear();
    proxy_initialized = false;
}

extern "C" uint16_t __cdecl eam_io_get_keypad_state(uint8_t unit_no)
{
    uint16_t proxy_state = 0;
    for (const auto &eamio_dll_target : eamio_dll_targets) {
        proxy_state |=
            eamio_dll_target.functions.eam_io_get_keypad_state(unit_no);
    }
    return proxy_state;
}

/* Sensor state is represented by a 2-bit value. Lower bit is front sensor,
   upper bit is back sensor. Give priority to active readers by returning
   highest value. Save the active reader that had a card inserted for later use
   when the game tries to read the card. */

extern "C" uint8_t __cdecl eam_io_get_sensor_state(uint8_t unit_no)
{
    if (unit_no >= NUMBER_OF_EMULATED_READERS) {
        return 0;
    }

    inserted_eamio_dll_targets[unit_no] = NULL;
    uint8_t proxy_state = 0;

    for (auto &eamio_dll_target : eamio_dll_targets) {
        const uint8_t state =
            eamio_dll_target.functions.eam_io_get_sensor_state(unit_no);
        if (state > proxy_state) {
            proxy_state = state;
            inserted_eamio_dll_targets[unit_no] = &eamio_dll_target;
        }
    }
    return proxy_state;
}

extern "C" uint8_t __cdecl eam_io_read_card(
    uint8_t unit_no, uint8_t *card_id, uint8_t nbytes)
{
    if (unit_no >= NUMBER_OF_EMULATED_READERS) {
        return EAM_IO_CARD_NONE;
    }

    if (inserted_eamio_dll_targets[unit_no]) {
        return inserted_eamio_dll_targets[unit_no]->functions.eam_io_read_card(
            unit_no, card_id, nbytes);
    }

    return EAM_IO_CARD_NONE;
}

/* Not too sure about how slotted readers should be treated, haven't got any to
 * test. Return false only if all forwarded card slot commands return false. */

extern "C" bool __cdecl eam_io_card_slot_cmd(uint8_t unit_no, uint8_t cmd)
{
    bool result = false;
    for (const auto &eamio_dll_target : eamio_dll_targets) {
        result |= eamio_dll_target.functions.eam_io_card_slot_cmd(unit_no, cmd);
    }
    return result;
}

/* The point is to forward polls so that eamio implementations can update
 * their state. Return value shouldn't really matter, return false only if
 * all forwarded polls return false. */

extern "C" bool __cdecl eam_io_poll(uint8_t unit_no)
{
    bool result = false;
    for (const auto &eamio_dll_target : eamio_dll_targets) {
        result |= eamio_dll_target.functions.eam_io_poll(unit_no);
    }
    return result;
}

/* Will allow config.exe to work if user decides to use the default
 * bemanitools keyboard implementation of eamio. */

extern "C" const struct eam_io_config_api *__cdecl eam_io_get_config_api(void)
{
    for (const auto &eamio_dll_target : eamio_dll_targets) {
        const struct eam_io_config_api *config_api =
            eamio_dll_target.functions.eam_io_get_config_api();
        if (config_api)
            return config_api;
    }
    return NULL;
}
