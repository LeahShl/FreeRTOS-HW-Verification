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

static struct netconn *conn_recv = NULL;
static struct netconn *conn_send = NULL;


/****************************
 * FUNCTION IMPLEMENTATION  *
 ****************************/

void UDP_Listen(void)
{
	struct netbuf *buf;
	void *raw_data;
	uint16_t len;
	InMsg_t in_msg;
	int n_read;

	conn_recv = netconn_new(NETCONN_UDP);
	if (conn_recv == NULL)
	{
		printf("error initializing UDP receive netconn\n");
		osThreadExit();
	}

	if (netconn_bind(conn_recv, IP_ADDR_ANY, SERVER_PORT) != ERR_OK)
	{
		printf("error binding UDP port\n");
		netconn_delete(conn_recv);
		conn_recv = NULL;
		osThreadExit();
	}

	while (1)
	{
		if (netconn_recv(conn_recv, &buf) == ERR_OK)
		{
			// Load in_msg
			in_msg.addr = *netbuf_fromaddr(buf);
			in_msg.port = netbuf_fromport(buf);

			netbuf_data(buf, &raw_data, &len);

			n_read = 0;
			memcpy(&in_msg.test_id, raw_data, sizeof(in_msg.test_id));
			n_read += sizeof(in_msg.test_id);

			memcpy(&in_msg.peripheral, &raw_data[n_read++], 1);
			memcpy(&in_msg.n_iter, &raw_data[n_read++], 1);
			memcpy(&in_msg.p_len, &raw_data[n_read++], 1);
			memcpy(&in_msg.payload, &raw_data[n_read], in_msg.p_len);

			// send in_msg to InMsgQueue
			osStatus_t status = osMessageQueuePut(inMsgQueueHandle, &in_msg, 0, osWaitForever);
			if (status != osOK)
			{
				printf("inMsg put error: %d\n", status);
			}

			netbuf_delete(buf);
		}
		else
		{
			printf("netconn_recv failed\n");
		}
	}
}

void UDP_Response(void)
{
	struct netbuf *buf;
	OutMsg_t out_msg;

	conn_send = netconn_new(NETCONN_UDP);
	if (conn_send == NULL)
	{
		printf("error initializing UDP send netconn\n");
		osThreadExit();
	}

	while(1)
	{
		if(osMessageQueueGet(outMsgQueueHandle, &out_msg, 0, osWaitForever) == osOK)
		{
			//Load response buffer
			buf = netbuf_new();
			if (!buf)
			{
				printf("responder netbuf_new() failed");
				continue;
			}

			void *data = netbuf_alloc(buf, RESPONSE_SIZE);
			if (!data)
			{
				printf("responder netbuf_alloc() failed");
			    netbuf_delete(buf);
			    continue;
			}
			memcpy(data, &out_msg.test_id, sizeof(out_msg.test_id));
			((uint8_t *)data)[sizeof(out_msg.test_id)] = out_msg.test_result;

			// Send response
			netconn_sendto(conn_send, buf, &out_msg.addr, out_msg.port);
			netbuf_delete(buf);
		}
	}
}
