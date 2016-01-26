/*
 * umsgpack.c: MessagePack for MCUs
 *
 *  Created on: Jun 21, 2015
 *      Author: hasegaw
 *
 * Supported encodings:
 *   0x00-0x7f: positive fixint
 *   0x80-0x8f: fixmap
 *   0x90-0x9f: fixarray
 *   0xa0-0xbf: fixstr
 *   0xc2-0xc3: boolean
 *   0xcc-0xcd: uint8/16
 *   0xd9-0xda: string
 *   0xdc-0xdd: array8/16
 *   0xde:      map16
 *   0xe0-0xff: negative fixint
 *
 * Unsupported encodings:
 *   0xc0: nil
 */

#include "stdlib.h"
#include "string.h"
#include "stdint.h"
#include "umsgpack.h"

/*
 * Little Endian
 */
#ifdef UMSGPACK_LITTLE_ENDIAN
static inline unsigned short _bswap_16(unsigned short x) {
    return (x << 8 ) | (x >> 8);
}

static inline unsigned int _bswap_32(unsigned int x) {
    return (_bswap_16(x&0xffff) << 16) | (_bswap_16(x >> 16));
}

#if 0
static inline void _store_be16(void *dest, unsigned short *val) {
    *((unsigned short*)dest) = _bswap_16(*val);
}
#endif

static inline void _store_be32(void *dest, unsigned long *val) {
    *((unsigned long*)dest) = _bswap_32(*val);
}
#endif

/*
 * Big Endian
 */
#ifdef UMSGPACK_BIG_ENDIAN
static inline unsigned short _bswap_16(unsigned short x) {
    return x;
}

static inline unsigned int _bswap_32(unsigned int x) {
    return x;
}

#if 0
static inline void _store_be16(void *dest, unsigned short val) {
    *((unsigned short*)dest) = val;
}
#endif

static inline void _store_be32(void *dest, void *val) {
    unsigned long *d = (unsigned long *) dest;
    *d = *((unsigned long *) val);
}
#endif

/**
 * @param[in] buf    Destination buffer
 * @param[in] length Number of objects in the array
 */
int umsgpack_pack_array(struct umsgpack_packer_buf *buf, int length) {
    int bytes;
    bytes = length > 0x0f ? 3: 1;

    if (buf->pos + bytes > buf->length)
        return 0;

    switch (bytes) {
    case 1:
        buf->data[buf->pos++] = 0x90 | (length & 0x0f);
        break;

    case 3:
        buf->data[buf->pos++] = 0xde;
        buf->data[buf->pos++] = (length >> 8) & 0xff;
        buf->data[buf->pos++] = length & 0xff;
    }
    return 1;
}

/* 16 bit integer */

/**
 * @param[in] buf    Destination buffer
 * @param[in] val    Value to be packed
 */
int umsgpack_pack_uint16(struct umsgpack_packer_buf *buf, uint16_t val) {
    int bytes;

    bytes = (val <= 0x7f) ? 1:
            (val <= 0xff) ? 2: 3;

    if (buf->pos + bytes > buf->length) {
        return 0;
    }

    switch (bytes) {
    case 1:
        buf->data[buf->pos++] = val & ~0x80;
        break;

    case 2:
        buf->data[buf->pos++] = 0xcc;
        buf->data[buf->pos++] = val;
        break;

    case 3:
        buf->data[buf->pos++] = 0xcd;
        buf->data[buf->pos++] = (val >> 8) & 0xff;
        buf->data[buf->pos++] = val & 0xff;
    }
    return 1;
}

int umsgpack_pack_int16(struct umsgpack_packer_buf *buf, int16_t val) {
    int bytes;
    unsigned char *p;

    if (val >= 0)
        return umsgpack_pack_uint16(buf, (uint16_t) val);

    /* handle negative values */

    bytes = (val >= -32) ? 1 :
            (val >= -128) ? 2 : 3;

    if (buf->pos + bytes > buf->length)
        return 0;

    switch (bytes) {
    case 1:
        buf->data[buf->pos++] = val & 0xFF;
        break;

    case 2:
        buf->data[buf->pos++] = 0xd0;
        buf->data[buf->pos++] = val & 0xFF;
        break;

    case 3:
        /* fixme: assume big endian */
        p = (void*) &val;

        buf->data[buf->pos++] = 0xd1;
        buf->data[buf->pos++] = *p;
        buf->data[buf->pos++] = *(p + 1);
    }
    return 1;
}

/* 32 bit integer */

#ifdef UMSGPACK_FUNC_INT32
int umsgpack_pack_uint32(struct umsgpack_packer_buf *buf, uint32_t val) {
    const int bytes = 5;

    if (val < 0x10000)
        return umsgpack_pack_int16(buf, val);

    if (buf->pos + bytes > buf->length)
        return 0;

    buf->data[buf->pos++] = 0xce;
    buf->data[buf->pos++] = (val >> 24) & 0xff;
    buf->data[buf->pos++] = (val >> 16) & 0xff;
    buf->data[buf->pos++] = (val >> 8) & 0xff;
    buf->data[buf->pos++] = val & 0xff;
    return 1;
}

