#ifndef LIBZBXMODBUS_DATATYPE_H
#define LIBZBXMODBUS_DATATYPE_H

#include "sysinc.h"
#include "module.h"

typedef enum
{
	MODBUS_BIT,
	MODBUS_UINT16,
	MODBUS_SIGNED_INT,
	MODBUS_UINT32,
	MODBUS_SIGNED_INT32,
	MODBUS_FLOAT,
	MODBUS_SIGNED_INT64,
	MODBUS_UINT64,
	MODBUS_FLOAT64,
	MODBUS_SKIP
}
datatype_code_t;

typedef struct
{
	const char		*name;
	const char		*legacy_name;
	const datatype_code_t	type_code;
	const int		regs_to_read;
}
datatype_token_t;

extern const datatype_token_t	bit_syntax[];
extern const datatype_token_t	register_syntax[];

int	parse_datatype(const datatype_token_t *syntax, const char *datatype, char **error);
void	set_result_based_on_datatype(AGENT_RESULT *result, const char *datatype, int start, const uint8_t *bits,
		size_t bits_size, const uint16_t *registers, size_t registers_size, int endianness);

#endif /* LIBZBXMODBUS_DATATYPE_H */
