/* red_test.c — round-trip test harness for red_decompress.c
 *
 * Usage: red_test <pairs_dir>
 * Iterates each *.payload file, decompresses via red_decompress_wcsc_lha(),
 * compares against corresponding .oracle file.
 *
 * Build: gcc -Wall -O2 -o red_test red_test.c red_decompress.c
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

/* Forward decls from red_decompress.c */
int red_extract_all(const char *red_path, const char *out_dir);

int main(int argc, char **argv)
{
    if (argc < 3) {
        fprintf(stderr, "usage: %s <archive.RED> <out_dir>\n", argv[0]);
        return 1;
    }
    return red_extract_all(argv[1], argv[2]);
}