int umsgpack_pack_int32(struct umsgpack_packer_buf *buf, int32_t val) {
    int bytes = 5;
    unsigned char *p;

    if (val >= 0)
        return umsgpack_pack_uint32(buf, (uint32_t) val);

    if (val >= (int32_t) -32768)
        return umsgpack_pack_int16(buf, (int16_t) val);

    if (buf->pos + bytes > buf->length)
        return 0;

    /* fixme: assume big endian */
    p = (void*) &val;

    buf->data[buf->pos++] = 0xd2;
    buf->data[buf->pos++] = *p;
    buf->data[buf->pos++] = *(p + 1);
    buf->data[buf->pos++] = *(p + 2);
    buf->data[buf->pos++] = *(p + 3);
    return 1;
}
#endif

/* 64 bit integer */

#ifdef UMSGPACK_FUNC_INT64

int umsgpack_pack_uint64(struct umsgpack_packer_buf *buf, uint64_t val) {
    const int bytes = 9;

    if (val <= (uint64_t) 0xFFFF)
        return umsgpack_pack_uint16(buf, (uint16_t) val);
    if (val <= (uint64_t) 0xFFFFFFFF)
        return umsgpack_pack_uint32(buf, (uint32_t) val);

    if (buf->pos + bytes > buf->length)
        return 0;

    /* fixme: assue big endian */
    buf->data[buf->pos++] = 0xcf;
    buf->data[buf->pos++] = (val >> 56) & 0xff;
    buf->data[buf->pos++] = (val >> 48) & 0xff;
    buf->data[buf->pos++] = (val >> 40) & 0xff;
    buf->data[buf->pos++] = (val >> 32) & 0xff;
    buf->data[buf->pos++] = (val >> 24) & 0xff;
    buf->data[buf->pos++] = (val >> 16) & 0xff;
    buf->data[buf->pos++] = (val >> 8) & 0xff;
    buf->data[buf->pos++] = val & 0xff;
    return 1;
}

int umsgpack_pack_int64(struct umsgpack_packer_buf *buf, int64_t val) {
    int bytes = 9;
    unsigned char *p;

    if (val >= 0)
        return umsgpack_pack_uint64(buf, (uint64_t) val);

#ifdef UMSGPACK_HW_NEGATIVE_INT64
    /* Some compilers doesn't support negative int64_t values. */
    if (val >= (int64_t) -2147483648)
        return umsgpack_pack_int32(buf, (int32_t) val);
#endif

    if (buf->pos + bytes > buf->length)
        return 0;

    /* fixme: assume big endian */
    p = (void*) &val;

    buf->data[buf->pos++] = 0xd3;
    buf->data[buf->pos++] = *p;
    buf->data[buf->pos++] = *(p + 1);
    buf->data[buf->pos++] = *(p + 2);
    buf->data[buf->pos++] = *(p + 3);
    buf->data[buf->pos++] = *(p + 4);
    buf->data[buf->pos++] = *(p + 5);
    buf->data[buf->pos++] = *(p + 6);
    buf->data[buf->pos++] = *(p + 7);
    return 1;
}

#endif

int umsgpack_pack_int(struct umsgpack_packer_buf *buf, int val) {
#ifdef UMSGPACK_INT_WIDTH_16
    return umsgpack_pack_int16(buf, val);
#elif UMSGPACK_INT_WIDTH_32
    return umsgpack_pack_int32(buf, val);
#else
#error unknown word length
#endif
}

int umsgpack_pack_uint(struct umsgpack_packer_buf *buf, unsigned int val) {
#ifdef UMSGPACK_INT_WIDTH_16
    return umsgpack_pack_uint16(buf, val);
#elif UMSGPACK_INT_WIDTH_32
    return umsgpack_pack_uint32(buf, val);
#else
#error unknown word length
#endif
}

/**
 * @param[in] buf    Destination buffer
 * @param[in] val    Value to be packed
 */
