#include <inttypes.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>

#include "datatype.h"

typedef enum {
	LIBZBXMODBUS_SKIP, /* register to read, but discard */
	LIBZBXMODBUS_BIT,
	LIBZBXMODBUS_UINT16,
	LIBZBXMODBUS_SIGNED_INT,
	LIBZBXMODBUS_UINT32,
	LIBZBXMODBUS_SIGNED_INT32,
	LIBZBXMODBUS_FLOAT,
	LIBZBXMODBUS_UINT64,
	LIBZBXMODBUS_SIGNED_INT64,
	LIBZBXMODBUS_FLOAT64,
	LIBZBXMODBUS_NONE /* special code for invalid type */
} datatype_code_t;

static int type_registers(datatype_code_t type_code)
{
	switch (type_code)
	{
		case LIBZBXMODBUS_SKIP:
		case LIBZBXMODBUS_BIT:
		case LIBZBXMODBUS_UINT16:
		case LIBZBXMODBUS_SIGNED_INT:
			return 1;
		case LIBZBXMODBUS_UINT32:
		case LIBZBXMODBUS_SIGNED_INT32:
		case LIBZBXMODBUS_FLOAT:
			return 2;
		case LIBZBXMODBUS_UINT64:
		case LIBZBXMODBUS_SIGNED_INT64:
		case LIBZBXMODBUS_FLOAT64:
			return 4;
		case LIBZBXMODBUS_NONE:
		default:
			return 0;
	}
}

typedef struct
{
	const datatype_code_t type_code;
	const char *	  name;
	const char *	  legacy_name;
} datatype_token_t;

/* clang-format off */
const datatype_token_t bit_syntax[] =
{
	{LIBZBXMODBUS_BIT,		"bit",		"b"},
	{LIBZBXMODBUS_SKIP,		"skip",		NULL},
	{LIBZBXMODBUS_NONE,		NULL,		NULL}
};

const datatype_token_t register_syntax[] =
{
	{LIBZBXMODBUS_UINT16,		"uint16",	"i"},
	{LIBZBXMODBUS_SIGNED_INT,	"int16",	"s"},
	{LIBZBXMODBUS_UINT32,		"uint32",	"l"},
	{LIBZBXMODBUS_SIGNED_INT32,	"int32",	"S"},
	{LIBZBXMODBUS_FLOAT,		"float",	"f"},
	{LIBZBXMODBUS_UINT64,		"uint64",	"I"},
	{LIBZBXMODBUS_SIGNED_INT64,	"int64",	NULL},
	{LIBZBXMODBUS_FLOAT64,		"double",	"d"},
	{LIBZBXMODBUS_SKIP,		"skip",		NULL},
	{LIBZBXMODBUS_NONE,		NULL,		NULL}
};
/* clang-format on */

const char token_delimiters[] = " +*"; /* terminating '\0' counts as delimiter too! */

static datatype_code_t parse_type_name(const datatype_token_t *tokens, const char *string, int *jump)
{
	size_t			token_length = SIZE_MAX, i = sizeof(token_delimiters);
	const datatype_token_t *allowed_token;

	while (0 < i--)
	{
		const char *token_delimiter;

		if (NULL == (token_delimiter = strchr(string, token_delimiters[i])))
			continue;

		if ((size_t)(token_delimiter - string) < token_length)
			token_length = token_delimiter - string;
	}

	for (allowed_token = tokens; LIBZBXMODBUS_NONE != allowed_token->type_code; allowed_token++)
	{
		if (token_length == strlen(allowed_token->name) &&
			0 == strncmp(allowed_token->name, string, token_length))
		{
			*jump = token_length;
			return allowed_token->type_code;
		}

		if (NULL == allowed_token->legacy_name)
			continue;

		if (token_length == strlen(allowed_token->legacy_name) &&
			0 == strncmp(allowed_token->legacy_name, string, token_length))
		{
			*jump = token_length;
			return allowed_token->type_code;
		}
	}

	return LIBZBXMODBUS_NONE;
}

typedef enum {
	MULTIPLIER_ON_THE_LEFT,
	MULTIPLIER_ON_THE_RIGHT,
	CROSS_AFTER_MULTIPLIER,
	CROSS_AFTER_TYPE,
	TYPE_ON_THE_LEFT,
	TYPE_ON_THE_RIGHT,
	PLUS_OR_NOTHING
} parser_state_t;

struct datatype_parse_s
{
	datatype_code_t type_code;
	int		multiplier;
};

static void append(datatype_parse_t **layout, size_t *layout_alloc, size_t *layout_offset, int multiplier,
	datatype_code_t type_code)
{
	if (*layout_alloc <= *layout_offset)
		*layout =
			realloc(*layout, (*layout_alloc += 16) * sizeof(datatype_parse_t)); /* FIXME realloc() may
											       return NULL, handle it
											       somehow */

	(*layout)[(*layout_offset)++] = (datatype_parse_t){.type_code = type_code, .multiplier = multiplier};
}

