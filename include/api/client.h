#ifndef MCX_API_CLIENT_H
#define MCX_API_CLIENT_H

#include "api/data/definitions.h"
#include "api/data/types.h"

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct client_handler client_handler_t;

    /**
     * Starts the library.
     * @return client_handler_t object.
     */
    MCX_API OUT client_handler_t* CALLCLIENT init(IN void);

    /**
     * Detroys the library.
     * @param client_handler_t object.
     * @return status_code_t STATUS_CODE_OK when if it goes right, otherwise STATUS_CODE_ERROR.
     */
    MCX_API OUT status_code_t CALLCLIENT deinit(IN client_handler_t* client);

    /**
     * Loads INI config.
     * @param client The MCX client struct
     * @param iniConfig The INI configuration string or path to the INI file.
     * @param isFromPath true if iniConfig is a path to the INI file, false if it is the INI configuration string.
     * @return status_code_t STATUS_CODE_OK when if it goes right, otherwise STATUS_CODE_ERROR.
     */
    MCX_API OUT status_code_t CALLCLIENT loadIniFile(IN client_handler_t* client, IN const char* iniConfig, IN bool isFromPath);
#ifdef __cplusplus
}
#endif

#endif
