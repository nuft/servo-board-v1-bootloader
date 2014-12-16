#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>
#include <kprintf/kprintf.h>

void delay(int i)
{
    while (i--) {
        __asm__ volatile ("nop": /*out*/: /*in*/: /*clobber*/);
    }
}

void debug_init(void)
{
    // LEDs
    rcc_periph_clock_enable(RCC_GPIOC);
    gpio_mode_setup(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO13 | GPIO14 | GPIO15);
    gpio_set_output_options(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_100MHZ, GPIO13 | GPIO14 | GPIO15);

    // HEARTBEAT
    gpio_set(GPIOC, GPIO13);


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

int debug(const char *fmt, ...)
{
    int ret;
    va_list args;
    va_start(args, fmt);
    ret = vfkprintf(uart1_write, fmt, args);
    va_end(args);
    return ret;
}
