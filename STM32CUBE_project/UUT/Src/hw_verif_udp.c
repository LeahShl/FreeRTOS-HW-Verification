/**
 * @file hw_verif_udp.c
 * @author leah
 * @date 25-06-2025
 *
 * @brief UDP communication for UUT tester program
 *
 */
#include "lwip/api.h"
#include "hw_verif_udp.h"
#include "hw_verif_sys.h"
#include <string.h>

/*************************
 * GLOBALS               *
 *************************/

extern osMessageQueueId_t inMsgQueueHandle;
extern osMessageQueueId_t outMsgQueueHandle;
extern osMutexId_t netconnMutexHandle;

struct netconn *conn = NULL;

/****************************
 * FUNCTION IMPLEMENTATION  *
 ****************************/

void UDP_Server_Init(void)
{

	err_t err;

	conn = netconn_new(NETCONN_UDP);
	if (conn == NULL)
	{
		printf("error creating UDP connection\n");
		while(1);
	}

	err = netconn_bind(conn, IP_ADDR_ANY, SERVER_PORT);
	if (err != ERR_OK)
	{
		printf("error binding UDP port\n");
		netconn_delete(conn);
		conn = NULL;
		while(1);
	}

	printf("UDP init success\n");
}

void UDP_Listen(void)
{
	printf("Listening started\n");
	struct netbuf *buf;
	InMsg_t in_msg;
	int n_read;

	while(1)
	{
		if(netconn_recv(conn, &buf) == ERR_OK)
		{
			printf("Got message\n");
			// Load in_msg
			ip_addr_copy(in_msg.addr, *netbuf_fromaddr(buf));
			in_msg.port = netbuf_fromport(buf);

			n_read = 0;
			netbuf_copy_partial(buf, &in_msg.test_id, sizeof(in_msg.test_id), n_read);
			n_read += sizeof(in_msg.test_id);

			netbuf_copy_partial(buf, &in_msg.peripheral, 1, n_read++);
			netbuf_copy_partial(buf, &in_msg.n_iter, 1, n_read++);
			netbuf_copy_partial(buf, &in_msg.p_len, 1, n_read++);
			netbuf_copy_partial(buf, &in_msg.payload, in_msg.p_len, n_read);

			if (in_msg.p_len < sizeof(in_msg.payload))
			    in_msg.payload[in_msg.p_len] = '\0';
			printf("Message is: %s\n", in_msg.payload);

		    // send in_msg to InMsgQueue
		    osMessageQueuePut(inMsgQueueHandle, &in_msg, 0, osWaitForever);

		    netbuf_delete(buf);
		}
		else
		{
			printf("ERROR\n");
			osDelay(1);
		}

		//netbuf_delete(buf);
	}
}

void UDP_Response(void)
{
	struct netbuf *buf;
	OutMsg_t out_msg;
	uint8_t response[RESPONSE_SIZE];

	while(1)
	{
		if(osMessageQueueGet(outMsgQueueHandle, &out_msg, 0, osWaitForever) == osOK)
		{
			//Load response buffer
			memcpy(response, &out_msg.test_id, sizeof(out_msg.test_id));
			response[sizeof(out_msg.test_id)] = out_msg.test_result;

			// Send response
			buf = netbuf_new();
			if (!buf) continue;

			netbuf_ref(buf, response, RESPONSE_SIZE);

			if (osMutexAcquire(netconnMutexHandle, osWaitForever) == osOK)
			{
				netconn_connect(conn, &out_msg.addr, out_msg.port);
				netconn_send(conn, buf);
				netconn_disconnect(conn);
				osMutexRelease(netconnMutexHandle);
			}
		}
		osDelay(1);
	}
}
