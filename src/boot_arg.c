#include <libopencm3/cm3/scb.h>
#include "boot_arg.h"

// defined by linker
extern int ram_start;

void reboot(int boot_arg)
{
    ram_start = boot_arg;

    union {
        uint32_t word;
        uint8_t byte[4];
    } crc;

    crc.word = crc32(0xffffffff, &ram_start, sizeof(int));
    uint8_t *p = (uint8_t *)&ram_start + sizeof(int);

    p[0] = crc.byte[0];
    p[1] = crc.byte[1];
    p[2] = crc.byte[2];
    p[3] = crc.byte[3];

    scb_reset_system();
}

int bootarg(void)
{
    int arg;
    union {
        uint32_t word;
        uint8_t byte[4];
    } crc;

    uint8_t *p = (uint8_t *)&ram_start + sizeof(int);

    crc.byte[0] = p[0];
    crc.byte[1] = p[1];
    crc.byte[2] = p[2];
    crc.byte[3] = p[3];

    if (crc.word != crc32(0xffffffff, &ram_start, sizeof(int))) {
        return -1;
    }

    if (arg != BOOT_ARG_APP_START && arg != BOOT_ARG_APP_UPDATE
        && arg != BOOT_ARG_SAFE_MODE ) {
        return -1;
    }

    return arg;
}
