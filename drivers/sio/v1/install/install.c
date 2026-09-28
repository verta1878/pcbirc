/* ====================================================================
 * INSTALL.EXE — SIO Installation Utility for OS/2
 * ====================================================================
 * Copies SIO.SYS, VSIO.SYS, VX00.SYS to the OS/2 system directory
 * and updates CONFIG.SYS with the appropriate DEVICE= lines.
 * ====================================================================
 */

#define INCL_DOS
#define INCL_DOSERRORS
#include <os2.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CFG_BACKUP  "CONFIG.BAK"
#define MAX_CFG     32768

static int  CopyFile(const char *src, const char *dst);
static int  UpdateConfig(const char *bootDrive);
static void DetectBootDrive(char *drive);

int main(int argc, char *argv[])
{
    char bootDrive[4] = "C:";
    char sysDir[260];
    char srcDir[260];
    int  rc;

    printf("SIO Installation Utility v1.00\n");
    printf("==============================\n\n");

    /* Determine boot drive */
    DetectBootDrive(bootDrive);
    printf("Boot drive: %s\n", bootDrive);

    /* Source = current directory */
    DosQueryPathInfo(".", FIL_QUERYFULLNAME, srcDir, sizeof(srcDir));
    printf("Source: %s\n", srcDir);

    /* Target = boot:\OS2 */
    sprintf(sysDir, "%s\\OS2", bootDrive);
    printf("Target: %s\n\n", sysDir);

    /* Confirm */
    printf("This will:\n");
    printf("  1. Copy SIO.SYS, VSIO.SYS, VX00.SYS to %s\n", sysDir);
    printf("  2. Copy VMODEM.EXE, SU.EXE, PMLM.EXE to %s\n", sysDir);
    printf("  3. Update %s\\CONFIG.SYS\n", bootDrive);
    printf("  4. Backup CONFIG.SYS as %s\n\n", CFG_BACKUP);
    printf("Continue? (Y/N) ");

    {
        int ch = getchar();
        if (ch != 'Y' && ch != 'y') {
            printf("\nInstallation cancelled.\n");
            return 1;
        }
    }
    printf("\n");

    /* Copy driver files */
    {
        const char *files[] = {
            "SIO.SYS", "VSIO.SYS", "VX00.SYS",
            "VMODEM.EXE", "SU.EXE", "PMLM.EXE", "VIEWPMLM.EXE",
            NULL
        };
        int i;
        for (i = 0; files[i]; i++) {
            char src[260], dst[260];
            sprintf(src, "%s\\%s", srcDir, files[i]);
            sprintf(dst, "%s\\%s", sysDir, files[i]);
            printf("  Copying %s... ", files[i]);
            rc = CopyFile(src, dst);
            if (rc == 0) {
                printf("OK\n");
            } else {
                printf("SKIPPED (file not found)\n");
            }
        }
    }

    /* Update CONFIG.SYS */
    printf("\nUpdating CONFIG.SYS...\n");
    rc = UpdateConfig(bootDrive);
    if (rc == 0) {
        printf("CONFIG.SYS updated successfully.\n");
    } else {
        printf("Warning: CONFIG.SYS not modified (rc=%d).\n", rc);
        printf("You may need to add DEVICE= lines manually.\n");
    }

    printf("\nInstallation complete. Reboot to activate SIO.\n");
    printf("\nRecommended CONFIG.SYS lines:\n");
    printf("  DEVICE=%s\\SIO2K.SYS\n", sysDir);
    printf("  DEVICE=%s\\UART.SYS\n", sysDir);
    printf("  DEVICE=%s\\VSIO2K.SYS\n", sysDir);

    return 0;
}

static int CopyFile(const char *src, const char *dst)
{
    APIRET rc;
    rc = DosCopy(src, dst, DCPY_EXISTING);
    return (int)rc;
}

static int UpdateConfig(const char *bootDrive)
{
    char    cfgPath[260], bakPath[260];
    FILE   *f;
    char   *buf;
    long    len;
    char    sioLine[260], vsioLine[260];
    char    sysDir[260];

    sprintf(cfgPath, "%s\\CONFIG.SYS", bootDrive);
    sprintf(bakPath, "%s\\%s", bootDrive, CFG_BACKUP);
    sprintf(sysDir,  "%s\\OS2", bootDrive);

    /* Read current CONFIG.SYS */
    f = fopen(cfgPath, "rb");
    if (!f) return -1;

    fseek(f, 0, SEEK_END);
    len = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (len > MAX_CFG) { fclose(f); return -2; }

    buf = (char *)malloc(len + 512);
    if (!buf) { fclose(f); return -3; }

    fread(buf, 1, len, f);
    buf[len] = '\0';
    fclose(f);

    /* Check if SIO.SYS is already in CONFIG.SYS */
    if (strstr(buf, "SIO2K.SYS") || strstr(buf, "sio2k.sys")) {
        printf("  SIO.SYS already in CONFIG.SYS — not modifying.\n");
        free(buf);
        return 0;
    }

    /* Backup */
    DosCopy(cfgPath, bakPath, DCPY_EXISTING);

    /* Append DEVICE= lines */
    sprintf(sioLine,  "DEVICE=%s\\SIO2K.SYS\r\n", sysDir);
    sprintf(vsioLine, "DEVICE=%s\\UART.SYS\r\nDEVICE=%s\\VSIO2K.SYS\r\n", sysDir, sysDir);

    strcat(buf, "\r\nREM --- SIO2K Serial I/O Driver ---\r\n");
    strcat(buf, sioLine);
    strcat(buf, vsioLine);

    /* Write updated CONFIG.SYS */
    f = fopen(cfgPath, "wb");
    if (!f) { free(buf); return -4; }
    fwrite(buf, 1, strlen(buf), f);
    fclose(f);
    free(buf);

    return 0;
}

static void DetectBootDrive(char *drive)
{
    ULONG   bootDrive;
    APIRET  rc;

    rc = DosQuerySysInfo(QSV_BOOT_DRIVE, QSV_BOOT_DRIVE,
                         &bootDrive, sizeof(bootDrive));
    if (rc == 0) {
        drive[0] = (char)('A' + bootDrive - 1);
        drive[1] = ':';
        drive[2] = '\0';
    } else {
        strcpy(drive, "C:");
    }
}
