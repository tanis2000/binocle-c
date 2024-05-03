//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#ifndef BINOCLE_LOG_H
#define BINOCLE_LOG_H

/**
 * \brief Logs an error
 * @param msg The message to log
 * @param ... The parameters, if any
 */
void binocle_log_error(const char *msg, ...);

/**
 * \brief Logs a warning
 * @param msg The message to log
 * @param ... The parameters, if any
 */
void binocle_log_warning(const char *msg, ...);

/**
 * \brief Logs an information message
 * @param msg The message to log
 * @param ... The parameters, if any
 */
void binocle_log_info(const char *msg, ...);

/**
 * \brief Logs a debug message
 * @param msg The message to log
 * @param ... The parameters, if any
 */
void binocle_log_debug(const char *msg, ...);

/**
 * \brief Ensures that the buffer used to temporarily hold the formatted message is available
 */
void binocle_log_ensure_buffer_is_set();

#endif //BINOCLE_LOG_H
