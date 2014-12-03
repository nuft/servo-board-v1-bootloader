#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/cm3/scb.h>
#include <kprintf/kprintf.h>
#include "can_interface.h"

int uart1_write(const char *buf, int len);

void uart1_init(void)
{
    rcc_periph_clock_enable(RCC_GPIOA);
    // uart tx pin
    gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO9);
    gpio_set_output_options(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_100MHZ, GPIO10);
    gpio_set_af(GPIOA, GPIO_AF7, GPIO9);
    // uart rx pin
    gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO10);
    gpio_set_output_options(GPIOA, GPIO_OTYPE_OD, GPIO_OSPEED_100MHZ, GPIO10);
    gpio_set_af(GPIOA, GPIO_AF7, GPIO10);

    rcc_periph_clock_enable(RCC_USART1);
    usart_set_baudrate(USART1, 115200);
    usart_set_databits(USART1, 8);
    usart_set_stopbits(USART1, USART_STOPBITS_1);
    usart_set_mode(USART1, USART_MODE_TX_RX);
    usart_set_parity(USART1, USART_PARITY_NONE);
    usart_set_flow_control(USART1, USART_FLOWCONTROL_NONE);
    usart_enable(USART1);
}

int uart1_write(const char *buf, int len)
{
    int i;
    for (i = 0; i < len; i++) {
        usart_send_blocking(USART1, buf[i]);
    }
    return len;
}

void relocate_vector_table(void *ptr)
{
    // Vector table address is aligned to 32 words, different addresses are rounded down to a 32 word alignment.
    SCB_VTOR = (uint32_t)ptr;
    __asm__ volatile ("dsb");
    // vector table is now at new address
}

void panic(const char *s)
{
    kprintf("panic: %s\n", s);
    gpio_clear(GPIOC, GPIO13);
    gpio_set(GPIOC, GPIO15);
    while(1);
}

void delay(int x)
{
    while (x--)
        __asm__ volatile("nop");
}

int main(void)
{
    rcc_clock_setup_hsi(&hsi_8mhz[CLOCK_64MHZ]);

    // LEDs
    rcc_periph_clock_enable(RCC_GPIOC);
    gpio_mode_setup(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO13 | GPIO14 | GPIO15);
    gpio_set_output_options(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_100MHZ, GPIO13 | GPIO14 | GPIO15);

    // HEARTBEAT
    gpio_set(GPIOC, GPIO13);

    uart1_init();
    kprintf_stdout = uart1_write;
    kprintf("bootloader\n");

    kprintf("can init\n");
    can_interface_init();

    while(1) {
        union {
            uint8_t u8[8];
            uint32_t u32[2];
        } data;
        uint32_t id;
        int len = can_interface_receive(&id, &data.u8[0]);
        kprintf("can: %u, %d, %x %x\n", id, len, data.u32[0], data.u32[1]);
        if (len != 0) {
            can_interface_send(id, data.u8, len);
        }
        delay(5000000);
    }
    return 0;
}

void nmi_handler(void)
{
    panic("NMI");
    while(1);
}

void hard_fault_handler(void)
{
    panic("Hard Fault");
    while(1);
}

void mem_manage_handler(void)
{
    panic("Mem Manage");
    while(1);
}

void bus_fault_handler(void)
{
    panic("Bus Fault");
    while(1);
}

void usage_fault_handler(void)
{
    panic("Usage Fault");
    while(1);
}

