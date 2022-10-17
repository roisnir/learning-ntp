#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <sys/time.h>

void print_hex_buffer(unsigned char* buf, int count){
    for (int i = 0; i < count; i++)
    {
        if (i > 0 && i % 8 == 0)
        {
            if (i % 16 == 0)
            {
                printf("\n");
            }
            else
            {
                printf(" ");
            }
        }
        printf("%02x ", (unsigned char)buf[i]);
        
    }
    printf("\n");
}

void isoformatts(char dst[35], uint32_t sec, uint32_t usec){
    if (sec == 0){
        sprintf(dst, "NULL");
        return;
    }
    char tmp[22] = {0};
    time_t tmpts = sec;
    struct tm *tmp_time = localtime(&tmpts);
    strftime(tmp, 21, "%FT%H:%M", tmp_time);
    sprintf(dst, "%s:%f", tmp, tmp_time->tm_sec + ((double)usec / 0xFFFFFFFF));
}