int parse_datatype(datatype_syntax_t syntax, const char *datatype, datatype_parse_t **layout, char **error)
{
	const char *		p = datatype;
	const datatype_token_t *tokens;
	parser_state_t		state = MULTIPLIER_ON_THE_LEFT;
	datatype_code_t		type_code;
	size_t			layout_alloc = 0, layout_offset = 0;
	int			reg_count = 0, multiplier;

	if (NULL == datatype || '\0' == *datatype)
		return 1;

	switch (syntax)
	{
		case BIT_SYNTAX:
			tokens = bit_syntax;
			break;
		case REGISTER_SYNTAX:
			tokens = register_syntax;
			break;
		default:
			return -1;
	}

	*layout = NULL;

	while ('\0' != *p)
	{
		int jump;

		if (' ' == *p)
		{
			p++;
			continue;
		}

		switch (state)
		{
			case MULTIPLIER_ON_THE_LEFT: /* optional multiplier on the left of type string */
				if (1 != sscanf(p, "%d%n", &multiplier, &jump))
				{
					state = TYPE_ON_THE_LEFT;
				}
				else if (0 >= multiplier)
				{
					*error = strdup("Multiplier must be positive.");
					free(*layout);
					return -1;
				}
				else
				{
					state = CROSS_AFTER_MULTIPLIER;
					p += jump;
				}
				continue;
			case CROSS_AFTER_MULTIPLIER: /* mandatory "*" after left hand side multiplier */
				if ('*' != *p)
				{
					*error = strdup("There must be \"*\" sign after a multiplier and before a "
							"type.");
					free(*layout);
					return -1;
				}
				state = TYPE_ON_THE_RIGHT;
				p++;
				continue;
			case TYPE_ON_THE_RIGHT: /* type string after multiplier and "*" sign */
				if (LIBZBXMODBUS_NONE == (type_code = parse_type_name(tokens, p, &jump)))
				{
					*error = strdup("Invalid type in datatype expression.");
					free(*layout);
					return -1;
				}
				state = PLUS_OR_NOTHING;
				p += jump;
				break;
			case TYPE_ON_THE_LEFT: /* type string without multiplier before it */
				if (LIBZBXMODBUS_NONE == (type_code = parse_type_name(tokens, p, &jump)))
				{
					*error = strdup("Invalid type in datatype expression.");
					free(*layout);
					return -1;
				}
				state = CROSS_AFTER_TYPE;
				p += jump;
				continue;
			case CROSS_AFTER_TYPE: /* optional "*" with multiplier after type string */
				if ('*' != *p)
				{
					state = PLUS_OR_NOTHING;
					multiplier = 1;
					break;
				}
				state = MULTIPLIER_ON_THE_RIGHT;
				p++;
				continue;
			case MULTIPLIER_ON_THE_RIGHT: /* mandatory multiplier after type string and "*" */
				if (1 != sscanf(p, "%d%n", &multiplier, &jump))
				{
					*error = strdup("Multiplier expected after type and \"*\" sign.");
					free(*layout);
					return -1;
				}
				else if (0 >= multiplier)
				{
					*error = strdup("Multiplier must be positive.");
					free(*layout);
					return -1;
				}
				else
				{
					state = PLUS_OR_NOTHING;
					p += jump;
					break;
				}
			case PLUS_OR_NOTHING: /* next summand or the end of expression */
				if ('+' != *p)
				{
					*error = strdup("Expected \"+\" or the end of expression.");
					free(*layout);
					return -1;
				}
				state = MULTIPLIER_ON_THE_LEFT;
				p++;
				continue;
		}

		reg_count += multiplier * type_registers(type_code);
		append(layout, &layout_alloc, &layout_offset, multiplier, type_code);
	}

	if (CROSS_AFTER_TYPE == state)
	{
		reg_count += type_registers(type_code);
		append(layout, &layout_alloc, &layout_offset, 1, type_code);
	}
	else if (PLUS_OR_NOTHING != state)
	{
		*error = strdup("Unexpected end of expression.");
		free(*layout);
		return -1;
	}

	append(layout, &layout_alloc, &layout_offset, 0, LIBZBXMODBUS_NONE);

	return reg_count;
}

static void strappf(char **old, size_t *old_size, size_t *old_offset, const char *format, ...)
{
	char *new = *old;
	size_t  new_size = *old_size, new_offset = *old_offset;
	va_list args;

	if (NULL == new &&NULL == (new = malloc(new_size = 128)))
		return;

	va_start(args, format);
	new_offset += vsnprintf(new + new_offset, new_size - new_offset, format, args);
	va_end(args);

	if (new_offset >= new_size)
	{
		new_size = new_offset + 1;

		if (NULL != (new = realloc(new, new_size)))
		{
			va_start(args, format);
			vsnprintf(new + *old_offset, new_size - *old_offset, format, args);
			va_end(args);
		}
		else
			new_size = new_offset = 0;
	}

	*old = new;
	*old_size = new_size;
	*old_offset = new_offset;
}

