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

/*************************
 * MACROS                *
 *************************/

#define SERVER_PORT 54321          // Port for UDP communication

/*************************
 * FUNCTION DECLARATIONS *
 *************************/

/**
 * @brief Initiates the udp server
 */
void UDP_Server_Init();




#endif /* INC_HW_VERIF_UDP_H_ */
