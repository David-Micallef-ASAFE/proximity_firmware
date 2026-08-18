/* Copyright 2026 ASAFE Ltd. All Rights Reserved.
 *
 * See file LICENSE.txt for full license details.
 */

/*
 * \file    as_storage.c
 * \brief   This file handles all the secure storage
 */

#include <stdlib.h>

#include "api.h"
#include "node_configuration.h"
#include "shared_data.h"
#include "app_scheduler.h"

#include "app_secure_storage.h"

#define DEBUG_LOG_MODULE_NAME "AS STORAGE"
#define DEBUG_LOG_MAX_LEVEL LVL_INFO

#include "debug_log.h"

#define APP_PERSISTENT_MEMORY_AREA_ID 0x8AE573BA // Default app_persistent memory area id
#define RECORD_SIZE 16

/** Test record structure */
typedef struct test_record
{
    const uint32_t tag;
    const uint8_t data[RECORD_SIZE];
    uint32_t flags;
} test_record_t;

/** Test record */
test_record_t test_record = {
    .tag = 0x12347856,
    .data = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16},
    .flags = 0};

void asStorageInit()
{
}