void set_result_based_on_datatype(AGENT_RESULT *result, const datatype_parse_t *layout, int start, const uint8_t *bits,
	const uint16_t *registers, endianness_code_t endianness)
{
	if (1 == layout[0].multiplier && LIBZBXMODBUS_NONE == layout[1].type_code)
	{
		/* there is a single value to return */

		uint32_t ui32;
		float    f;
		uint64_t ui64;
		double   d;

		switch (layout[0].type_code)
		{
			case LIBZBXMODBUS_BIT:
				SET_UI64_RESULT(result, bits[0]);
				return;
			case LIBZBXMODBUS_UINT16:
				SET_UI64_RESULT(result, registers[0]);
				return;
			case LIBZBXMODBUS_SIGNED_INT:
				/* use Numeric (float) data type in Zabbix item */
				SET_DBL_RESULT(result, (int16_t)registers[0]);
				return;
			case LIBZBXMODBUS_UINT32:
				SET_UI64_RESULT(result, extract32bits(registers, endianness));
				return;
			case LIBZBXMODBUS_SIGNED_INT32:
				/* use Numeric (float) data type in Zabbix item */
				SET_DBL_RESULT(result, (int32_t)extract32bits(registers, endianness));
				return;
			case LIBZBXMODBUS_FLOAT:
				ui32 = extract32bits(registers, endianness);
				memcpy(&f, &ui32, 4);
				SET_DBL_RESULT(result, f);
				return;
			case LIBZBXMODBUS_UINT64:
				SET_UI64_RESULT(result, extract64bits(registers, endianness));
				return;
			case LIBZBXMODBUS_SIGNED_INT64:
				/* use Numeric (float) data type in Zabbix item */
				SET_DBL_RESULT(result, (int64_t)extract64bits(registers, endianness));
				return;
			case LIBZBXMODBUS_FLOAT64:
				ui64 = extract64bits(registers, endianness);
				memcpy(&d, &ui64, 8);
				SET_DBL_RESULT(result, d);
				return;
			case LIBZBXMODBUS_SKIP:
			case LIBZBXMODBUS_NONE:
			default:
				return;
		}
	}

	char *      json = NULL;
	size_t      json_alloc = 0, json_offset = 0;
	const char *delimiter = "";

	strappf(&json, &json_alloc, &json_offset, "{");

	while (LIBZBXMODBUS_NONE != layout->type_code)
	{
		int i, step;

		for (i = 0, step = type_registers(layout->type_code); i < layout->multiplier;
			i++, start += step, bits += step, registers += step)
		{
			uint32_t ui32;
			float    f;
			uint64_t ui64;
			double   d;

			if (LIBZBXMODBUS_SKIP == layout->type_code)
				continue;

			strappf(&json, &json_alloc, &json_offset, "%s\"%d\":", delimiter, start);

			switch (layout->type_code)
			{
				case LIBZBXMODBUS_BIT:
					strappf(&json, &json_alloc, &json_offset, "%" PRIu8, *bits);
					break;
				case LIBZBXMODBUS_UINT16:
					strappf(&json, &json_alloc, &json_offset, "%" PRIu16, *registers);
					break;
				case LIBZBXMODBUS_SIGNED_INT:
					strappf(&json, &json_alloc, &json_offset, "%" PRId16, (int16_t)*registers);
					break;
				case LIBZBXMODBUS_UINT32:
					strappf(&json, &json_alloc, &json_offset, "%" PRIu32,
						extract32bits(registers, endianness));
					break;
				case LIBZBXMODBUS_SIGNED_INT32:
					strappf(&json, &json_alloc, &json_offset, "%" PRId32,
						(int32_t)extract32bits(registers, endianness));
					break;
				case LIBZBXMODBUS_FLOAT:
					ui32 = extract32bits(registers, endianness);
					memcpy(&f, &ui32, 4);
					strappf(&json, &json_alloc, &json_offset, "%f", (double)f);
					break;
				case LIBZBXMODBUS_UINT64:
					strappf(&json, &json_alloc, &json_offset, "%" PRIu64,
						extract64bits(registers, endianness));
					break;
				case LIBZBXMODBUS_SIGNED_INT64:
					strappf(&json, &json_alloc, &json_offset, "%" PRId64,
						(int64_t)extract64bits(registers, endianness));
					break;
				case LIBZBXMODBUS_FLOAT64:
					ui64 = extract64bits(registers, endianness);
					memcpy(&d, &ui64, 8);
					strappf(&json, &json_alloc, &json_offset, "%f", d);
					break;
				case LIBZBXMODBUS_SKIP:
				case LIBZBXMODBUS_NONE:
				default:
					break;
			}

			delimiter = ",";
		}

		layout++;
	}

	strappf(&json, &json_alloc, &json_offset, "}");

	SET_TEXT_RESULT(result, json);
}
