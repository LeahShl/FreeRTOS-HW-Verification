/**
 * @file dispatcher.h
 * @author leah
 * @date 27-06-2025
 *
 * @brief UUT Hardware verification system dispatcher api
 *
 */

#ifndef INC_DISPATCHER_H_
#define INC_DISPATCHER_H_

/**
 * @brief Read messages from InMsg queue and send them to the relevant peripheral test queue
 */
void TestDispatcher(void);

#endif /* INC_DISPATCHER_H_ */
