#include <stdio.h>
#include <time.h>

#include <stdio.h>
#include <string.h>

//function to convert ascii char[] to hex-string (char[])
void string2hexString(char* input, char* output)
{
    int loop;
    int i; 
    
    i=0;
    loop=0;
    
    while(input[loop] != '\0')
    {
        sprintf((char*)(output+i),"%02x", input[loop]);
        printf("%02x", (char*)(output+i));
        loop+=1;
        i+=2;
    }
    printf("\n");
    //insert NULL at the end of the output string
    output[i++] = '\0';
}

int main()
{
    time_t raw_time;
    struct timespec spec;
    clock_gettime(CLOCK_REALTIME, &spec);
    printf("%li\n", spec.tv_nsec);
    printf("%li.%li\n", spec.tv_sec, spec.tv_nsec / 1000000);
    time(&raw_time);
    printf("%li\r\n", raw_time);
    struct tm *time_info;
    time_info = localtime(&raw_time);
    printf("current local time and date: %s", asctime(time_info));
    printf("\n-------------------------------------\n\n");
    char ascii_str[] = "Hello world!";
    //declare output string with double size of input string
    //because each character of input string will be converted
    //in 2 bytes
    int len = strlen(ascii_str);
    char hex_str[(len*2)+1];
    
    //converting ascii string to hex string
    string2hexString(ascii_str, hex_str);
    
    printf("ascii_str: %s\n", ascii_str);
    printf("hex_str: %s\n", hex_str);
    
    return 0;
}