#include <stdint.h>
#include <string.h>

#include <can-bootloader/command.h>
#include <can-bootloader/can_datagram.h>
#include <can-bootloader/config.h>
#include <can-bootloader/can_interface.h>
#include <can-bootloader/boot_arg.h>

#include "can_interface.h"

#define PAGE_SIZE   2048
#define START_OF_DATAGRAM_MASK  (1<<7)

extern int config_page1, config_page2;   // defined by linker

command_t commands[] = {
    {.index = 1, .callback = command_jump_to_application},
    {.index = 2, .callback = command_crc_region},
    {.index = 3, .callback = command_write_flash},
    {.index = 6, .callback = command_config_update}
};

static void return_datagram(uint8_t source_id, uint8_t dest_id, uint8_t *data, int len)
{
    can_datagram_t out_dt;
    uint8_t dest_nodes[] = {dest_id};
    uint8_t buf[8];

    can_datagram_init(&out_dt);
    out_dt.destination_nodes = &dest_nodes[0];
    out_dt.destination_nodes_len = 1;
    out_dt.data = data;
    out_dt.data_len = len;
    out_dt.crc = can_datagram_compute_crc(&out_dt);

    bool start_of_datagram = true;
    while (true) {
        uint8_t dlc = can_datagram_output_bytes(&out_dt, (char *)buf, sizeof(buf));

        if (dlc == 0) {
            break;
        }

        if (start_of_datagram) {
            can_interface_write_message(source_id | START_OF_DATAGRAM_MASK, data, dlc);
            start_of_datagram = false;
        } else {
            can_interface_write_message(source_id, buf, dlc);
        }
    }
}

void bootloader(int arg)
{
    bootloader_config_t config;
    if (config_is_valid(&config_page1, PAGE_SIZE)) {
        config = config_read(&config_page1, PAGE_SIZE);
    } else if (config_is_valid(&config_page2, PAGE_SIZE)) {
        config = config_read(&config_page2, PAGE_SIZE);
    } else {
        config.ID = 0x2a;
        memcpy(config.board_name, "cvra.demo", 9);
        memcpy(config.device_class, "servoboard.v1", 13);
        config.application_crc = 0;
        config.application_size = 0;
        config.update_count = 0;
    }

    can_interface_init();

    static uint8_t output_buf[2048];
    static uint8_t data_buf[2048];
    static uint8_t addr_buf[64];

    can_datagram_t dt;
    can_datagram_init(&dt);
    can_datagram_set_address_buffer(&dt, addr_buf);
    can_datagram_set_data_buffer(&dt, data_buf, sizeof(data_buf));
    can_datagram_start(&dt);

    uint8_t data[8];
    uint32_t id;
    while (true) {
        int len = can_interface_read_message(&id, data);

        if ((id & START_OF_DATAGRAM_MASK) != 0) {
            can_datagram_start(&dt);
        }

        int i;
        for (i = 0; i < len; i++) {
            can_datagram_input_byte(&dt, data[i]);
        }

        if (can_datagram_is_complete(&dt) && can_datagram_is_valid(&dt)) {
            int len = protocol_execute_command((char *)dt.data, dt.data_len,
                &commands[0], sizeof(commands)/sizeof(command_t),
                (char *)output_buf, sizeof(output_buf), &config);

            if (len > 0) {
                return_datagram(config.ID, id, output_buf, len);
            }

            can_datagram_start(&dt);
        }
    }
}
