#include <stdint.h>

extern uint32_t _sbss;
extern uint32_t _ebss;
extern uint32_t _sdata;
extern uint32_t _edata;
extern uint32_t _ldata;
extern uint32_t _eram;

extern int main(void);

extern void __attribute__ ((weak)) reset_handler(void);
extern void __attribute__ ((weak)) nmi_handler(void);
extern void __attribute__ ((weak)) hard_fault_handler(void);
extern void __attribute__ ((weak)) mem_manage_handler(void);
extern void __attribute__ ((weak)) bus_fault_handler(void);
extern void __attribute__ ((weak)) usage_fault_handler(void);
extern void __attribute__ ((weak)) sv_call_handler(void);
extern void __attribute__ ((weak)) debug_monitor_handler(void);
extern void __attribute__ ((weak)) pend_sv_handler(void);
extern void __attribute__ ((weak)) sys_tick_handler(void);

__attribute__ ((section(".vector_table")))
void (*const vector_table[]) (void) = {
    (void*)&_eram,
    reset_handler,
    nmi_handler,
    hard_fault_handler,
    mem_manage_handler,
    bus_fault_handler,
    usage_fault_handler,
    // 0, 0, 0, 0,
    // sv_call_handler,
    // debug_monitor_handler,
    // 0,
    // pend_sv_handler,
    // sys_tick_handler
};

void __attribute__ ((naked)) reset_handler(void)
{
    volatile uint32_t *p_ram, *p_flash;
    // clear .bss section
    p_ram = &_sbss;
    while (p_ram < &_ebss) {
        *p_ram++ = 0;
    }
    // copy .data section from flash to ram
    p_flash = &_ldata;
    p_ram = &_sdata;
    while (p_ram < &_edata) {
        *p_ram++ = *p_flash++;
    }

    // Call the application's entry point.
    main();

    while(1);
}
