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

/**
 * @brief Holds data for incoming communication
 *
 * Data about the original sender is saved in order to send a response back.
 *
 * @struct InMsg
 */
typedef struct InMsg
{
	struct netconn *conn;          /** Source netconn object */

	uint32_t test_id;              /** Unique test ID */
	uint8_t peripheral;            /** Peripheral code */
	uint8_t n_iter;                /** Number of iterations */
	uint8_t p_len;                 /** Payload length */
	char payload[256];             /** Payload buffer */
}InMsg_t;

/**
 * @brief Holds data for outgoing communication
 *
 * @struct OutMsg
 */
typedef struct OutMsg
{
	struct netconn *conn;          /** Destination netconn object */

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
	struct netconn *conn;          /** Source netconn object */

	uint32_t test_id;              /** Unique test ID */
	uint8_t n_iter;                /** Number of iterations */
	uint8_t p_len;                 /** Payload length */
	char payload[256];             /** Payload buffer */
}TestData_t;

#endif /* INC_HW_VERIF_SYS_H_ */
