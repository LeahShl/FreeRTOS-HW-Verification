/**
 * @file hw_verif_crc.c
 * @author leah
 * @date 24-06-2025
 * 
 * @brief CRC check implementation for UUT tester program
 * 
 */
#include "cmsis_os2.h"
#include "crc.h"
#include "hw_verif_crc.h"
#include "log.h"

/****************************
 * GLOBALS                  *
 ****************************/

static osMutexId_t crcMutex;

/****************************
 * FUNCTION IMPLEMENTATION  *
 ****************************/

int Init_CRC()
{
	// init mutex only if needed
	if (crcMutex == NULL) crcMutex = osMutexNew(NULL);

	if (crcMutex == NULL)
	{
		LOG_ERR("Init_CRC couldn't init mutex");
		return CRC_MATCH_FAIL;
	}

	return CRC_MATCH_OK;
}

int Match_CRC(uint8_t *buf1, uint8_t buf1_len, uint8_t *buf2, uint8_t buf2_len)
{
	if (crcMutex == NULL)
	{
		LOG_ERR("CRC mutex is NULL. Must call Init_CRC beforehand");
		return CRC_MATCH_FAIL;
	}

	while (osMutexAcquire(crcMutex, osDelay(CRC_MTX_TO)) != osOK)
	{
		LOG_WARN("Match_CRC hit timeout while trying to acquire mutex");
	}
	uint32_t buf1_crc = HAL_CRC_Calculate(&hcrc, (uint32_t *)buf1, (uint32_t) buf1_len);
	uint32_t buf2_crc = HAL_CRC_Calculate(&hcrc, (uint32_t *)buf2, (uint32_t) buf2_len);
	if (osMutexRelease(crcMutex) != osOK)
	{
		LOG_ERR("Match_CRC couldn't release mutex");
	}

	if(buf1_crc == buf2_crc)
		return CRC_MATCH_OK;

	return CRC_MATCH_FAIL;
}
