#include "mock-transport.h"
#include "proto_lib.h"
#include "msg_parse_lib.h"

static void
test_log_proto_server_options_limits(void)
{
  LogProtoServerOptions opts;

  log_proto_server_options_defaults(&opts);
  log_proto_server_options_init(&opts, configuration);
  assert_true(opts.max_msg_size > 0, "LogProtoServerOptions.max_msg_size is not initialized properly, max_msg_size=%d", opts.max_msg_size);
  assert_true(opts.init_buffer_size > 0, "LogProtoServerOptions.init_buffer_size is not initialized properly, init_buffer_size=%d", opts.init_buffer_size);
  assert_true(opts.max_buffer_size > 0, "LogProtoServerOptions.max_buffer_size is not initialized properly, max_buffer_size=%d", opts.max_buffer_size);

  assert_true(log_proto_server_options_validate(&opts), "Default LogProtoServerOptions were deemed invalid");
  log_proto_server_options_destroy(&opts);
}

static void
test_log_proto_server_options_valid_encoding(void)
{
  LogProtoServerOptions opts;

  log_proto_server_options_defaults(&opts);
  /* check that encoding can be set and error is properly returned */
  log_proto_server_options_set_encoding(&opts, "utf-8");
  log_proto_server_options_set_encoding_mode(&opts, LP_ENCODING_MODE_STRICT);
  assert_string(opts.encoding, "utf-8", "LogProtoServerOptions.encoding was not properly set");

  log_proto_server_options_init(&opts, configuration);
  assert_true(log_proto_server_options_validate(&opts), "utf-8 was not accepted as a valid LogProtoServerOptions");
  log_proto_server_options_destroy(&opts);
}

static void
test_log_proto_server_options_invalid_encoding(void)
{
  LogProtoServerOptions opts;
  gboolean success;

  log_proto_server_options_defaults(&opts);

  log_proto_server_options_set_encoding(&opts, "never-ever-is-going-to-be-such-an-encoding");
  assert_string(opts.encoding, "never-ever-is-going-to-be-such-an-encoding", "LogProtoServerOptions.encoding was not properly set");

  log_proto_server_options_init(&opts, configuration);
  start_grabbing_messages();
  success = log_proto_server_options_validate(&opts);
  assert_grabbed_messages_contain("Unknown character set name specified; encoding='never-ever-is-going-to-be-such-an-encoding'", "message about unknown charset missing");
  assert_false(success, "Successfully set a bogus encoding, which is insane");

  log_proto_server_options_destroy(&opts);
}

static void
test_log_proto_server_options_encoding_mode_strict(void)
{
  LogProtoServerOptions opts;
  gboolean success;

  log_proto_server_options_defaults(&opts);
  log_proto_server_options_set_encoding_mode(&opts, LP_ENCODING_MODE_STRICT);
  assert_true(LP_ENCODING_MODE_STRICT == opts.encoding_mode, "Setting encoding-mode to strict failed");

  log_proto_server_options_init(&opts, configuration);
  start_grabbing_messages();
  success = log_proto_server_options_validate(&opts);
  assert_grabbed_messages_contain("Invalid use of encoding-mode(strict) without an explicit encoding() specified", "message about encoding-mode(strict) being invalid without encoding specified missing");
  assert_false(success, "Successfully set strict encoding mode, without specifying an encoding");

  log_proto_server_options_destroy(&opts);
}

static void
test_log_proto_server_options_encoding_mode_8bit_clean_w_encoding(void)
{
  LogProtoServerOptions opts;
  gboolean success;

  log_proto_server_options_defaults(&opts);
  log_proto_server_options_set_encoding_mode(&opts, LP_ENCODING_MODE_8BIT_CLEAN);
  log_proto_server_options_set_encoding(&opts, "utf-8");
  assert_true(LP_ENCODING_MODE_8BIT_CLEAN == opts.encoding_mode, "Setting encoding-mode to 8bit-clean failed");

  log_proto_server_options_init(&opts, configuration);
  start_grabbing_messages();
  success = log_proto_server_options_validate(&opts);
  assert_grabbed_messages_contain("Invalid use of encoding-mode(8bit-clean) with an explicit encoding() specified", "message about encoding-mode(8bit-clean) being invalid with explicit encoding specified missing");
  assert_false(success, "Successfully set 8bit-clean encoding mode, with an explicit encoding");

  log_proto_server_options_destroy(&opts);
}

