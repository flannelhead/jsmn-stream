# jsmn-stream

A streaming JSON parser based on [jsmn](https://github.com/zserge/jsmn) by Serge Zaitsev.
The code has been rewritten to take an input stream character by character and
emit events to user supplied callbacks. Such a parsing scheme is especially
beneficial on embedded systems where the whole parse tree (and possibly the
whole JSON string) cannot be stored in RAM at once.

## Truncated values

In some cases, you can not control lengths of keys and\or string values in JSON you need to parse.
`can_truncate` flag adds ability to receive truncated values and continue parsing JSON instead of stopping at `JSMN_STREAM_ERROR_NOMEM` error.
You should use additional parameter `can_truncate` in `jsmn_stream_init(pParser, pCallbacks, pUserArg, char can_truncate)` to specify if object keys and string values can be truncated (`JSMN_STREAM_TRUNCATING_ENABLE`) if value exceeds buffer size or stop throw `JSMN_STREAM_ERROR_NOMEM` (`(JSMN_STREAM_TRUNCATING_DISABLE`) instead.

Callbacks `object_key_callback` and `string_callback` have 4th `char` parameter indicating if value is truncated (`(char)1`) or not (`(char)0`).

## Compiler defines

* `JSMN_STREAM_BUFFER_SIZE` : Maximum buffer size for tokens in bytes. Defaults to `512` bytes. Strings and keys longer than `this-1` will be truncated (if `JSMN_STREAM_TRUNCATING_ENABLE` is set) or stop parsing with `JSMN_STREAM_ERROR_NOMEM` error.
* `JSMN_STREAM_MAX_DEPTH` : Maximum depth of JSON. Defaults to `32` levels. If JSON exceeds this depth, `JSMN_STREAM_ERROR_MAX_DEPTH` will be thrown.

Example:

`gcc -DJSMN_STREAM_BUFFER_SIZE=20 -DJSMN_STREAM_MAX_DEPTH=32`

## Examples

See the [examples](examples) folder.

## License

Like the original jsmn project, this one is licensed under the MIT license.

