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

void UDP_Server_Init(void)
{
    conn_recv = netconn_new(NETCONN_UDP);
	if (conn_recv == NULL)
	{
		printf("error initializing UDP receive netconn\n");
		while(1);
	}

	if (netconn_bind(conn_recv, IP_ADDR_ANY, SERVER_PORT) != ERR_OK)
	{
		printf("error binding UDP port\n");
		netconn_delete(conn_recv);
		conn_recv = NULL;
		while(1);
	}

	netconn_set_recvtimeout(conn_recv, 100);

	conn_send = netconn_new(NETCONN_UDP);
	if (conn_send == NULL)
	{
		printf("error initializing UDP send netconn\n");
		while(1);
	}

	printf("UDP init success\n");
}

void UDP_Listen(void)
{
	printf("Listening started\n");
	struct netbuf *buf;
	void *raw_data;
	uint16_t len;
	InMsg_t in_msg;
	int n_read;
	err_t err;


	while(1)
	{
		//printf("listener alive\n");
		err = netconn_recv(conn_recv, &buf);
	    if(err == ERR_OK)
		{
			printf("listener got a message\n");
			// Load in_msg
			ip_addr_copy(in_msg.addr, *netbuf_fromaddr(buf));
			in_msg.port = netbuf_fromport(buf);

			netbuf_data(buf, &raw_data, &len);
			netbuf_delete(buf); // finished with buf

			n_read = 0;
			memcpy(&in_msg.test_id, raw_data, sizeof(in_msg.test_id));
			n_read += sizeof(in_msg.test_id);

			memcpy(&in_msg.peripheral, &raw_data[n_read++], 1);
			memcpy(&in_msg.n_iter, &raw_data[n_read++], 1);
			memcpy(&in_msg.p_len, &raw_data[n_read++], 1);
			memcpy(&in_msg.payload, &raw_data[n_read], in_msg.p_len);

			/*
			n_read = 0;
			netbuf_copy_partial(buf, &in_msg.test_id, sizeof(in_msg.test_id), n_read);
			n_read += sizeof(in_msg.test_id);

			netbuf_copy_partial(buf, &in_msg.peripheral, 1, n_read++);
			netbuf_copy_partial(buf, &in_msg.n_iter, 1, n_read++);
			netbuf_copy_partial(buf, &in_msg.p_len, 1, n_read++);
			netbuf_copy_partial(buf, &in_msg.payload, in_msg.p_len, n_read);

			netbuf_delete(buf); // finished with buf
            */


			if (in_msg.p_len < sizeof(in_msg.payload))
				in_msg.payload[in_msg.p_len] = '\0';
			printf("Peripheral is: %d\n", in_msg.peripheral);
			printf("n iterations is: %d\n", in_msg.n_iter);
			printf("Message is: %s\n", in_msg.payload);

			// send in_msg to InMsgQueue
			if (osMessageQueuePut(inMsgQueueHandle, &in_msg, 0, 10) != osOK)
			{
				printf("inMsg full, dropped msg\n");
			}
		}
	    else if (err == ERR_TIMEOUT)
	    {
	    	osDelay(0);
	    }
	    else
	    {
	    	printf("UDP receive error: %d\n", err);
	    	osDelay(1);
	    }
	}
}

void UDP_Response(void)
{
	struct netbuf *buf;
	OutMsg_t out_msg;

	while(1)
	{
		//printf("IN responder\n");
		if(osMessageQueueGet(outMsgQueueHandle, &out_msg, 0, 10) == osOK)
		{
			printf("responder got a response to send\n");
			//Load response buffer
			buf = netbuf_new();
			if (!buf)
			{
				continue;
			}

			void *data = netbuf_alloc(buf, RESPONSE_SIZE);
			if (!data)
			{
			    netbuf_delete(buf);
			    continue;
			}
			memcpy(data, &out_msg.test_id, sizeof(out_msg.test_id));
			((uint8_t *)data)[sizeof(out_msg.test_id)] = out_msg.test_result;

			// Send response
			if (netconn_sendto(conn_send, buf, &out_msg.addr, out_msg.port) != ERR_OK)
			{
				printf("Error sending response\n");
			}
			else
			{
				printf("Error connecting to client\n");
			}
			netbuf_delete(buf);
		}
		else
		{
			osDelay(1);
		}
	}
}
