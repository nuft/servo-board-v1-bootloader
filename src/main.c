#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

#include <can-bootloader/boot_arg.h>
#include "bootloader.h"
#include "debug.h"

void fault_handler(void)
{
    // debug("fault!\n", s);
    gpio_clear(GPIOC, GPIO13);
    gpio_set(GPIOC, GPIO15);
    reboot(BOOT_ARG_START_BOOTLOADER_NO_TIMEOUT);
}

int bootloader_main(int arg)
{
    rcc_clock_setup_hsi(&hsi_8mhz[CLOCK_64MHZ]);

    // debug_init();

    // debug("bootloader\n");
    delay(10000000);

    bootloader(arg);

    reboot(BOOT_ARG_START_BOOTLOADER);

    return 0;
}
