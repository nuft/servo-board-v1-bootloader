#include <libopencm3/stm32/can.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>

int can_interface_init(void)
{
    rcc_periph_clock_enable(RCC_CAN);
    rcc_periph_clock_enable(RCC_GPIOA);

    gpio_mode_setup(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO8);
    gpio_set_output_options(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_100MHZ, GPIO8);
    gpio_clear(GPIOA, GPIO8);

    gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO11 | GPIO12);
    gpio_set_output_options(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_100MHZ, GPIO11 | GPIO12);
    gpio_set_af(GPIOA, GPIO_AF9, GPIO11 | GPIO12);

    /*
    STM32F3 CAN on 32MHz configured APB1 peripheral clock
    32MHz / 2 -> 16MHz
    16MHz / (1tq + 9tq + 6tq) = 1MHz
    */
    if (can_init(CAN,          // Interface
             false,             // Time triggered communication mode.
             true,              // Automatic bus-off management.
             false,             // Automatic wakeup mode.
             false,             // No automatic retransmission.
             false,             // Receive FIFO locked mode.
             true,              // Transmit FIFO priority.
             CAN_BTR_SJW_1TQ,   // Resynchronization time quanta jump width
             CAN_BTR_TS1_9TQ,   // Time segment 1 time quanta width
             CAN_BTR_TS2_6TQ,   // Time segment 2 time quanta width
             2,                 // Prescaler
             false,             // Loopback
             false)) {          // Silent)
        return 1;
    }

    // filter to match any std id
    // mask bits: 0 -> Don't care, 1 -> mute match corresponding id bit
    can_filter_id_mask_32bit_init(
        CAN,
        0,      // filter nr
        0,      // id: only std id, no rtr
        6,      // mask: macth any std id
        0,      // assign to fifo0
        true    // enable
    );

    return 0;
}

int can_interface_read_message(uint32_t *id, uint8_t *message)
{
    uint32_t fid;
    uint8_t len;
    bool ext, rtr;

    while((CAN_RF0R(CAN) & CAN_RF0R_FMP0_MASK) == 0);

    can_receive(
        CAN,        // canport
        0,          // fifo
        true,       // release
        id,         // can id
        &ext,       // extended id
        &rtr,       // transmission request
        &fid,       // filter id
        &len,       // length
        message
    );
    return len;
}

void can_interface_write_message(uint32_t id, uint8_t *message, uint8_t length)
{
    can_transmit(
        CAN,        // canport
        id,         // can id
        false,      // extended id
        false,      // request transmit
        length,     // data leng
        message     // data
    );
    while((CAN_TSR(CAN) & CAN_TSR_RQCP0) == 0);

    // if ((CAN_TSR(CAN) & CAN_TSR_TXOK0)) {
    //     // can ok
    // } else {
    //     // error
    // }
    // if ((CAN_TSR(CAN) & CAN_TSR_TERR0)) {
    //     // transmit error
    // }
    // if ((CAN_TSR(CAN) & CAN_TSR_ALST0)) {
    //     // arbitration lost
    // }
}

