#ifndef CHAT_PROTO_H
#define CHAT_PROTO_H

// ------------------------------------------------------------
// Chat Server — Protocol (framing + commands)
// Defines the line-based wire format and command vocabulary.
// Parsing/formatting implementations live in framing.c / proto.c
// ------------------------------------------------------------

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "config.h"  // for NAME_LEN, MAX_MSG_LEN, MAX_WIRE_LINE

// ---------- Framing rules ----------
// INPUT: accept lines ending with '\n' or "\r\n". Trim trailing '\r'.
// OUTPUT: always terminate with '\n'.

// ---------- Message kinds on the wire ----------
typedef enum {
    MSG_CHAT = 0,     // regular chat line from a user
    MSG_COMMAND,      // a slash command line (parsed into a command)
    MSG_SYSTEM,       // server-generated informational (join/leave, errors)
    MSG_PRIVATE       // server-delivered private message (/whisper)
} msg_kind_t;

// ---------- Supported commands ----------
typedef enum {
    CMD_INVALID = 0,
    CMD_NICK,        // /nick <name>
    CMD_QUIT,        // /quit
    CMD_ME,          // /me <action>
    CMD_WHISPER      // /whisper <target> <text>  (alias: /w)
} cmd_t;

// ---------- Parsed command payload ----------
typedef struct {
    cmd_t  type;
    // For /nick: arg1 = new name
    // For /me:   rest = action text
    // For /whisper: arg1 = target name, rest = message text
    char   arg1[NAME_LEN];                 // name-like token
    char   rest[MAX_MSG_LEN];              // remaining text (may be empty)
} command_t;

// ---------- A normalized message line ----------
typedef struct {
    msg_kind_t kind;
    // Present for MSG_CHAT, MSG_PRIVATE, MSG_SYSTEM (formatted text without trailing newline).
    // For MSG_COMMAND, 'text' holds the original line without trailing newline;
    // the parsed form is in 'cmd'.
    char       text[MAX_WIRE_LINE];
    command_t  cmd;    // valid iff kind == MSG_COMMAND
} wire_msg_t;

// ======================================================================
// Validation helpers (header-only declarations; impl in proto.c)
//
// NOTE: These functions do NOT do I/O; they only interpret/format buffers.
// All return values use the usual C conventions:
//   - true/false for predicates
//   - >=0 for lengths written, -1 on error
// ======================================================================

// Returns true if all chars are printable (no control chars) and not leading/trailing space.
// Length must be 1..NAME_LEN-1.
bool proto_validate_name(const char *name);

// Trim trailing CR and LF in-place; returns new length (>=0).
int  proto_chomp_crlf(char *buf, int len);

// Parse a raw input line (no trailing newline) into a wire_msg_t.
// - Detects commands (leading '/'), fills 'cmd' if applicable.
// - Truncates safely to fit buffers.
// Returns 0 on success, -1 on parse error (e.g., bad /nick syntax).
int  proto_parse_line(const char *line, wire_msg_t *out);

// Convenience: detect if a line is a command without full parse.
bool proto_is_command(const char *line);

// Formatters (write into 'dst', ensure one trailing '\n' added, never overflow).
// Return number of bytes written (excluding terminating '\0' since this is for sockets),
// or -1 if the buffer is too small.

// Format regular chat: "name: message\n"
int  proto_fmt_chat(char *dst, size_t dstsz, const char *name, const char *message);

// Format /me emote: "* name action\n"
int  proto_fmt_emote(char *dst, size_t dstsz, const char *name, const char *action);

// Format system info: "* text\n"
int  proto_fmt_system(char *dst, size_t dstsz, const char *text);

// Format private message to recipient: "[from->to] message\n"
int  proto_fmt_private_to_recipient(char *dst, size_t dstsz,
                                    const char *from, const char *to, const char *message);

// Format private confirm to sender: "[to @to] message\n"
int  proto_fmt_private_to_sender(char *dst, size_t dstsz,
                                 const char *to, const char *message);

// ---------- Command parsing helpers ----------
// Extract command token and split args.
// e.g., "/nick Alice" => CMD_NICK, arg1="Alice"
//       "/w Bob hello there" => CMD_WHISPER, arg1="Bob", rest="hello there"
cmd_t proto_cmd_from_line(const char *line, command_t *out);

// ---------- Constants for UI styling (clients may use) ----------
#define SYS_PREFIX         "* "
#define CHAT_SEP           ": "
#define PRIV_OPEN          "["
#define PRIV_ARROW         "->"
#define PRIV_CLOSE         "] "

// ---------- Error messages (server -> client) ----------
#define ERR_BAD_CMD        "* error: unknown command\n"
#define ERR_BAD_NICK       "* error: invalid nickname\n"
#define ERR_NICK_IN_USE    "* error: nickname already in use\n"
#define ERR_WHISPER_ARGS   "* error: usage: /whisper <name> <message>\n"

#endif // CHAT_PROTO_H
