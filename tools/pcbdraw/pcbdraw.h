/* ====================================================================
 * pcbdraw.h — PCBoard ANSI Art Viewer/Editor Core Types
 * ====================================================================
 * C port of sysop/0's PabloDraw Pascal port (pdtypes.pas).
 * Original PabloDraw by Curtis Wensley (MIT License).
 * Pascal port by sysop/0 (pcbirc crew).
 * C port for OpenWatcom by pcbirc crew (GPLv3).
 *
 * Canvas/attribute/palette types shared by all pcbdraw modules.
 * ==================================================================== */

#ifndef PCBDRAW_H
#define PCBDRAW_H

/* ---- Character cell ---- */
typedef struct {
    short ch;               /* CP437 code point (or Unicode)         */
} PDCharacter;

/* ---- Text attribute ---- */
typedef struct {
    unsigned char fg;       /* foreground (0-15)                     */
    unsigned char bg;       /* background (0-15)                     */
} PDAttribute;

#define ATTR_BYTE(a)        (((a).fg & 0x0F) | (((a).bg & 0x0F) << 4))
#define ATTR_INIT(a, b)     do { (a).fg = (b) & 0x0F; (a).bg = ((b) >> 4) & 0x0F; } while(0)
#define ATTR_BOLD(a)        (((a).fg & 0x08) != 0)
#define ATTR_BLINK(a)       (((a).bg & 0x08) != 0)
#define ATTR_FG_ONLY(a)     ((a).fg & 0x07)
#define ATTR_BG_ONLY(a)     ((a).bg & 0x07)

/* ---- Canvas element ---- */
typedef struct {
    PDCharacter ch;
    PDAttribute attr;
} PDCanvasElement;

/* ---- Canvas (2D grid) ---- */
typedef struct {
    int width;
    int height;
    PDCanvasElement *data;  /* width * height elements               */
} PDCanvas;

/* Canvas API */
PDCanvas *canvas_create(int width, int height);
void      canvas_free(PDCanvas *c);
void      canvas_resize(PDCanvas *c, int width, int height);
void      canvas_clear(PDCanvas *c);
void      canvas_fill(PDCanvas *c, short ch, unsigned char attr);
void      canvas_scroll_up(PDCanvas *c, int lines);
void      canvas_trim_height(PDCanvas *c, int new_height);

PDCanvasElement canvas_get(PDCanvas *c, int x, int y);
void            canvas_set(PDCanvas *c, int x, int y, PDCanvasElement e);

/* ---- RGB color ---- */
typedef struct {
    unsigned char r, g, b;
} PDColor;

/* Standard EGA/VGA 16-color palette */
extern const PDColor default_palette[16];

/* ---- SAUCE metadata (FTS) ---- */
typedef struct {
    char     id[5];         /* "SAUCE"                               */
    char     version[2];    /* "00"                                  */
    char     title[35];
    char     author[20];
    char     group[20];
    char     date[8];
    long     filesize;
    unsigned char datatype;
    unsigned char filetype;
    unsigned short tinfo1;  /* width                                 */
    unsigned short tinfo2;  /* height                                */
    unsigned short tinfo3;
    unsigned short tinfo4;
    unsigned char comments;
    unsigned char tflags;
    char     tinfos[22];
} SauceRecord;

int  sauce_load(const char *filename, SauceRecord *sauce);
int  sauce_get_width(const SauceRecord *s);
int  sauce_get_height(const SauceRecord *s);
int  sauce_get_ice(const SauceRecord *s);

/* ---- ANSI parser ---- */
typedef struct {
    int cur_x, cur_y;
    int save_x, save_y;
    PDAttribute attr;
    PDCanvas *canvas;
    int clip_left, clip_top, clip_right, clip_bottom;
    int line_wrap;
    int ice_colors;
    int ice_detected;
    int last_line_data;
} AnsiParser;

void ansi_init(AnsiParser *p);
int  ansi_load_file(AnsiParser *p, const char *filename, PDCanvas *canvas);
int  ansi_load_buffer(AnsiParser *p, const unsigned char *buf, long len,
                      PDCanvas *canvas);
int  ansi_get_final_y(AnsiParser *p);

/* ---- PCBoard @X color code parser ---- */
int  pcboard_load_file(const char *filename, PDCanvas *canvas);

/* ---- Binary (raw char+attr pairs) ---- */
int  binary_load_file(const char *filename, PDCanvas *canvas, int width);

/* ---- SAUCE color map (ANSI SGR → DOS) ---- */
extern const unsigned char ansi_color_map[8];

/* ---- Network protocol (teleconference) ---- */

#define PD_NET_PORT     3693
#define PD_MAX_USERS    32
#define PD_MAX_MSG      65000
#define PD_RECV_BUF     8192
#define PD_NET_VERSION  1

/* Wire commands */
#define CMD_CHAT        0x01
#define CMD_UPDATE      0x02
#define CMD_LOADDOC     0x03
#define CMD_USERLIST    0x04
#define CMD_USERSTATUS  0x05
#define CMD_CURSOR      0x06
#define CMD_SETATTR     0x07
#define CMD_KICK        0x08
#define CMD_AUTH        0x09
#define CMD_WELCOME     0x0A
#define CMD_BYE         0x0B

typedef enum {
    LEVEL_VIEWER   = 0,
    LEVEL_EDITOR   = 1,
    LEVEL_OPERATOR = 2
} UserLevel;

typedef struct {
    char          alias[32];
    UserLevel     level;
    unsigned short cursor_x;
    unsigned short cursor_y;
    int           socket;
    int           active;
    unsigned char recv_buf[PD_RECV_BUF];
    int           recv_len;
} PDNetUser;

typedef struct {
    unsigned long len;
    unsigned char cmd;
    unsigned char data[PD_MAX_MSG];
} PDNetMsg;

/* Server */
typedef struct {
    PDNetUser     users[PD_MAX_USERS];
    PDCanvas     *canvas;
    unsigned short port;
    int           listen_sock;
    int           running;
    char          password[64];
    UserLevel     default_level;
} PDServer;

int  pd_server_start(PDServer *srv, PDCanvas *canvas, unsigned short port);
void pd_server_stop(PDServer *srv);
void pd_server_poll(PDServer *srv);
void pd_server_kick(PDServer *srv, int idx, const char *reason);
void pd_server_broadcast_chat(PDServer *srv, const char *from, const char *text);

/* Client */
typedef struct {
    int           socket;
    PDCanvas     *canvas;
    char          alias[32];
    UserLevel     level;
    int           my_index;
    int           connected;
    unsigned char recv_buf[PD_RECV_BUF];
    int           recv_len;
    PDNetUser     users[PD_MAX_USERS];
    int           user_count;
} PDClient;

int  pd_client_connect(PDClient *cli, PDCanvas *canvas,
                       const char *host, unsigned short port,
                       const char *alias, const char *pass);
void pd_client_disconnect(PDClient *cli);
void pd_client_poll(PDClient *cli);
void pd_client_send_chat(PDClient *cli, const char *text);
void pd_client_send_update(PDClient *cli, int x1, int y1, int x2, int y2);
void pd_client_send_cursor(PDClient *cli, int x, int y);

/* Message pack/unpack helpers */
void msg_pack_word(unsigned char *d, int *p, unsigned short w);
unsigned short msg_unpack_word(const unsigned char *d, int *p);
void msg_pack_str(unsigned char *d, int *p, const char *s);
int  msg_unpack_str(const unsigned char *d, int *p, char *out, int maxlen);

#endif /* PCBDRAW_H */
