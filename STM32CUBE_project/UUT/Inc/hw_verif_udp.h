/**
 * @file hw_verif_udp.h
 * @author leah
 * @date 24-06-2025
 * 
 * @brief UDP communication header for UUT tester program
 * 
 */

#ifndef INC_HW_VERIF_UDP_H_
#define INC_HW_VERIF_UDP_H_

#include "hw_verif_sys.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*************************
 * MACROS                *
 *************************/

#define SERVER_PORT 54321          // Port for UDP communication
#define RESPONSE_SIZE 5            // Response is always 5 bytes

#define INMSG_OK 0                 // InMsg validated successfully
#define INMSG_ERR_TID -1           // Test ID validation error
#define INMSG_ERR_ITER -2          // Number of iterations validation error
#define INMSG_ERR_PLEN -3          // payload length validation error
#define INMSG_ERR_PLD -4           // Payload validation error

/*************************
 * FUNCTION DECLARATIONS *
 *************************/

/**
 * @brief Listens to UDP port and loads read messages to InMsgQueue
 */
void UDP_Listen(void);

/**
 * @brief Reads from OutMsgQueue and sends requests accordingly
 */
void UDP_Response(void);

#ifdef __cplusplus
}
#endif

#endif /* INC_HW_VERIF_UDP_H_ */
