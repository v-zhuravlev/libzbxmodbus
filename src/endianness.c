#include <byteswap.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "endianness.h"

typedef struct
{
	const endianness_code_t endianness_code;
	const char *		endianness_str;
} endianness_token_t;

/* clang-format off */
endianness_token_t	endianness_tokens[] =
{
	{LIBZBXMODBUS_MLE_CDAB,	"MLE"},
	{LIBZBXMODBUS_BE_ABCD,	"BE"},
	{LIBZBXMODBUS_MBE_BADC,	"MBE"},
	{LIBZBXMODBUS_LE_DCBA,	"LE"}
};
/* clang-format on */

/* endianness LE(0) BE(1) MBE(2) MLE(3) default BE */
int parse_endianness(const char *endianness_string, endianness_code_t *endianness, char **error)
{
	size_t   i = sizeof(endianness_tokens) / sizeof(endianness_token_t);
	char *   endptr;
	long int endianness_value;

	if (NULL == endianness_string || '\0' == *endianness_string)
	{
		*endianness = LIBZBXMODBUS_BE_ABCD;
		return 0;
	}

	while (0 < i--)
	{
		if (0 == strcmp(endianness_string, endianness_tokens[i].endianness_str))
		{
			*endianness = endianness_tokens[i].endianness_code;
			return 0;
		}
	}

	errno = 0;
	endianness_value = strtol(endianness_string, &endptr, 0);

	if (0 == errno && '\0' == *endptr)
	{
		switch (endianness_value)
		{
			case LIBZBXMODBUS_MLE_CDAB:
			case LIBZBXMODBUS_BE_ABCD:
			case LIBZBXMODBUS_MBE_BADC:
			case LIBZBXMODBUS_LE_DCBA:
				*endianness = endianness_value;
				return 0;
		}
	}

	*error = strdup("Check endiannes used: BE,LE,MLE,MBE.");
	return -1;
}

#define MODBUS_GET_BE_32BIT(tab_int16, index) (((uint32_t)tab_int16[(index)]) << 16) | tab_int16[(index) + 1]
#define MODBUS_GET_MLE_32BIT(tab_int16, index) (((uint32_t)tab_int16[(index) + 1]) << 16) | tab_int16[(index)]
#define MODBUS_GET_MBE_32BIT(tab_int16, index) \
	(((uint32_t)bswap_16(tab_int16[(index)])) << 16) | bswap_16(tab_int16[(index) + 1])
#define MODBUS_GET_LE_32BIT(tab_int16, index) \
	(((uint32_t)bswap_16(tab_int16[(index) + 1])) << 16) | bswap_16(tab_int16[(index)])

#define MODBUS_GET_BE_64BIT(tab_int16, index)                                             \
	(((uint64_t)tab_int16[index]) << 48) | (((uint64_t)tab_int16[index + 1]) << 32) | \
		(((uint64_t)tab_int16[index + 2]) << 16) | tab_int16[index + 3]
#define MODBUS_GET_MLE_64BIT(tab_int16, index)                                                \
	(((uint64_t)tab_int16[index + 3]) << 48) | (((uint64_t)tab_int16[index + 2]) << 32) | \
		(((uint64_t)tab_int16[index + 1]) << 16) | tab_int16[index]
#define MODBUS_GET_MBE_64BIT(tab_int16, index)                                                                \
	(((uint64_t)bswap_16(tab_int16[index])) << 48) | (((uint64_t)bswap_16(tab_int16[index + 1])) << 32) | \
		(((uint64_t)bswap_16(tab_int16[index + 2])) << 16) | bswap_16(tab_int16[index + 3])
#define MODBUS_GET_LE_64BIT(tab_int16, index)                                                                     \
	(((uint64_t)bswap_16(tab_int16[index + 3])) << 48) | (((uint64_t)bswap_16(tab_int16[index + 2])) << 32) | \
		(((uint64_t)bswap_16(tab_int16[index + 1])) << 16) | bswap_16(tab_int16[index])

#define EXTRACT_DEFINITION(number_of_bits)                                                 \
	uint##number_of_bits##_t extract##number_of_bits##bits(                            \
		const uint16_t *registers, endianness_code_t endianness)                   \
	{                                                                                  \
		switch (endianness)                                                        \
		{                                                                          \
			case LIBZBXMODBUS_LE_DCBA:                                         \
				return MODBUS_GET_LE_##number_of_bits##BIT(registers, 0);  \
			case LIBZBXMODBUS_BE_ABCD:                                         \
				return MODBUS_GET_BE_##number_of_bits##BIT(registers, 0);  \
			case LIBZBXMODBUS_MBE_BADC:                                        \
				return MODBUS_GET_MBE_##number_of_bits##BIT(registers, 0); \
			case LIBZBXMODBUS_MLE_CDAB:                                        \
				return MODBUS_GET_MLE_##number_of_bits##BIT(registers, 0); \
		}                                                                          \
	}

EXTRACT_DEFINITION(32) /* uint32_t extract32bits() */
EXTRACT_DEFINITION(64) /* uint64_t extract64bits() */

#undef EXTRACT_DEFINITION
