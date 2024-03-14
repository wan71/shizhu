/**
 * Copyright (c) 2019, Freqchip
 * 
 * All rights reserved.
 * 
 * 
 */

#ifndef SP_GATT_PROFILE_H
#define SP_GATT_PROFILE_H

/*
 * INCLUDES (����ͷ�ļ�)
 */
#include <stdio.h>
#include <string.h>
#include "gap_api.h"
#include "gatt_api.h"
#include "gatt_sig_uuid.h"


/*
 * MACROS (�궨��)
 */

/*
 * CONSTANTS (��������)
 */
// Simple Profile attributes index. 
enum
{
    SP_IDX_SERVICE,

    SP_IDX_CHAR1_DECLARATION,
    SP_IDX_CHAR1_VALUE,
	SP_IDX_CHAR1_CFG,
    SP_IDX_CHAR1_USER_DESCRIPTION,

    SP_IDX_CHAR2_DECLARATION,
    SP_IDX_CHAR2_VALUE,
    SP_IDX_CHAR2_USER_DESCRIPTION,

    SP_IDX_CHAR3_DECLARATION,
    SP_IDX_CHAR3_VALUE,
    SP_IDX_CHAR3_USER_DESCRIPTION,

    SP_IDX_CHAR4_DECLARATION,
    SP_IDX_CHAR4_VALUE,
    SP_IDX_CHAR4_CFG,
    SP_IDX_CHAR4_USER_DESCRIPTION,
    
    SP_IDX_CHAR5_DECLARATION,
    SP_IDX_CHAR5_VALUE,
    SP_IDX_CHAR5_USER_DESCRIPTION,
    
    SP_IDX_NB,
};

// Simple GATT Profile Service UUID
#define SP_SVC_UUID              0xFFF0

#define SP_CHAR1_TX_UUID            {0xb8, 0x5c, 0x49, 0xd2, 0x04, 0xa3, 0x40, 0x71, 0xa0, 0xb5, 0x35, 0x85, 0x3e, 0xb0, 0x83, 0x07}
#define SP_CHAR2_RX_UUID            {0xba, 0x5c, 0x49, 0xd2, 0x04, 0xa3, 0x40, 0x71, 0xa0, 0xb5, 0x35, 0x85, 0x3e, 0xb0, 0x83, 0x07}
#define SP_CHAR3_UUID            0xFFF3
#define SP_CHAR4_UUID            0xFFF4
#define SP_CHAR5_UUID            0xFFF5

/*
 * TYPEDEFS (���Ͷ���)
 */

/*
 * GLOBAL VARIABLES (ȫ�ֱ���)
 */
extern uint8_t sp_svc_id;
/*
 * LOCAL VARIABLES (���ر���)
 */


/*
 * PUBLIC FUNCTIONS (ȫ�ֺ���)
 */
/*********************************************************************
 * @fn      sp_gatt_add_service
 *
 * @brief   Simple Profile add GATT service function.
 *			����GATT service��ATT�����ݿ����档
 *
 * @param   None. 
 *        
 *
 * @return  None.
 */
void sp_gatt_add_service(void);

uint8_t checksum(uint8_t *data, int length);

void headcheck(uint8_t *data,uint32_t len);

void ntf_data(uint8_t con_idx,uint8_t att_idx,uint8_t *data,uint16_t len);

#endif






