/* Copyright 2026 ASAFE. All Rights Reserved.
 *
 * See file LICENSE.txt for full license details.
 *
 */

/**
 * \file as_cbor.h
 * \brief Minimal CBOR (RFC 8949) reader/writer.
 *
 * Supports only what the device <-> cloud protocol needs: definite-length
 * items, integers, byte/text strings, arrays, maps, tags and simple values.
 * Indefinite-length items are rejected as malformed.
 */

#ifndef __AS_CBOR_H_
#define __AS_CBOR_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* CBOR major types */
#define CBOR_MAJOR_UINT 0
#define CBOR_MAJOR_NINT 1
#define CBOR_MAJOR_BYTES 2
#define CBOR_MAJOR_TEXT 3
#define CBOR_MAJOR_ARRAY 4
#define CBOR_MAJOR_MAP 5
#define CBOR_MAJOR_TAG 6
#define CBOR_MAJOR_SIMPLE 7

/* CBOR simple values (major type 7) */
#define CBOR_SIMPLE_FALSE 20
#define CBOR_SIMPLE_TRUE 21
#define CBOR_SIMPLE_NULL 22

/* Maximum nesting depth accepted when skipping items */
#define CBOR_MAX_DEPTH 4

/* Decoded value types */
typedef enum
{
    CBOR_TYPE_UINT,
    CBOR_TYPE_NINT, /* Negative integer, value = -1 - arg */
    CBOR_TYPE_BYTES,
    CBOR_TYPE_TEXT,
    CBOR_TYPE_ARRAY,
    CBOR_TYPE_MAP,
    CBOR_TYPE_BOOL,
    CBOR_TYPE_NULL,
    CBOR_TYPE_OTHER /* Tags, floats, undefined, other simple values */
} cborType_e;

/* A decoded CBOR value. Containers are skipped, only their element count is kept. */
typedef struct
{
    cborType_e type;
    uint64_t arg;         /* UINT/NINT raw argument, string length, container count */
    bool boolean;         /* Valid when type == CBOR_TYPE_BOOL */
    const uint8_t *bytes; /* Valid when type == CBOR_TYPE_BYTES/TEXT, points into the input buffer */
} cborValue_t;

typedef struct
{
    const uint8_t *buf;
    size_t len;
    size_t pos;
} cborReader_t;

typedef struct
{
    uint8_t *buf;
    size_t size;
    size_t pos;
    bool overflow;
} cborWriter_t;

/* Reader */
void cborReaderInit(cborReader_t *r, const uint8_t *buf, size_t len);
bool cborReadHead(cborReader_t *r, uint8_t *major, uint8_t *ai, uint64_t *arg);
bool cborReadInt(cborReader_t *r, int64_t *value);
bool cborReadValue(cborReader_t *r, cborValue_t *value);
bool cborSkip(cborReader_t *r);
bool cborReaderAtEnd(const cborReader_t *r);

/* Writer */
void cborWriterInit(cborWriter_t *w, uint8_t *buf, size_t size);
void cborPutUint(cborWriter_t *w, uint64_t value);
void cborPutInt(cborWriter_t *w, int64_t value);
void cborPutBool(cborWriter_t *w, bool value);
void cborPutNull(cborWriter_t *w);
void cborPutBytes(cborWriter_t *w, const uint8_t *bytes, size_t len);
void cborPutArrayHeader(cborWriter_t *w, uint64_t count);
void cborPutMapHeader(cborWriter_t *w, uint64_t count);
void cborPutRaw(cborWriter_t *w, const uint8_t *bytes, size_t len);

#endif /* __AS_CBOR_H_ */
