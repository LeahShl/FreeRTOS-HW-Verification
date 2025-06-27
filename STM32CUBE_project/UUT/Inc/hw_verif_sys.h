/**
 * @file hw_verif_sys.h
 * @author leah
 * @date 24-06-2025
 *
 * @brief UUT Hardware verification system definitions
 *
 */

#ifndef INC_HW_VERIF_SYS_H_
#define INC_HW_VERIF_SYS_H_

#include "lwip/ip_addr.h"
#include "cmsis_os2.h"

/*************************
 * MACROS                *
 *************************/

/**
 * @brief Peripheral codes
 */
#define TEST_TIM 1                 // Timer test code
#define TEST_UART 2                // UART test code
#define TEST_SPI 4                 // SPI test code
#define TEST_I2C 8                 // I2C test code
#define TEST_ADC 16                // ADC test code

/**
 * @brief Limits
 */
#define MAX_BUF 256

/**
 * @brief Available test results
 */
#define TEST_SUCCESS 0x01
#define TEST_FAILED 0xff

/*************************
 * TYPESDEFS             *
 *************************/

/**
 * @brief Holds data for incoming communication
 *
 * Data about the original sender is saved in order to send a response back.
 *
 * @struct InMsg
 */
typedef struct InMsg
{
	ip_addr_t addr;                /** Source IP address */
	u16_t port;                    /** Source port */

	uint32_t test_id;              /** Unique test ID */
	uint8_t peripheral;            /** Peripheral code */
	uint8_t n_iter;                /** Number of iterations */
	uint8_t p_len;                 /** Payload length */
	char payload[MAX_BUF];         /** Payload buffer */
}InMsg_t;

/**
 * @brief Holds data for outgoing communication
 *
 * @struct OutMsg
 */
typedef struct OutMsg
{
	ip_addr_t addr;                /** Destination IP address */
	u16_t port;                    /** Destination port */

	uint32_t test_id;              /** Unique test ID */
	uint8_t test_result;           /** Test result (success/fail) */
}OutMsg_t;

/**
 * @brief Holds data for a single peripheral test
 *
 * @struct TestData
 */
typedef struct TestData
{
	ip_addr_t addr;                /** Source IP address */
	u16_t port;                    /** Source port */

	uint32_t test_id;              /** Unique test ID */
	uint8_t n_iter;                /** Number of iterations */
	uint8_t p_len;                 /** Payload length */
	char payload[MAX_BUF];         /** Payload buffer */
}TestData_t;

#endif /* INC_HW_VERIF_SYS_H_ */
