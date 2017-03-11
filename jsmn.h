/**
 * Original author: Serge Zaitsev <zaitsev.serge@gmail.com>
 * Event based stream parsing rewrite: Sakari Kapanen <sakari.m.kapanen@gmail.com>
 */
#ifndef __JSMN_H_
#define __JSMN_H_

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define JSMN_MAX_DEPTH 32
#define JSMN_BUFFER_SIZE 512

/**
 * JSON type identifier. Basic types are:
 * 	o Object
 * 	o Array
 * 	o String
 * 	o Other primitive: number, boolean (true/false) or null
 */
typedef enum {
	JSMN_UNDEFINED = 0,
	JSMN_OBJECT = 1,
	JSMN_ARRAY = 2,
	JSMN_STRING = 3,
	JSMN_PRIMITIVE = 4,
	JSMN_KEY = 5
} jsmntype_t;

enum jsmnerr {
	/* Buffer not large enough */
	JSMN_ERROR_NOMEM = -1,
	/* Invalid character inside JSON string */
	JSMN_ERROR_INVAL = -2,
	/* The string is not a full JSON packet, more bytes expected */
	JSMN_ERROR_PART = -3,
	/* Reached maximal stack depth (too deep nesting) */
	JSMN_ERROR_MAX_DEPTH = -4
};

typedef enum {
    JSMN_PARSING = 0,
    JSMN_PARSING_STRING = 1,
    JSMN_PARSING_PRIMITIVE = 2
} jsmnstate_t;

/**
 * A structure containing callbacks for various parse events.
 */
typedef struct {
	void (* start_array_callback)(void);
	void (* end_array_callback)(void);
	void (* start_object_callback)(void);
	void (* end_object_callback)(void);
	void (* object_key_callback)(const char *key, size_t key_length);
	void (* string_callback)(const char *value, size_t length);
	void (* primitive_callback)(const char *value, size_t length);
} jsmn_callbacks_t;

/**
 * JSON parser. Contains an array of token blocks available. Also stores
 * the string being parsed now and current position in that string
 */
typedef struct {
    jsmnstate_t state;
	jsmn_callbacks_t callbacks; /* callbacks for parse events */
	jsmntype_t type_stack[JSMN_MAX_DEPTH]; /* Stack for storing the type structure */
	size_t stack_height;
	char buffer[JSMN_BUFFER_SIZE];
	size_t buffer_size;
} jsmn_parser;

/**
 * Create JSON parser over an array of tokens
 */
void jsmn_init(jsmn_parser *parser, jsmn_callbacks_t *callbacks);

/**
 * Run JSON parser. It parses a JSON data string into and array of tokens, each describing
 * a single JSON object.
 */
int jsmn_parse(jsmn_parser *parser, char c);

#ifdef __cplusplus
}
#endif

#endif /* __JSMN_H_ */
