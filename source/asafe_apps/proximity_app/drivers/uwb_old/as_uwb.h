/* Copyright 2022 Redlore. All Rights Reserved.
 *
 * See file LICENSE.txt for full license details.
 *
 */

/**
 * \file as_uwb.h
 *
 */

#ifndef __AS_UWB_H_
#define __AS_UWB_H_

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "api.h"

#define MAX_UWB_NBORS 20

#define UWB_ADDRESS_BROADCAST 0XFFFFFFFF

typedef enum
{
    UWB_FUNCTION_CODE_DISCOVERY = 0X01,
    UWB_FUNCTION_CODE_RSS = 0X02,
    UWB_FUNCTION_CODE_RST = 0X03,
    UWB_FUNCTION_CODE_RSE = 0X04,
    UWB_FUNCTION_CODE_NBOR_DISCOVERY = 0XFF // This is not actually used but its simply there to indicated the current function
} uwbFunction_e;

typedef struct
{
    bool enableRxAfterTx;
    bool sleepAfterTx;
    bool sleepActivated;
    uint8_t sequence;
    uwbFunction_e currentFunction;
} uwbControl_t;

void asUWBInit();
void uwbWakeUpSt1();
void uwbWakeUpSt2();
void uwbSleep();
void uwbEnableRX();
void uwbDisableRX();

/*V2 Updates*/
// void uwbTransmitMsg(uint8_t function, uint8_t sequence, uint32_t dstAddress);
void uwbTransmitMsg(uint8_t function, uint8_t sequence, uint32_t dstAddress, uint32_t nextRS);

#endif /* __RL_UWB_H_ */