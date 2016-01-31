#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include "umsgpack.h"
#include "minunit/minunit.h"

#define FORMAT_MAX_SIZE 9

struct umsgpack_packer_buf *m_pack = NULL;

const static char m_char_patterns[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ~!@#$%^&*()-_=+{}[]{}|;:',.<>/?";
const static size_t m_char_pattern_size = sizeof(m_char_patterns);

struct {
	const char *pattern;
	const size_t size;
} str_test_data = {
	.pattern = m_char_patterns,
	.size = m_char_pattern_size,
};


void test_setup() {
}

void test_teardown() {
	umsgpack_free(m_pack);
	m_pack = NULL;
}

MU_TEST(test_positive_fixint) {
	/* 0x00 - 0x7f */
	const size_t data_size = FORMAT_MAX_SIZE;
	m_pack = umsgpack_alloc(data_size);
	if (!m_pack) {
		fprintf(stderr, "%s: failed umsgpack_alloc(%lu). skip test.\n", __func__, data_size);
		return;
	}

	for (uint8_t expects = 0; expects < 0x7F; expects++) {
		umsgpack_pack_uint(m_pack, expects);
		// length
		mu_assert_int_eq(1, m_pack->pos);
		// format
		mu_assert_int_eq(expects, m_pack->data[0]);
		m_pack->pos = 0;
	}
}

MU_TEST(test_fixmap) {
	/* 0x80 - 0x8f */
	const size_t data_size = FORMAT_MAX_SIZE;
	m_pack = umsgpack_alloc(data_size);
	if (!m_pack) {
		fprintf(stderr, "%s: failed umsgpack_alloc(%lu). skip test.\n", __func__, data_size);
		return;
	}

	for (int8_t i = 0; i < 0x0F; i++) {
		uint8_t expects = 0x80 | i;
		umsgpack_pack_map(m_pack, i);
		// length
		mu_assert_int_eq(1, m_pack->pos);
		// format
		mu_assert_int_eq(expects, m_pack->data[0]);
		m_pack->pos = 0;
	}
}

MU_TEST(test_fixarray) {
	/* 0x90 - 0x9f (b1001XXXX) */
	const size_t data_size = FORMAT_MAX_SIZE;
	m_pack = umsgpack_alloc(data_size);
	if (!m_pack) {
		fprintf(stderr, "%s: failed umsgpack_alloc(%lu). skip test.\n", __func__, data_size);
		return;
	}

	for (int8_t i = 0; i < 0x0F; i++) {
		uint8_t expects = 0x90 | i;
		umsgpack_pack_array(m_pack, i);
		// length
		mu_assert_int_eq(1, m_pack->pos);
		// format
		mu_assert_int_eq(expects, m_pack->data[0]);
		m_pack->pos = 0;
	}
}

MU_TEST(test_fixstr) {
	/* 0xa0 - 0xbf (b101XXXXX) 0~31bytes */
	const size_t str_length = 31;
	const size_t data_size = FORMAT_MAX_SIZE + str_length;
	m_pack = umsgpack_alloc(data_size);
	if (!m_pack) {
		fprintf(stderr, "%s: failed umsgpack_alloc(%lu). skip test.\n", __func__, data_size);
		return;
	}

	for (int8_t i = 0; i <= (int8_t)str_length; i++) {
		uint8_t expects = 0xA0 | i;
		umsgpack_pack_str(m_pack, (char*)str_test_data.pattern, i);
		// length
		mu_assert_int_eq(i+1, m_pack->pos);
		// format
		mu_assert_int_eq(expects, m_pack->data[0]);
		// string
		for (int8_t j = 0; j < i; j++) {
			mu_assert_int_eq(str_test_data.pattern[j], m_pack->data[j+1]);
		}
		m_pack->pos = 0;
	}
}

MU_TEST(test_nil) {
	/* 0xc0 */
}

MU_TEST(test_false) {
	/* 0xc2 */
	const size_t data_size = FORMAT_MAX_SIZE;
	m_pack = umsgpack_alloc(data_size);
	if (!m_pack) {
		fprintf(stderr, "%s: failed umsgpack_alloc(%lu). skip test.\n", __func__, data_size);
		return;
	}

	uint8_t expects = 0xc2;
	umsgpack_pack_bool(m_pack, 0);
	// length
	mu_assert_int_eq(1, m_pack->pos);
	// format
	mu_assert_int_eq(expects, (uint8_t)m_pack->data[0]);
}

MU_TEST(test_true) {
	/* 0xc3 */
	const size_t data_size = FORMAT_MAX_SIZE;
	m_pack = umsgpack_alloc(data_size);
	if (!m_pack) {
		fprintf(stderr, "%s: failed umsgpack_alloc(%lu). skip test.\n", __func__, data_size);
		return;
	}

	uint8_t expects = 0xc3;
	umsgpack_pack_bool(m_pack, 1);
	// length
	mu_assert_int_eq(1, m_pack->pos);
	// format
	mu_assert_int_eq(expects, (uint8_t)m_pack->data[0]);
}

MU_TEST(test_bin8) {
	/* 0xc4 + uint8-length + data... */
}

MU_TEST(test_bin16) {
	/* 0xc5 + uint16-length + data... */
}

MU_TEST(test_bin32) {
	/* 0xc6 + uint32-length + data... */
}

MU_TEST(test_ext8) {
	/* 0xc7 + uint8-length + 8bit-type + data... */
}

MU_TEST(test_ext16) {
	/* 0xc8 + uint16-length + 8bit-type + data... */
}

MU_TEST(test_ext32) {
	/* 0xc9 + uint32-length + 8bit-type + data... */
}

MU_TEST(test_float32) {
	/* 0xca + float32-value[BigEndian,IEEE754] */
}

MU_TEST(test_float64) {
	/* 0xcb + float64-value[BigEndian,IEEE754] */
}

MU_TEST(test_uint8) {
	/* 0xcc + uint8-value */

}

MU_TEST(test_uint16) {
	/* 0xcd + uint16-value[BigEndian] */
}

MU_TEST(test_uint32) {
	/* 0xce + uint32-value[BigEndian] */
}

MU_TEST(test_uint64) {
	/* 0xcf + uint64-value[BigEndian] */
}

MU_TEST(test_int8) {
	/* 0xd0 + int8-value */
}

MU_TEST(test_int16) {
	/* 0xd1 + int16-value[BigEndian] */
}

MU_TEST(test_int32) {
	/* 0xd2 + int32-value[BigEndian] */
}

MU_TEST(test_int64) {
	/* 0xd3 + int64-value[BigEndian] */
}

MU_TEST(test_fixext1) {
	/* 0xd4 + 8bit-type + 8bit-data */
}

MU_TEST(test_fixext2) {
	/* 0xd5 + 8bit-type + 16bit-data */
}

MU_TEST(test_fixext4) {
	/* 0xd6 + 8bit-type + 32bit-data */
}

MU_TEST(test_fixext8) {
	/* 0xd7 + 8bit-type + 64bit-data */
}

MU_TEST(test_fixext16) {
	/* 0xd8 + 8bit-type + 128bit-data */
}

MU_TEST(test_str8) {
	/* 0xd9 + uint8-lenght + string... */
}

MU_TEST(test_str16) {
	/* 0xda + uint16-lenght[BigEndian] + string... */
}

MU_TEST(test_str32) {
	/* 0xdb + uint32-lenght[BigEndian] + string... */
}

MU_TEST(test_array16) {
	/* 0xdc + uint16-length[BigEndian] + {N objects} */
}

MU_TEST(test_array32) {
	/* 0xdd + uint32-lenght[BigEndian] + {N objects} */
}

MU_TEST(test_map16) {
	/* 0xde + uint16-length[BigEndian] + {N*2 objects} */
}

MU_TEST(test_map32) {
	/* 0xdf + uint32-length[BigEndian] + {N*2 objects} */
}

MU_TEST(test_negative_fixint) {
	/* 0xe0 - 0xff */
	const size_t data_size = FORMAT_MAX_SIZE;
	m_pack = umsgpack_alloc(data_size);
	if (!m_pack) {
		fprintf(stderr, "%s: failed umsgpack_alloc(%lu). skip test.\n", __func__, data_size);
		return;
	}

	for (int8_t expects = -32; expects < 0; expects++) {
		umsgpack_pack_int(m_pack, expects);
		// length
		mu_assert_int_eq(1, m_pack->pos);
		// format
		mu_assert_int_eq(expects, (int8_t)m_pack->data[0]);
		m_pack->pos = 0;
	}
}

MU_TEST_SUITE(test_suite) {
	MU_SUITE_CONFIGURE(&test_setup, &test_teardown);

	MU_RUN_TEST(test_positive_fixint);
	MU_RUN_TEST(test_fixmap);
	MU_RUN_TEST(test_fixarray);
	MU_RUN_TEST(test_fixstr);
	MU_RUN_TEST(test_nil);
	MU_RUN_TEST(test_false);
	MU_RUN_TEST(test_true);
	MU_RUN_TEST(test_bin8);
	MU_RUN_TEST(test_bin16);
	MU_RUN_TEST(test_bin32);
	MU_RUN_TEST(test_ext8);
	MU_RUN_TEST(test_ext16);
	MU_RUN_TEST(test_ext32);
	MU_RUN_TEST(test_float32);
	MU_RUN_TEST(test_float64);
	MU_RUN_TEST(test_uint8);
	MU_RUN_TEST(test_uint16);
	MU_RUN_TEST(test_uint32);
	MU_RUN_TEST(test_uint64);
	MU_RUN_TEST(test_int8);
	MU_RUN_TEST(test_int16);
	MU_RUN_TEST(test_int32);
	MU_RUN_TEST(test_int64);
	MU_RUN_TEST(test_fixext1);
	MU_RUN_TEST(test_fixext2);
	MU_RUN_TEST(test_fixext4);
	MU_RUN_TEST(test_fixext8);
	MU_RUN_TEST(test_fixext16);
	MU_RUN_TEST(test_str8);
	MU_RUN_TEST(test_str16);
	MU_RUN_TEST(test_str32);
	MU_RUN_TEST(test_array16);
	MU_RUN_TEST(test_array32);
	MU_RUN_TEST(test_map16);
	MU_RUN_TEST(test_map32);
	MU_RUN_TEST(test_negative_fixint);
}

int main(int argc, char *argv[]) {
    MU_RUN_SUITE(test_suite);
    MU_REPORT();
    return 0;
}
