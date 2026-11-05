/* Copyright 2026 ASAFE Ltd. All Rights Reserved.
 *
 * See file LICENSE.txt for full license details.
 */

/*
 * \file    as_cbor.c
 * \brief   Minimal CBOR (RFC 8949) reader/writer
 */

#include <string.h>

#include "as_cbor.h"

/******************************************************************************
 * Reader
 *****************************************************************************/

void cborReaderInit(cborReader_t *r, const uint8_t *buf, size_t len)
{
    r->buf = buf;
    r->len = len;
    r->pos = 0;
}

bool cborReaderAtEnd(const cborReader_t *r)
{
    return r->pos >= r->len;
}

/**
 * \brief   Reads the initial byte and argument of the next item
 * \return  false if the data is truncated or uses indefinite/reserved lengths
 */
bool cborReadHead(cborReader_t *r, uint8_t *major, uint8_t *ai, uint64_t *arg)
{
    if (r->pos >= r->len)
    {
        return false;
    }

    uint8_t initial = r->buf[r->pos++];
    *major = initial >> 5;
    *ai = initial & 0x1F;

    if (*ai < 24)
    {
        *arg = *ai;
        return true;
    }

    uint8_t extra;
    switch (*ai)
    {
    case 24:
        extra = 1;
        break;
    case 25:
        extra = 2;
        break;
    case 26:
        extra = 4;
        break;
    case 27:
        extra = 8;
        break;
    default:
        // 28..30 reserved, 31 indefinite length: neither is allowed
        return false;
    }

    if (r->len - r->pos < extra)
    {
        return false;
    }

    *arg = 0;
    for (uint8_t i = 0; i < extra; i++)
    {
        *arg = (*arg << 8) | r->buf[r->pos++];
    }
    return true;
}

static bool cborSkipDepth(cborReader_t *r, uint8_t depth)
{
    uint8_t major, ai;
    uint64_t arg;

    if (depth > CBOR_MAX_DEPTH || !cborReadHead(r, &major, &ai, &arg))
    {
        return false;
    }

    switch (major)
    {
    case CBOR_MAJOR_UINT:
    case CBOR_MAJOR_NINT:
        return true;
    case CBOR_MAJOR_BYTES:
    case CBOR_MAJOR_TEXT:
        if (arg > r->len - r->pos)
        {
            return false;
        }
        r->pos += (size_t)arg;
        return true;
    case CBOR_MAJOR_ARRAY:
    case CBOR_MAJOR_MAP:
    {
        // Every element needs at least one byte, so this also bounds the loop
        uint64_t items = (major == CBOR_MAJOR_MAP) ? arg * 2 : arg;
        if (arg > r->len || items > r->len - r->pos)
        {
            return false;
        }
        for (uint64_t i = 0; i < items; i++)
        {
            if (!cborSkipDepth(r, depth + 1))
            {
                return false;
            }
        }
        return true;
    }
    case CBOR_MAJOR_TAG:
        return cborSkipDepth(r, depth + 1);
    case CBOR_MAJOR_SIMPLE:
    default:
        // Simple values and floats: the head already consumed the payload
        return true;
    }
}

/**
 * \brief   Skips the next complete item, checking it is well formed
 */
bool cborSkip(cborReader_t *r)
{
    return cborSkipDepth(r, 0);
}

/**
 * \brief   Reads an integer (major type 0 or 1) that fits in an int64_t
 */
bool cborReadInt(cborReader_t *r, int64_t *value)
{
    uint8_t major, ai;
    uint64_t arg;

    if (!cborReadHead(r, &major, &ai, &arg) || arg > INT64_MAX)
    {
        return false;
    }

    if (major == CBOR_MAJOR_UINT)
    {
        *value = (int64_t)arg;
        return true;
    }
    if (major == CBOR_MAJOR_NINT)
    {
        *value = -1 - (int64_t)arg;
        return true;
    }
    return false;
}

/**
 * \brief   Reads the next item. Containers and tagged items are skipped entirely.
 * \return  false if the item is not well formed
 */
