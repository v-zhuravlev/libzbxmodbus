#include <stdio.h>

#include "datatype.h"

typedef enum
{
	LIBZBXMODBUS_SKIP,	/* register to read, but discard */
	LIBZBXMODBUS_BIT,
	LIBZBXMODBUS_UINT16,
	LIBZBXMODBUS_SIGNED_INT,
	LIBZBXMODBUS_UINT32,
	LIBZBXMODBUS_SIGNED_INT32,
	LIBZBXMODBUS_FLOAT,
	LIBZBXMODBUS_SIGNED_INT64,
	LIBZBXMODBUS_UINT64,
	LIBZBXMODBUS_FLOAT64,
	LIBZBXMODBUS_NONE	/* special code for invalid type */
}
datatype_code_t;

static int	type_registers(datatype_code_t type_code)
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
		case LIBZBXMODBUS_SIGNED_INT64:
		case LIBZBXMODBUS_UINT64:
		case LIBZBXMODBUS_FLOAT64:
			return 4;
		case LIBZBXMODBUS_NONE:
		default:
			return 0;
	}
}

typedef struct
{
	const datatype_code_t	type_code;
	const char		*name;
	const char		*legacy_name;
}
datatype_token_t;

const datatype_token_t	bit_syntax[] =
{
	{LIBZBXMODBUS_BIT,		"bit",		"b"},
	{LIBZBXMODBUS_SKIP,		"skip",		NULL},
	{LIBZBXMODBUS_NONE,		NULL,		NULL}
};

const datatype_token_t	register_syntax[] =
{
	{LIBZBXMODBUS_UINT16,		"uint16",	"i"},
	{LIBZBXMODBUS_SIGNED_INT,	"int16",	"s"},
	{LIBZBXMODBUS_UINT32,		"uint32",	"l"},
	{LIBZBXMODBUS_SIGNED_INT32,	"int32",	"S"},
	{LIBZBXMODBUS_FLOAT,		"float",	"f"},
	{LIBZBXMODBUS_SIGNED_INT64,	"int64",	NULL},
	{LIBZBXMODBUS_UINT64,		"uint64",	"I"},
	{LIBZBXMODBUS_FLOAT64,		"double",	"d"},
	{LIBZBXMODBUS_SKIP,		"skip",		NULL},
	{LIBZBXMODBUS_NONE,		NULL,		NULL}
};

const char	token_delimiters[] = " +*";	/* terminating '\0' counts as delimiter too! */

static datatype_code_t	parse_type_name(const datatype_token_t *tokens, const char *string, int *jump)
{
	int			token_length = -1;
	size_t			i = sizeof(token_delimiters);
	const datatype_token_t	*allowed_token;

	while (0 < i--)
	{
		const char	*token_delimiter;

		if (NULL == (token_delimiter = strchr(string, token_delimiters[i])))
			continue;

		if (-1 == token_length || token_delimiter - string < token_length)
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

typedef enum
{
	MULTIPLIER_ON_THE_LEFT,
	MULTIPLIER_ON_THE_RIGHT,
	CROSS_AFTER_MULTIPLIER,
	CROSS_AFTER_TYPE,
	TYPE_ON_THE_LEFT,
	TYPE_ON_THE_RIGHT,
	PLUS_OR_NOTHING
}
parser_state_t;

int	parse_datatype(datatype_syntax_t syntax, const char *datatype, char **error)
{
	const char		*p = datatype;
	const datatype_token_t	*tokens;
	parser_state_t		state = MULTIPLIER_ON_THE_LEFT;
	datatype_code_t		type_code;
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

	while ('\0' != *p)
	{
		int	jump;

		if (' ' == *p)
		{
			p++;
			continue;
		}

		switch (state)
		{
			case MULTIPLIER_ON_THE_LEFT:	/* optional multiplier on the left of type string */
				if (1 != sscanf(p, "%d%n", &multiplier, &jump))
				{
					state = TYPE_ON_THE_LEFT;
				}
				else if (0 >= multiplier)
				{
					*error = strdup("Multiplier must be positive.");
					return -1;
				}
				else
				{
					state = CROSS_AFTER_MULTIPLIER;
					p += jump;
				}
				continue;
			case CROSS_AFTER_MULTIPLIER:	/* mandatory "*" after left hand side multiplier */
				if ('*' != *p)
				{
					*error = strdup("There must be \"*\" sign after a multiplier and before a type.");
					return -1;
				}
				state = TYPE_ON_THE_RIGHT;
				p++;
				continue;
			case TYPE_ON_THE_RIGHT:		/* type string after multiplier and "*" sign */
				if (LIBZBXMODBUS_NONE == (type_code = parse_type_name(tokens, p, &jump)))
				{
					*error = strdup("Invalid type in datatype expression.");
					return -1;
				}
				state = PLUS_OR_NOTHING;
				p += jump;
				break;
			case TYPE_ON_THE_LEFT:		/* type string without multiplier before it */
				if (LIBZBXMODBUS_NONE == (type_code = parse_type_name(tokens, p, &jump)))
				{
					*error = strdup("Invalid type in datatype expression.");
					return -1;
				}
				state = CROSS_AFTER_TYPE;
				p += jump;
				continue;
			case CROSS_AFTER_TYPE:		/* optional "*" with multiplier after type string */
				if ('*' != *p)
				{
					state = PLUS_OR_NOTHING;
					multiplier = 1;
					break;
				}
				state = MULTIPLIER_ON_THE_RIGHT;
				p++;
				continue;
			case MULTIPLIER_ON_THE_RIGHT:	/* mandatory multiplier after type string and "*" */
				if (1 != sscanf(p, "%d%n", &multiplier, &jump))
				{
					*error = strdup("Multiplier expected after type and \"*\" sign.");
					return -1;
				}
				else if (0 >= multiplier)
				{
					*error = strdup("Multiplier must be positive.");
					return -1;
				}
				else
				{
					state = PLUS_OR_NOTHING;
					p += jump;
					break;
				}
			case PLUS_OR_NOTHING:		/* next summand or the end of expression */
				if ('+' != *p)
				{
					*error = strdup("Expected \"+\" or the end of expression.");
					return -1;
				}
				state = MULTIPLIER_ON_THE_LEFT;
				p++;
				continue;
		}

		reg_count += multiplier * type_registers(type_code);
	}

	if (CROSS_AFTER_TYPE == state)
	{
		reg_count += type_registers(type_code);
	}
	else if (PLUS_OR_NOTHING != state)
	{
		*error = strdup("Unexpected end of expression.");
		return -1;
	}

	return reg_count;
}

void	set_result_based_on_datatype(AGENT_RESULT *result, const char *datatype, int start, const uint8_t *bits,
		size_t bits_size, const uint16_t *registers, size_t registers_size, int endianness)
{
	/* TODO */
}
