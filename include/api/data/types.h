#ifndef MCX_API_DATA_TYPES_H
#define MCX_API_DATA_TYPES_H

/**
 * Status code returned by functions
 * STATUS_CODE_ERR_INTERNAL --> An unexpected exception was caught.
 * STATUS_CODE_ERR_STATE    --> The library is in the wrong lifecycle state.
 * STATUS_CODE_ERR_SIP      --> PJSIP initialization or transport failed.
 * STATUS_CODE_ERR_CONFIG   --> Configuration is invalid.
 * STATUS_CODE_ERR_NULL     --> A required handle or pointer was NULL.
 * STATUS_CODE_OK           --> Operation succeeded. 
 * STATUS_CODE_ERROR        --> General failure.
 */
typedef enum status_code {
    STATUS_CODE_ERR_INTERNAL  = -99,
    STATUS_CODE_ERR_STATE     = -4,
    STATUS_CODE_ERR_SIP       = -3,
    STATUS_CODE_ERR_CONFIG    = -2,
    STATUS_CODE_ERR_NULL      = -1,
    STATUS_CODE_OK            = 0,
    STATUS_CODE_ERROR         = 1
} status_code_t;

/**
 * Boolean code returned by functions
 * BOOLEAN_FALSE --> False value.
 * BOOLEAN_TRUE  --> True value.
 */
typedef enum boolean_code {
    BOOLEAN_FALSE = 0,
    BOOLEAN_TRUE = 1
} boolean_code_t;

#endif