int umsgpack_pack_float(struct umsgpack_packer_buf *buf, float val) {
    int bytes = 5;

    if (buf->pos + bytes > buf->length) {
        return 0;
    }
    
#if UMSGPACK_HW_FLOAT_IEEE754COMPLIANT && UMSGPACK_HW_BIG_ENDIAN
    unsigned char *f = (void*) &val;
    buf->data[buf->pos++] = 0xca;
    buf->data[buf->pos++] = *(f + 0);
    buf->data[buf->pos++] = *(f + 1);
    buf->data[buf->pos++] = *(f + 2);
    buf->data[buf->pos++] = *(f + 3);
#if 0
    long *f = &val;
    buf->data[buf->pos++] = 0xca;
    buf->data[buf->pos++] = *f >> 24;
    buf->data[buf->pos++] = *f >> 16;
    buf->data[buf->pos++] = *f >> 8;
    buf->data[buf->pos++] = *f;
    return 1;
#endif
#endif
#if UMSGPACK_HW_FLOAT_IEEE754COMPLIANT && UMSGPACK_HW_LITTLE_ENDIAN
    unsigned char *f = (void*)&val;
    buf->data[buf->pos++] = 0xca;
    buf->data[buf->pos++] = *(f + 3);
    buf->data[buf->pos++] = *(f + 2);
    buf->data[buf->pos++] = *(f + 1);
    buf->data[buf->pos++] = *(f + 0);
    return 1;
#endif
    return 0;
}

/**
 * @param[in] buf    Destination buffer
 * @param[in] val    Value to be packed
 */
#if 0
int umsgpack_pack_double(struct umsgpack_packer_buf *buf, double val) {
    int bytes = 9;

    if (buf->pos + bytes > buf->length) {
        return 0;
    }

    buf->data[buf->pos++] = 0xcb;
    /* FIXME */
    *((unsigned long*) &buf->data[buf->pos]) = _bswap_32(*((unsigned long*)&val));
    buf->pos += 8;
    return 0;
    return 1;
}
#endif

/**
 * @param[in] buf         Destination buffer
 * @param[in] num_objects Number of objects (key-value pairs) in the map
 */
int umsgpack_pack_map(struct umsgpack_packer_buf *buf, int num_objects) {
    int bytes;
    bytes = num_objects <= 0x0f ? 1:
            num_objects <= 0xFFFF ? 2:
            num_objects <= 0xFFFF ? 3: 0;

    if (buf->pos + bytes > buf->length)
        return 0;

    switch (bytes) {
    case 1:
        buf->data[buf->pos++] = 0x80 | (num_objects & 0x0f);
        break;

    case 2:
        buf->data[buf->pos++] = 0xde;
        buf->data[buf->pos++] = num_objects & 0xff;
        break;

    case 3:
        buf->data[buf->pos++] = 0xdf;
        buf->data[buf->pos++] = (num_objects >> 8) & 0xff;
        buf->data[buf->pos++] = num_objects & 0xff;
        break;
    default:
        return 0;
    }

    return 1;
}

/**
 * @param[in] buf    Destination buffer
 * @param[in] s      Pointer to the string to be packed
 * @param[in] length Length of the string
 *
 * If s is NULL, the function won't copy the string into the buffer.
 */
int umsgpack_pack_str(struct umsgpack_packer_buf *buf, char* s, int length) {
    int bytes;
    bytes = length <= 31 ? 1:
            length <= 0xFF ? 2:
            length <= 0xFFFF ? 3: 0;

    if (buf->pos + bytes + length > buf->length)
        return 0;

    switch (bytes) {
    case 1:
        buf->data[buf->pos++] = 0xa0 | (length & 0x1f);
        break;

    case 2:
        buf->data[buf->pos++] = 0xd9;
        buf->data[buf->pos++] = length;
        break;

    case 4:
        buf->data[buf->pos++] = 0x0da;
        buf->data[buf->pos++] = (length >> 8) & 0xff;
        buf->data[buf->pos++] = length & 0xff;
        break;
    default:
        return 0;
    }
    if (s) {
        memcpy(&buf->data[buf->pos], (void*)s, length);
        buf->pos += length;
    }
    return 1;
}

/**
 * @param[in] buf    Destination buffer
 * @param[in] val    Boolean value (0 == FALSE, Otherwise TRUE)
 */
int umsgpack_pack_bool(struct umsgpack_packer_buf *buf, int val) {
    int bytes = 1;

    if (buf->pos + bytes > buf->length)
        return 0;

    buf->data[buf->pos++] = val ? 0xc3: 0xc2;
    return 1;
}

void umsgpack_packer_init(struct umsgpack_packer_buf *buf, int size) {
     if (buf) {
        buf->length = size - sizeof(struct umsgpack_packer_buf);
        buf->pos = 0;
    }
}

/**
 * @param[in] size   Size of the buffer to be allocated
 *
 * In messagepack-lite, the caller is responsible for estimating
 * the buffer size needed.
 */
struct umsgpack_packer_buf *umsgpack_alloc(int size) {
    struct umsgpack_packer_buf *buf = malloc(size + sizeof(struct umsgpack_packer_buf));
    if (buf) {
        buf->length = size;
        buf->pos = 0;
    }
    return buf;
}

/**
 * @param[in] buf    Destination buffer to be freed
 */
int umsgpack_free(struct umsgpack_packer_buf *buf) {
    free(buf);
    return 1;
}
