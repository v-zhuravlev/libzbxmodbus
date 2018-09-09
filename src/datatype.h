#ifndef LIBZBXMODBUS_DATATYPE_H
#define LIBZBXMODBUS_DATATYPE_H

#include "sysinc.h"
#include "module.h"

typedef enum
{
	BIT_SYNTAX,
	REGISTER_SYNTAX
}
datatype_syntax_t;

int	parse_datatype(datatype_syntax_t syntax, const char *datatype, char **error);
void	set_result_based_on_datatype(AGENT_RESULT *result, const char *datatype, int start, const uint8_t *bits,
		size_t bits_size, const uint16_t *registers, size_t registers_size, int endianness);

#endif /* LIBZBXMODBUS_DATATYPE_H */