static void
test_log_proto_server_options_encoding_mode_8bit_clean_wo_encoding(void)
{
  LogProtoServerOptions opts;
  gboolean success;

  log_proto_server_options_defaults(&opts);
  log_proto_server_options_set_encoding_mode(&opts, LP_ENCODING_MODE_8BIT_CLEAN);
  assert_true(LP_ENCODING_MODE_8BIT_CLEAN == opts.encoding_mode, "Setting encoding-mode to 8bit-clean failed");

  log_proto_server_options_init(&opts, configuration);
  success = log_proto_server_options_validate(&opts);
  assert_true(success, "Failed to set 8bit-clean encoding mode, without an explicit encoding");

  log_proto_server_options_destroy(&opts);
}

static void
test_log_proto_server_options_encoding_mode_assume_utf8_non_utf8_encoding(void)
{
  LogProtoServerOptions opts;
  gboolean success;

  log_proto_server_options_defaults(&opts);
  log_proto_server_options_set_encoding_mode(&opts, LP_ENCODING_MODE_ASSUME_UTF8);
  log_proto_server_options_set_encoding(&opts, "UCS-2");
  assert_true(LP_ENCODING_MODE_ASSUME_UTF8 == opts.encoding_mode, "Setting encoding-mode to assume-utf8 failed");
  assert_string(opts.encoding, "UCS-2", "LogProtoServerOptions.encoding was not properly set");

  log_proto_server_options_init(&opts, configuration);
  start_grabbing_messages();
  success = log_proto_server_options_validate(&opts);
  assert_grabbed_messages_contain("Invalid use of explicit non UTF-8 encoding() with current encoding-mode(); encoding='UCS-2'", "message about encoding-mode(assume-utf8) being invalid with non utf-8 encoding() missing");
  assert_false(success, "Succesfully set assume-utf8 encoding mode with a non-utf8 encoding");

  log_proto_server_options_destroy(&opts);
}

static void
test_log_proto_server_options_encoding_mode_assume_utf8_utf8_encoding(void)
{
  LogProtoServerOptions opts;
  gboolean success;

  log_proto_server_options_defaults(&opts);
  log_proto_server_options_set_encoding_mode(&opts, LP_ENCODING_MODE_ASSUME_UTF8);
  log_proto_server_options_set_encoding(&opts, "utf-8");
  assert_true(LP_ENCODING_MODE_ASSUME_UTF8 == opts.encoding_mode, "Setting encoding-mode to assume-utf8 failed");
  assert_string(opts.encoding, "utf-8", "LogProtoServerOptions.encoding was not properly set");

  log_proto_server_options_init(&opts, configuration);
  success = log_proto_server_options_validate(&opts);
  assert_true(success, "Failed to set assume-utf8 encoding mode with utf8 encoding");

  log_proto_server_options_destroy(&opts);
}

static void
test_log_proto_server_options_encoding_mode_assume_utf8_no_encoding(void)
{
  LogProtoServerOptions opts;
  gboolean success;

  log_proto_server_options_defaults(&opts);
  log_proto_server_options_set_encoding_mode(&opts, LP_ENCODING_MODE_ASSUME_UTF8);
  assert_true(LP_ENCODING_MODE_ASSUME_UTF8 == opts.encoding_mode, "Setting encoding-mode to assume-utf8 failed");

  log_proto_server_options_init(&opts, configuration);
  success = log_proto_server_options_validate(&opts);
  assert_true(success, "Failed to set assume-utf8 encoding mode without encoding");

  log_proto_server_options_destroy(&opts);
}

