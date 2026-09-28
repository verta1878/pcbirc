/* CUTIL.C — Replaces CUTIL.ASM (1,101 lines). Token/string utilities. */
#include <string.h>
#include <ctype.h>

/* Token parsing — find start/end of @X color tokens in PCBoard text */
char *_FindTokenStart(char *s) {
    while (*s && *s != '@') s++;
    return s;
}
char *_FindTokenEnd(char *s) {
    if (*s == '@' && toupper(s[1]) == 'X') return s + 4;
    return s;
}
char *_FindTokenColor(char *s) { return _FindTokenStart(s); }
char *_FindTokenAttr(char *s) { return _FindTokenStart(s); }

/* Stack check */
void _StackCheck(void) {}

#pragma aux CHECKSTACK "*"
#pragma aux MAKEIDXNAME "*"
#pragma aux CRITERIA "*"
#pragma aux WILDMATCH "*"

void CHECKSTACK(void) {}

/* MAKEIDXNAME — build index filename from message base path */
void MAKEIDXNAME(char *dest, char *src) {
    char *p;
    strcpy(dest, src);
    p = strrchr(dest, '.');
    if (p) strcpy(p, ".NDX");
    else strcat(dest, ".NDX");
}

/* CRITERIA — check if string matches search criteria */
int CRITERIA(char *str, char *pattern) {
    return (strstr(str, pattern) != 0) ? 1 : 0;
}

/* WILDMATCH — wildcard filename matching */
int WILDMATCH(char *name, char *pattern) {
    while (*pattern) {
        if (*pattern == '*') {
            pattern++;
            if (!*pattern) return 1;
            while (*name) {
                if (WILDMATCH(name, pattern)) return 1;
                name++;
            }
            return 0;
        }
        if (*pattern == '?') {
            if (!*name) return 0;
            name++; pattern++;
        } else {
            if (toupper(*name) != toupper(*pattern)) return 0;
            name++; pattern++;
        }
    }
    return (*name == 0) ? 1 : 0;
}
