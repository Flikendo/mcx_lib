#pragma once

#include "helper_tool/log_helper.hpp"

#include <pjsip.h>
#include <pjlib.h>

/**
 * @file client_impl.hpp
 * @brief The ClientImpl class provides an implementation for managing the lifecycle of a SIP client.
 * 
 * The ClientImpl class encapsulates the functionality required to start and stop a SIP client, handling the necessary
 * initialization and cleanup of resources. It interacts with the underlying PJSIP library to manage SIP sessions,
 * connections, and communication with SIP/IMS infrastructure. The class is designed to be used in conjunction with 
 * the Config class, which provides the necessary configuration settings for the client.
 */
namespace sip {
    class ClientImpl {
        public:
            /**
             * Constructs a new ClientImpl object.
             * Does not initialize PJSIP; call start() explicitly.
             */
            ClientImpl();

            /**
             * Destroys the ClientImpl object.
             * Cleans up resources and shuts down the PJSIP library.
             */
            ~ClientImpl();

            /**
             * Starts the SIP client, establishing connections and preparing for communication.
             * @return PJ_SUCCESS on success, otherwise an error status.
             */
            pj_status_t start();

            /**
             * Stops the SIP client, closing connections and cleaning up resources.
             * @return PJ_SUCCESS on success, otherwise an error status.
             */
            pj_status_t stop();

        private:
            /**
             * The PJSIP endpoint used for managing SIP sessions and communication.
             */
            pjsip_endpoint* endpoint = nullptr;

            /**
             * The caching pool used for memory management within the PJSIP library.
             * It is initialized during the start process and cleaned up during the stop process. 
             */
            pj_caching_pool cp{};

            /**
             * Indicates whether the PJSIP library has been initialized.
             */
            bool pjInitialized = false;

            /**
             * Indicates whether the PJSIP caching pool has been initialized.
             */
            bool poolInitialized = false;

            /**
             * Logger instance for logging configuration-related messages.
             * This can be used to log errors, warnings, or informational messages related to configuration processing
             */
            Logger logger;
    };
}