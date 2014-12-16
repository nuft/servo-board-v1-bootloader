#ifndef DEBUG_H
#define DEBUG_H

#ifdef __cplusplus
extern "C" {
#endif

void delay(int i);

void debug_init(void);
int debug(const char *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif /* DEBUG_H */