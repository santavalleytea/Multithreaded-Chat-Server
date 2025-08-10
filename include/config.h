#ifndef CHAT_CONFIG_H
#define CHAT_CONFIG_H

#include <stddef.h>
#include <stdint.h>

// ---------- Networking defaults ----------
#define DEFAULT_PORT        5555     // used if no port provided via CLI
#define LISTEN_BACKLOG      64       // pending connection queue length for listen()

// ---------- Limits & sizing ----------
#define MAX_CLIENTS         128

// I/O buffer for socket reads/writes.
#define BUF_SIZE            4096

// Maximum length for a username (excluding terminating NUL).
#define NAME_LEN            32

// Maximum length of a single logical message payload (not including any protocol framing).
#define MAX_MSG_LEN         1024

// ---------- Timeouts & intervals ----------
#define READ_TIMEOUT_SEC    0        // 0 => disabled
#define PING_INTERVAL_SEC   0        // 0 => disabled

// ---------- Features / toggles ----------
#define ENABLE_JOIN_LEAVE_MSGS   1   // announce "* user joined/left"
#define ENABLE_COLORS            0   // ANSI color output for client UI (0/1)

// ---------- Logging & diagnostics ----------
#ifndef NDEBUG
  #define DEBUG_LOGS 1
#else
  #define DEBUG_LOGS 0
#endif

// Helper macro for compile-time array sizing.
#ifndef COUNT_OF
  #define COUNT_OF(arr) (sizeof(arr) / sizeof((arr)[0]))
#endif

// ---------- Safety checks (compile-time) ----------
#if (NAME_LEN < 3)
  #error "NAME_LEN should be at least 3 characters."
#endif

#if (MAX_MSG_LEN >= BUF_SIZE)
  #error "MAX_MSG_LEN must be strictly less than BUF_SIZE to allow headers/prefix."
#endif

#if (MAX_CLIENTS <= 0)
  #error "MAX_CLIENTS must be > 0."
#endif

// ---------- Derivable constants ----------
#define MAX_WIRE_LINE  (NAME_LEN + 2 /*": "*/ + MAX_MSG_LEN + 2 /*"\r\n" or "\n"*/)

// ANSI escape codes (enabled only if ENABLE_COLORS)
#if ENABLE_COLORS
  #define ANSI_RESET  "\x1b[0m"
  #define ANSI_DIM    "\x1b[2m"
  #define ANSI_GREEN  "\x1b[32m"
  #define ANSI_YELLOW "\x1b[33m"
  #define ANSI_CYAN   "\x1b[36m"
#else
  #define ANSI_RESET  ""
  #define ANSI_DIM    ""
  #define ANSI_GREEN  ""
  #define ANSI_YELLOW ""
  #define ANSI_CYAN   ""
#endif

#endif // CHAT_CONFIG_H