bool cborReadValue(cborReader_t *r, cborValue_t *value)
{
    size_t start = r->pos;
    uint8_t major, ai;
    uint64_t arg;

    memset(value, 0, sizeof(*value));

    if (!cborReadHead(r, &major, &ai, &arg))
    {
        return false;
    }
    value->arg = arg;

    switch (major)
    {
    case CBOR_MAJOR_UINT:
        value->type = CBOR_TYPE_UINT;
        return true;
    case CBOR_MAJOR_NINT:
        value->type = CBOR_TYPE_NINT;
        return true;
    case CBOR_MAJOR_BYTES:
    case CBOR_MAJOR_TEXT:
        if (arg > r->len - r->pos)
        {
            return false;
        }
        value->type = (major == CBOR_MAJOR_BYTES) ? CBOR_TYPE_BYTES : CBOR_TYPE_TEXT;
        value->bytes = &r->buf[r->pos];
        r->pos += (size_t)arg;
        return true;
    case CBOR_MAJOR_ARRAY:
    case CBOR_MAJOR_MAP:
    case CBOR_MAJOR_TAG:
        value->type = (major == CBOR_MAJOR_ARRAY) ? CBOR_TYPE_ARRAY : (major == CBOR_MAJOR_MAP) ? CBOR_TYPE_MAP
                                                                                                : CBOR_TYPE_OTHER;
        // Rewind and skip the whole item to validate its content
        r->pos = start;
        return cborSkip(r);
    case CBOR_MAJOR_SIMPLE:
    default:
        if (ai < 24 && arg == CBOR_SIMPLE_FALSE)
        {
            value->type = CBOR_TYPE_BOOL;
            value->boolean = false;
        }
        else if (ai < 24 && arg == CBOR_SIMPLE_TRUE)
        {
            value->type = CBOR_TYPE_BOOL;
            value->boolean = true;
        }
        else if (ai < 24 && arg == CBOR_SIMPLE_NULL)
        {
            value->type = CBOR_TYPE_NULL;
        }
        else
        {
            value->type = CBOR_TYPE_OTHER;
        }
        return true;
    }
}

/******************************************************************************
 * Writer
 *****************************************************************************/

void cborWriterInit(cborWriter_t *w, uint8_t *buf, size_t size)
{
    w->buf = buf;
    w->size = size;
    w->pos = 0;
    w->overflow = false;
}

void cborPutRaw(cborWriter_t *w, const uint8_t *bytes, size_t len)
{
    if (w->overflow || len > w->size - w->pos)
    {
        w->overflow = true;
        return;
    }
    memcpy(&w->buf[w->pos], bytes, len);
    w->pos += len;
}

/**
 * \brief   Writes an item head using the shortest form (deterministic encoding)
 */
static void cborPutHead(cborWriter_t *w, uint8_t major, uint64_t arg)
{
    uint8_t head[9];
    uint8_t len;

    if (arg < 24)
    {
        head[0] = (major << 5) | (uint8_t)arg;
        len = 1;
    }
    else if (arg <= 0xFF)
    {
        head[0] = (major << 5) | 24;
        len = 2;
    }
    else if (arg <= 0xFFFF)
    {
        head[0] = (major << 5) | 25;
        len = 3;
    }
    else if (arg <= 0xFFFFFFFF)
    {
        head[0] = (major << 5) | 26;
        len = 5;
    }
    else
    {
        head[0] = (major << 5) | 27;
        len = 9;
    }

    for (uint8_t i = len - 1; i > 0; i--)
    {
        head[i] = (uint8_t)arg;
        arg >>= 8;
    }

    cborPutRaw(w, head, len);
}

void cborPutUint(cborWriter_t *w, uint64_t value)
{
    cborPutHead(w, CBOR_MAJOR_UINT, value);
}

void cborPutInt(cborWriter_t *w, int64_t value)
{
    if (value >= 0)
    {
        cborPutHead(w, CBOR_MAJOR_UINT, (uint64_t)value);
    }
    else
    {
        cborPutHead(w, CBOR_MAJOR_NINT, (uint64_t)(-1 - value));
    }
}

void cborPutBool(cborWriter_t *w, bool value)
{
    cborPutHead(w, CBOR_MAJOR_SIMPLE, value ? CBOR_SIMPLE_TRUE : CBOR_SIMPLE_FALSE);
}

void cborPutNull(cborWriter_t *w)
{
    cborPutHead(w, CBOR_MAJOR_SIMPLE, CBOR_SIMPLE_NULL);
}

void cborPutBytes(cborWriter_t *w, const uint8_t *bytes, size_t len)
{
    cborPutHead(w, CBOR_MAJOR_BYTES, len);
    cborPutRaw(w, bytes, len);
}

void cborPutArrayHeader(cborWriter_t *w, uint64_t count)
{
    cborPutHead(w, CBOR_MAJOR_ARRAY, count);
}

void cborPutMapHeader(cborWriter_t *w, uint64_t count)
{
    cborPutHead(w, CBOR_MAJOR_MAP, count);
}
