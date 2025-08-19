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
 * FUNCTION DECLARETION     *
 ****************************/

err_t validate_data(InMsg_t *inmsg);

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
	err_t err;
	osStatus_t status;

	conn_recv = netconn_new(NETCONN_UDP);
	if (conn_recv == NULL)
	{
		LOG_ERR("error initializing UDP receive netconn");
		osThreadExit();
	}

	err = netconn_bind(conn_recv, IP_ADDR_ANY, SERVER_PORT);
	if (err != ERR_OK)
	{
		LOG_ERR("error binding UDP port (err code %d)", err);

		err = netconn_delete(conn_recv);
		if (err != ERR_OK)
			LOG_ERR("UDP_Listen couldn't delete netconn (err code %d)", err);
		conn_recv = NULL;
		osThreadExit();
	}

	while (1)
	{
		err = netconn_recv(conn_recv, &buf);
		if (err == ERR_OK)
		{
			// Load in_msg
			in_msg.addr = *netbuf_fromaddr(buf);
			in_msg.port = netbuf_fromport(buf);

			err = netbuf_data(buf, &raw_data, &len);
			if (err == ERR_OK)
			{
				n_read = 0;
				memcpy(&in_msg.test_id, raw_data, sizeof(in_msg.test_id));
				n_read += sizeof(in_msg.test_id);

				memcpy(&in_msg.peripheral, &raw_data[n_read++], 1);
				memcpy(&in_msg.n_iter, &raw_data[n_read++], 1);
				memcpy(&in_msg.p_len, &raw_data[n_read++], 1);
				memcpy(&in_msg.payload, &raw_data[n_read], in_msg.p_len);
				in_msg.payload[in_msg.p_len] = '\0'; // for printing

				err = validate_data(&in_msg);
				if (err == INMSG_OK)
				{
					LOG_INFO("UDP Listener got message: id=%lu plen=%d msg=%s",
							 in_msg.test_id, in_msg.p_len, in_msg.payload);

					// send in_msg to InMsgQueue
					status = osMessageQueuePut(inMsgQueueHandle, &in_msg, 0, osWaitForever);
					if (status != osOK)
					{
						LOG_ERR("inMsgQueue put error: %d\n", status);
					}
				}
				else
				{
					LOG_ERR("Invalid incoming message (err code %d)", err);
				}

				netbuf_delete(buf);
			}
			else
			{
				LOG_ERR("netbuf_data() error code: %d", err);
			}
		}
		else
		{
			LOG_ERR("UDP_Listen: netconn receive error (err code %d)", err);
		}
	}
}

void UDP_Response(void)
{
	struct netbuf *buf;
	OutMsg_t out_msg;
	osStatus_t status;
	err_t err;

	conn_send = netconn_new(NETCONN_UDP);
	if (conn_send == NULL)
	{
		LOG_ERR("error initializing UDP send netconn");
		osThreadExit();
	}

	while(1)
	{
		status = osMessageQueueGet(outMsgQueueHandle, &out_msg, 0, osWaitForever);
		if(status == osOK)
		{
			//Load response buffer
			buf = netbuf_new();
			if (!buf)
			{
				LOG_ERR("responder netbuf_new() failed");
				continue;
			}

			void *data = netbuf_alloc(buf, RESPONSE_SIZE);
			if (!data)
			{
				LOG_ERR("responder netbuf_alloc() failed");
			    netbuf_delete(buf);
			    continue;
			}

			memcpy(data, &out_msg.test_id, sizeof(out_msg.test_id));
			((uint8_t *)data)[sizeof(out_msg.test_id)] = out_msg.test_result;

			// Send response
			err = netconn_sendto(conn_send, buf, &out_msg.addr, out_msg.port);
			if (err != ERR_OK)
			{
				LOG_ERR("responder netconn_sendto() failed with error %d", err);
			}

			netbuf_delete(buf);
		}
	}
}

err_t validate_data(InMsg_t *inmsg)
{
	// check test ID
	// to be implemented

	if (!(inmsg->peripheral & ANY_TEST)) return INMSG_ERR_TID;

	if (inmsg->n_iter <= 0) return INMSG_ERR_ITER;

	if (inmsg->p_len <= 0) return INMSG_ERR_PLEN;

	return INMSG_OK;
}