static void
test_log_proto_server_options_encoding_mode_utf8_with_fallback_non_utf8_encoding(void)
{
  LogProtoServerOptions opts;
  gboolean success;
  log_proto_server_options_defaults(&opts);
  log_proto_server_options_set_encoding_mode(&opts, LP_ENCODING_MODE_UTF8_WITH_FALLBACK);
  log_proto_server_options_set_encoding(&opts, "UCS-2");

  assert_true(LP_ENCODING_MODE_UTF8_WITH_FALLBACK == opts.encoding_mode, "Setting encoding-mode to utf8-with-fallback failed");
  assert_string(opts.encoding, "UCS-2", "LogProtoServerOptions.encoding was not properly set");

  log_proto_server_options_init(&opts, configuration);
  start_grabbing_messages();
  success = log_proto_server_options_validate(&opts);
  assert_grabbed_messages_contain("Invalid use of explicit non UTF-8 encoding() with current encoding-mode(); encoding='UCS-2'", "message about encoding-mode(utf8-with-fallback) being invalid with non utf-8 encoding() missing");
  assert_false(success, "Succesfully set utf8-with-fallback encoding mode with a non-utf8 encoding");

  log_proto_server_options_destroy(&opts);
}

static void
test_log_proto_server_options_encoding_mode_utf8_with_fallback_utf8_encoding(void)
{
  LogProtoServerOptions opts;
  gboolean success;

  log_proto_server_options_defaults(&opts);
  log_proto_server_options_set_encoding_mode(&opts, LP_ENCODING_MODE_UTF8_WITH_FALLBACK);
  log_proto_server_options_set_encoding(&opts, "utf-8");
  assert_true(LP_ENCODING_MODE_UTF8_WITH_FALLBACK== opts.encoding_mode, "Setting encoding-mode to utf8-with-fallback failed");
  assert_string(opts.encoding, "utf-8", "LogProtoServerOptions.encoding was not properly set");

  log_proto_server_options_init(&opts, configuration);
  success = log_proto_server_options_validate(&opts);
  assert_true(success, "Failed to set utf8-with-fallback encoding mode with utf8 encoding");

  log_proto_server_options_destroy(&opts);
}

static void
test_log_proto_server_options_encoding_mode_utf8_with_fallback_no_encoding(void)
{
  LogProtoServerOptions opts;
  gboolean success;

  log_proto_server_options_defaults(&opts);
  log_proto_server_options_set_encoding_mode(&opts, LP_ENCODING_MODE_UTF8_WITH_FALLBACK);
  assert_true(LP_ENCODING_MODE_UTF8_WITH_FALLBACK == opts.encoding_mode, "Setting encoding-mode to utf8-with-fallback failed");

  log_proto_server_options_init(&opts, configuration);
  success = log_proto_server_options_validate(&opts);
  assert_true(success, "Failed to set utf8-with-fallback encoding mode without encoding");

  log_proto_server_options_destroy(&opts);
}

/* abstract LogProtoServer methods */
void
test_log_proto_server_options(void)
{
  PROTO_TESTCASE(test_log_proto_server_options_limits);
  PROTO_TESTCASE(test_log_proto_server_options_valid_encoding);
  PROTO_TESTCASE(test_log_proto_server_options_invalid_encoding);
  PROTO_TESTCASE(test_log_proto_server_options_encoding_mode_strict);
  PROTO_TESTCASE(test_log_proto_server_options_encoding_mode_8bit_clean_w_encoding);
  PROTO_TESTCASE(test_log_proto_server_options_encoding_mode_8bit_clean_wo_encoding);
  PROTO_TESTCASE(test_log_proto_server_options_encoding_mode_assume_utf8_non_utf8_encoding);
  PROTO_TESTCASE(test_log_proto_server_options_encoding_mode_assume_utf8_utf8_encoding);
  PROTO_TESTCASE(test_log_proto_server_options_encoding_mode_assume_utf8_no_encoding);
  PROTO_TESTCASE(test_log_proto_server_options_encoding_mode_utf8_with_fallback_non_utf8_encoding);
  PROTO_TESTCASE(test_log_proto_server_options_encoding_mode_utf8_with_fallback_utf8_encoding);
  PROTO_TESTCASE(test_log_proto_server_options_encoding_mode_utf8_with_fallback_no_encoding);
}
