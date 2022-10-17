#ifndef UTILS
#define UTILS
#include <stdint.h>
#define DEBUG_FLAG 1
#define DEBUG(fmt, ...)                                              \
    do                                                               \
    {                                                                \
        if (DEBUG_FLAG)                                              \
            fprintf(stderr, "DEBUG:%s:%d:%s(): " fmt "\n", __FILE__, \
                    __LINE__, __func__, __VA_ARGS__);                \
    } while (0)
#define ERROR(fmt, ...)                                          \
    do                                                           \
    {                                                            \
        fprintf(stderr, "ERROR:%s:%d:%s(): " fmt "\n", __FILE__, \
                __LINE__, __func__, __VA_ARGS__);                \
        perror("error: ");                                       \
    } while (0)


void print_hex_buffer(unsigned char* buf, int count);
void isoformatts(char dst[35], uint32_t sec, uint32_t usec);

#endif /* UTILS */