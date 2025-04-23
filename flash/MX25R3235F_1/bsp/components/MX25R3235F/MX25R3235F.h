/*
 ******************************************************************************
 * @file    mx25r3235f.h
 * @date    28-Dec-2022
 * @brief   This file contains all the function prototypes for interfacing
 *          external QSPI based Flash memory (MX25R3235F).
 ******************************************************************************
*/

#ifndef BSP_COMPONENTS_MX25R3235F_H
#define BSP_COMPONENTS_MX25R3235F_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stddef.h"
#include "math.h"
#include "stdbool.h"

/* Exported types ------------------------------------------------------------*/
typedef enum
{
    QSPI_ERASE_LEN_4KB  = 1,   /**< Erase 4 kB block. */
    QSPI_ERASE_LEN_64KB = 2,   /**< Erase 64 kB block. */
    QSPI_ERASE_LEN_ALL  = 3    /**< Erase all. */
} qspi_erase_len;

/* Exported macro ------------------------------------------------------------*/
#define FLASH_OK             0
#define FLASH_ERROR         -1

/* Exported functions prototypes ---------------------------------------------*/
esp_err_t MX25R3235F_Flash_Init(void);
esp_err_t MX25R3235F_Flash_DeInit(void);
esp_err_t MX25R3235F_Flash_Erase(uint32_t start_address, qspi_erase_len len);
esp_err_t MX25R3235F_Flash_Write(const void *p_tx_buffer, size_t tx_buffer_length, uint32_t dst_address);
esp_err_t MX25R3235F_Flash_Read(void *p_rx_buffer, size_t rx_buffer_length, uint32_t src_address);

#ifdef __cplusplus
}
#endif

#endif /* BSP_COMPONENTS_MX25R3235F_H */

/***************** (C) COPYRIGHT DotCom IoT *****END OF FILE*************************/
