/*
 * log.h
 *
 *  Created on: Aug 19, 2025
 *      Author: leah
 */

#ifndef INC_LOG_H_
#define INC_LOG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

/* Log level flags */
#define LOG_LVL_I (1 << 0)
#define LOG_LVL_W (1 << 1)
#define LOG_LVL_E (1 << 2)

/* Default configuration (user can override before including log.h) */
#ifndef HWT_LOGS_ON
#define HWT_LOGS_ON 0
#endif

#ifndef HWT_LOG_LVL
#define HWT_LOG_LVL (LOG_LVL_I | LOG_LVL_W | LOG_LVL_E)
#endif

/* Info log */
#if (HWT_LOGS_ON && (HWT_LOG_LVL & LOG_LVL_I))
#define LOG_INFO(fmt, ...)    printf("[INFO] " fmt "\n", ##__VA_ARGS__)
#else
#define LOG_INFO(fmt, ...)    ((void)0)
#endif

#if (HWT_LOGS_ON && (HWT_LOG_LVL & LOG_LVL_W))
#define LOG_WARN(fmt, ...)    printf("[WARN] " fmt "\n", ##__VA_ARGS__)
#else
#define LOG_WARN(fmt, ...)    ((void)0)
#endif

#if (HWT_LOGS_ON && (HWT_LOG_LVL & LOG_LVL_E))
#define LOG_ERR(fmt, ...)     printf("[ERR] " fmt "\n", ##__VA_ARGS__)
#else
#define LOG_ERR(fmt, ...)     ((void)0)
#endif

#ifdef __cplusplus
}
#endif

#endif /* INC_LOG_H_ */
