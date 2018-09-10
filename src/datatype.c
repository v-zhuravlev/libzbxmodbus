#include <stdio.h>

#include "datatype.h"

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

const datatype_token_t	bit_syntax[] =
{
	{"bit",		"b",	MODBUS_BIT,		1},
	{"skip",	NULL,	MODBUS_SKIP,		1},
	{NULL}
};

const datatype_token_t	register_syntax[] =
{
	{"uint16",	"i",	MODBUS_UINT16,		1},
	{"int16",	"s",	MODBUS_SIGNED_INT,	1},
	{"uint32",	"l",	MODBUS_UINT32,		2},
	{"int32",	"S",	MODBUS_SIGNED_INT32,	2},
	{"float",	"f",	MODBUS_FLOAT,		2},
	{"int64",	NULL,	MODBUS_SIGNED_INT64,	4},
	{"uint64",	"I",	MODBUS_UINT64,		4},
	{"double",	"d",	MODBUS_FLOAT64,		4},
	{"skip",	NULL,	MODBUS_SKIP,		1},
	{NULL}
};

static int	parse_type_name(const datatype_token_t *tokens, const char *string, int *jump)
{
	int			token_length = -1;
	const char		token_delimiters[] = " +*";	/* terminating '\0' counts as delimiter too! */
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

	for (allowed_token = tokens; NULL != allowed_token->name; allowed_token++)
	{
		if (token_length == strlen(allowed_token->name) &&
				0 == strncmp(allowed_token->name, string, token_length))
		{
			*jump = token_length;
			return allowed_token->regs_to_read;
		}

		if (NULL == allowed_token->legacy_name)
			continue;

		if (token_length == strlen(allowed_token->legacy_name) &&
				0 == strncmp(allowed_token->legacy_name, string, token_length))
		{
			*jump = token_length;
			return allowed_token->regs_to_read;
		}
	}

	return -1;
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
	int			reg_count = 0, multiplier, type_registers;

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
				if (1 == sscanf(p, "%d%n", &multiplier, &jump))
				{
					state = CROSS_AFTER_MULTIPLIER;
					p += jump;
				}
				else
					state = TYPE_ON_THE_LEFT;
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
				if (-1 == (type_registers = parse_type_name(tokens, p, &jump)))
				{
					*error = strdup("Invalid type in datatype expression.");
					return -1;
				}
				state = PLUS_OR_NOTHING;
				p += jump;
				break;
			case TYPE_ON_THE_LEFT:		/* type string without multiplier before it */
				if (-1 == (type_registers = parse_type_name(tokens, p, &jump)))
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
				if (1 == sscanf(p, "%d%n", &multiplier, &jump))
				{
					state = PLUS_OR_NOTHING;
					p += jump;
					break;
				}
				*error = strdup("Multiplier expected after type and \"*\" sign.");
				return -1;
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

		reg_count += multiplier * type_registers;
	}

	if (CROSS_AFTER_TYPE == state)
	{
		reg_count += type_registers;
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
