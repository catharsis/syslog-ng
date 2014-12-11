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
test_log_proto_server_options_encoding_mode_8bit_clean(void)
{
  LogProtoServerOptions opts;
  gboolean success;

  log_proto_server_options_defaults(&opts);
  log_proto_server_options_set_encoding_mode(&opts, LP_ENCODING_MODE_8BIT_CLEAN);
  log_proto_server_options_set_encoding(&opts, "utf-8");
  assert_true(LP_ENCODING_MODE_8BIT_CLEAN == opts.encoding_mode, "Setting encoding-mode to strict failed");

  log_proto_server_options_init(&opts, configuration);
  start_grabbing_messages();
  success = log_proto_server_options_validate(&opts);
  assert_grabbed_messages_contain("Invalid use of encoding-mode(8bit-clean) with an explicit encoding() specified", "message about encoding-mode(8bit-clean) being invalid with explicit encoding specified missing");
  assert_false(success, "Successfully set 8bit-clean encoding mode, with an explicit encoding");

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
}
