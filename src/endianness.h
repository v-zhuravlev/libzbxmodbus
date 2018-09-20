#ifndef LIBZBXMODBUS_ENDIANNESS_H
#define LIBZBXMODBUS_ENDIANNESS_H

#include <stdint.h>

typedef enum {
	LIBZBXMODBUS_MLE_CDAB = 0, /* Mid-Little Endian (CDAB) */
	LIBZBXMODBUS_BE_ABCD = 1,  /* Big Endian (ABCD) */
	LIBZBXMODBUS_MBE_BADC = 2, /* Mid-Big Endian (BADC) */
	LIBZBXMODBUS_LE_DCBA = 3   /* Little Endian (DCBA) */
} endianness_code_t;

int parse_endianness(const char *endianness_string, endianness_code_t *endianness, char **error);

#define EXTRACT_DECLARATION(number_of_bits) \
	uint##number_of_bits##_t extract##number_of_bits##bits(const uint16_t *registers, endianness_code_t endianness);

EXTRACT_DECLARATION(32) /* uint32_t extract32bits() */
EXTRACT_DECLARATION(64) /* uint64_t extract64bits() */

#undef EXTRACT_DECLARATION

#endif /* LIBZBXMODBUS_ENDIANNESS_H */
