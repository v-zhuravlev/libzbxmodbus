#ifndef LIBZBXMODBUS_DATATYPE_H
#define LIBZBXMODBUS_DATATYPE_H

#include "sysinc.h"

#include "module.h"

#include "endianness.h"

typedef enum { BIT_SYNTAX, REGISTER_SYNTAX } datatype_syntax_t;

typedef struct datatype_parse_s datatype_parse_t;

int  parse_datatype(datatype_syntax_t syntax, const char *datatype, datatype_parse_t **layout, char **error);
void set_result_based_on_datatype(AGENT_RESULT *result, const datatype_parse_t *layout, int start, const uint8_t *bits,
	const uint16_t *registers, endianness_code_t endianness);

#endif /* LIBZBXMODBUS_DATATYPE_H */
