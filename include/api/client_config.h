#ifndef MCX_API_CLIENT_CONFIG_H
#define MCX_API_CLIENT_CONFIG_H

#include "api/client_lifecycle.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif
    
    /**
     * Loads INI configuration.
     * @param client The MCX client handle.
     * @param iniConfig The INI configuration string or path to the INI file.
     * @param isFromPath true if iniConfig is a path to the INI file, false if it is the INI configuration string.
     * @return STATUS_CODE_OK on success, otherwise an error status.
     */
    MCX_API OUT status_code_t CALLCLIENT loadIniFile(IN client_handler_t* client, IN const char* iniConfig, IN bool isFromPath);

#ifdef __cplusplus
}
#endif

#endif
