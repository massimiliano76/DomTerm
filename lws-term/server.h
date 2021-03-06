#ifdef HAVE_LWS_CONFIG_H
#include "lws_config.h"
#endif

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <getopt.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <sys/queue.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <assert.h>

#ifdef __APPLE__
#include <util.h>
#elif defined(__FreeBSD__)
#include <libutil.h>
#else
#include <pty.h>
#endif

#include <libwebsockets.h>
#include <json.h>

#include "utils.h"

#define SERVER_KEY_LENGTH 20
extern char server_key[SERVER_KEY_LENGTH];
extern char *main_html_url;
extern char *main_html_path;
extern volatile bool force_exit;
extern struct lws_context *context;
extern struct tty_server *server;
extern struct lws_vhost *vhost;
//extern struct tty_client *focused_client;
extern struct lws_context_creation_info info; // FIXME rename
extern struct lws *focused_wsi;
extern struct cmd_client *cclient;
extern int last_session_number;
extern const char *(default_argv[]);

struct pty_data {
    char *data;
    int len;
    STAILQ_ENTRY(pty_data) list;
};

/** Data specific to a pty process. */
struct pty_client {
    struct pty_client *next_pty_client;
    int pid;
    int pty;
    int session_number;
    char *session_name;
    int nrows, ncols;
    float pixh, pixw;
    int eof_seen;  // 1 means seen; 2 reported to client
    bool exit;
    bool detached;
    int paused;
    struct lws *first_client_wsi;
    struct lws **last_client_wsi_ptr;
    char *obuffer; // output from child process
    size_t olen; // used length of obuffer
    size_t osize; // allocated size of obuffer
    // both sent_count and confirmed_count are modulo MASK28.
    long sent_count; // # bytes send to (any) tty_client
    long confirmed_count; // # bytes confirmed received from (some) tty_client
    struct lws *pty_wsi;
};

/** Data specific to a (browser) client connection. */
struct tty_client {
    struct pty_client *pclient;
    bool initialized;
    //bool pty_started; = pclient!=NULL
    bool authenticated;
    char hostname[100];
    char address[50];
    char *version_info; // received from client
    size_t osent; // index in obuffer
    //   The range in pclient->obuffer between osent and pclient->olen
    //   has not yet been sent to this client.
    struct lws *wsi;
    // data received from client and not yet processed.
    // (Normally, this is only if an incomplete reportEvent message.)
    char *buffer;
    size_t len; // length of data in buffer
    struct lws *next_client_wsi;
};

struct cmd_client {
    int socket;
};
#define MASK28 0xfffffff

struct tty_server {
    LIST_HEAD(client, tty_client) clients;    // client list
    int client_count;                         // client count FIXME remove?
    int session_count;                        // client count
    char *prefs_json;                         // client preferences
    char *credential;                         // encoded basic auth credential
    int reconnect;                            // reconnect timeout
    char *index;                              // custom index.html
    char **argv;                              // command with arguments
    int sig_code;                             // close signal
    char *sig_name;                           // human readable signal string
    bool readonly;                            // whether not allow clients to write to the TTY
    bool check_origin;                        // whether allow websocket connection from different origin
    bool once;                                // whether accept only one client and exit on disconnection
    bool client_can_close;
    char *socket_path;                        // UNIX domain socket path
    pthread_mutex_t lock;
};

extern int
callback_http(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len);

extern void
initialize_resource_map(struct lws_context *, const char*);

extern int
callback_tty(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len);

extern int
callback_pty(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len);

extern int
callback_cmd(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len);

extern char *get_resource_path();
extern int get_executable_directory_length();
extern char *get_bin_relative_path(const char* app_path);
extern char* get_executable_path();
extern char *get_bin_relative_path(const char* app_path);
extern void handle_command(int argc, char**argv, const char*cwd,
                           char **env, struct lws *wsi, int replyfd);
extern void do_run_browser(const char *specifier, char *url, int port);
extern char* check_browser_specifier(const char *specifier);
extern void fatal(const char *format, ...);
extern const char *find_home(void);
extern char ** copy_argv(int argc, char * const*argv);

extern int probe_domterm(void);
extern int check_domterm(void);

#if COMPILED_IN_RESOURCES
struct resource {
  char *name;
  unsigned char *data;
  unsigned int length;
};
extern struct resource resources[];
#endif

#define FOREACH_WSCLIENT(VAR, PCLIENT)      \
  for (VAR = (PCLIENT)->first_client_wsi; VAR != NULL; \
       VAR = ((struct tty_client *) lws_wsi_user(VAR))->next_client_wsi)

extern bool force_option;
