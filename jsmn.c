/**
 * Original author: Serge Zaitsev <zaitsev.serge@gmail.com>
 * Event based stream parsing rewrite: Sakari Kapanen <sakari.m.kapanen@gmail.com>
 */

#include "jsmn.h"

#include <stdbool.h>

#define JSMN_CALLBACK(f, ...) if ((f) != NULL) { (f)(__VA_ARGS__); }

static bool jsmn_stack_push(jsmn_parser *parser, jsmntype_t type) {
	if (parser->stack_height >= JSMN_MAX_DEPTH) {
		return false;
	}
	parser->type_stack[parser->stack_height++] = type;
	return true;
}

static jsmntype_t jsmn_stack_pop(jsmn_parser *parser) {
	if (parser->stack_height == 0) {
		return JSMN_UNDEFINED;
	}
	return parser->type_stack[parser->stack_height--];
}

static jsmntype_t jsmn_stack_top(jsmn_parser *parser) {
	if (parser->stack_height == 0) {
		return JSMN_UNDEFINED;
	}
	return parser->type_stack[parser->stack_height - 1];
}

/**
 * Fills next available token with JSON primitive.
 */
static int jsmn_parse_primitive(jsmn_parser *parser, char cin) {
	/* Leave space for the terminating null character */
	if (parser->buffer_size == JSMN_BUFFER_SIZE - 1) {
		return JSMN_ERROR_NOMEM;
	}
	parser->buffer[parser->buffer_size++] = cin;
	size_t len = parser->buffer_size;
	const char *js = parser->buffer;
	for (int pos = 0; pos < len && js[pos] != '\0'; pos++) {
		switch (js[pos]) {
			case '\t' : case '\r' : case '\n' : case ' ' :
			case ','  : case ']'  : case '}' :
				goto found;
		}
		if (js[pos] < 32 || js[pos] >= 127) {
			return JSMN_ERROR_INVAL;
		}
	}
	/* In strict mode primitive must be followed by a comma/object/array */
	return JSMN_ERROR_PART;

found:
	parser->buffer[len - 1] = '\0';
	JSMN_CALLBACK(parser->callbacks.primitive_callback, js, len - 1);
	parser->buffer_size = 0;
	parser->state = JSMN_PARSING;
	return 0;
}

/**
 * Fills next token with JSON string.
 */
static int jsmn_parse_string(jsmn_parser *parser, char cin) {
	/* Leave space for the terminating null character */
	if (parser->buffer_size == JSMN_BUFFER_SIZE - 1) {
		return JSMN_ERROR_NOMEM;
	}
	parser->buffer[parser->buffer_size++] = cin;
	size_t len = parser->buffer_size;
	const char *js = parser->buffer;
	for (int pos = 0; pos < len; pos++) {
		char c = js[pos];

		/* Quote: end of string */
		if (c == '\"') {
			parser->buffer[len - 1] = '\0';
			JSMN_CALLBACK(jsmn_stack_top(parser) == JSMN_KEY ?
				parser->callbacks.string_callback : parser->callbacks.object_key_callback,
				js, len - 1);
			parser->buffer_size = 0;
			parser->state = JSMN_PARSING;
			return 0;
		}

		/* Backslash: Quoted symbol expected */
		if (c == '\\' && pos + 1 < len) {
			int i;
			pos++;
			switch (js[pos]) {
				/* Allowed escaped symbols */
				case '\"': case '/' : case '\\' : case 'b' :
				case 'f' : case 'r' : case 'n'  : case 't' :
					break;
				/* Allows escaped symbol \uXXXX */
				case 'u':
					pos++;
					for(i = 0; i < 4 && pos < len && js[pos] != '\0'; i++) {
						/* If it isn't a hex character we have an error */
						if(!((js[pos] >= 48 && js[pos] <= 57) || /* 0-9 */
									(js[pos] >= 65 && js[pos] <= 70) || /* A-F */
									(js[pos] >= 97 && js[pos] <= 102))) { /* a-f */
							return JSMN_ERROR_INVAL;
						}
						pos++;
					}
					pos--;
					break;
				/* Unexpected symbol */
				default:
					return JSMN_ERROR_INVAL;
			}
		}
	}
	return JSMN_ERROR_PART;
}

/**
 * Parse JSON string and fill tokens.
 */
int jsmn_parse(jsmn_parser *parser, char c) {
	jsmntype_t type;
	int r;

	switch (parser->state) {
		case JSMN_PARSING:
			switch (c) {
				case '{': case '[':
					if (c == '{') {
						type = JSMN_OBJECT;
						JSMN_CALLBACK(parser->callbacks.start_object_callback);
					} else {
						type = JSMN_ARRAY;
						JSMN_CALLBACK(parser->callbacks.start_array_callback);
					}
					if (!jsmn_stack_push(parser, type)) {
						return JSMN_ERROR_MAX_DEPTH;
					}
					break;
				case '}': case ']':
					if (c == '}') {
						JSMN_CALLBACK(parser->callbacks.end_object_callback);
					} else {
						JSMN_CALLBACK(parser->callbacks.end_array_callback);
					}
					jsmn_stack_pop(parser);
					if (jsmn_stack_top(parser) == JSMN_KEY) {
						jsmn_stack_pop(parser);
					}
					break;
				case '\"':
					parser->state = JSMN_PARSING_STRING;
					break;
				case '\t' : case '\r' : case '\n' : case ' ' : case ',':
					break;
				case ':':
					if (jsmn_stack_top(parser) == JSMN_OBJECT &&
						!jsmn_stack_push(parser, JSMN_KEY)) {
						return JSMN_ERROR_MAX_DEPTH;
					}
					break;
				/* In strict mode primitives are: numbers and booleans */
				case '-': case '0': case '1' : case '2': case '3' : case '4':
				case '5': case '6': case '7' : case '8': case '9':
				case 't': case 'f': case 'n' :
					if (jsmn_stack_top(parser) == JSMN_OBJECT) {
						return JSMN_ERROR_INVAL;
					}
					parser->state = JSMN_PARSING_PRIMITIVE;
					jsmn_parse(parser, c);
					break;

				/* Unexpected char in strict mode */
				default:
					return JSMN_ERROR_INVAL;
			}
			break;

		case JSMN_PARSING_STRING:
			r = jsmn_parse_string(parser, c);
			if (r < 0) return r;
			if (jsmn_stack_top(parser) == JSMN_KEY) {
				jsmn_stack_pop(parser);
			}
			break;

		case JSMN_PARSING_PRIMITIVE:
			r = jsmn_parse_primitive(parser, c);
			if (r < 0) return r;
			else if (r == 0) {
				if (jsmn_stack_top(parser) == JSMN_KEY) {
					jsmn_stack_pop(parser);
				}
				return jsmn_parse(parser, c);
			}
			break;
	}

	return 0;
}

/**
 * Creates a new parser based over a given  buffer with an array of tokens
 * available.
 */
void jsmn_init(jsmn_parser *parser, jsmn_callbacks_t *callbacks) {
	parser->state = JSMN_PARSING;
	parser->stack_height = 0;
	parser->buffer_size = 0;
	parser->callbacks = *callbacks;
}

