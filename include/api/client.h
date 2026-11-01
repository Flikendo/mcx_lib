#pragma once

#include "api/data/types.h"
#include "api/data/definitions.h"

extern "C" {
    typedef struct client_handler client_handler_t;

    /**
     * Starts the library.
     * @return client_handler_t object.
     */
    OUT client_handler_t* CALLCLIENT init(IN void);

    /**
     * Detroys the library.
     * @param client_handler_t object.
     * @return status_code_t STATUS_CODE_OK when if it goes right, otherwise STATUS_CODE_ERROR.
     */
    OUT status_code_t CALLCLIENT deinit(IN client_handler_t* client);

    /**
     * Loads INI config from path file.
     * @param path Path to the INI file containing the configuration sections.
     */
    OUT status_code_t CALLCLIENT loadIniFileFromPath(IN client_handler_t* client, IN const char* pathFile);
    
    /**
     * Loads INI config from string.
     * @param iniConfig config already read from path.
     * @return status_code_t STATUS_CODE_OK when if it goes right, otherwise STATUS_CODE_ERROR.status_code_t STATUS_CODE_OK when if it goes right, otherwise STATUS_CODE_ERROR.
     */
    OUT status_code_t CALLCLIENT loadIniFileFromString(IN client_handler_t* client, const char* iniConfig);
}