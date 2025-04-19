#include "zmx.h"
#include "zmxbus.h"
#include <iostream>
#include <stdio.h>
// #include <unistd.h>
#include <stdlib.h>
#include <stdint.h>

#define LINES "-----------------------------------------------------\n"
using namespace std;

ReadfnPtr read = NULL;
WritefnPtr write = NULL;
InitfnPtr init = NULL;
ResetfnPtr reset = NULL;
StatusfnPtr msxstatus = NULL;

void print_memory_dump(uint16_t addr, uint8_t *data, uint8_t *data2, int len) {
    printf("%04X: ", addr);
    for (int i = 0; i < len; i++) {
        printf(data[i] == data2[i] ? "%02X " : "\u001b[0m%02x\u001b[0m ", data[i]);
    }
    printf("\n");
}
void dump(uint16_t start, uint16_t size, uint8_t page )
{
    uint8_t buffer[16], c = 0, b;
    uint8_t buffer2[16];
    write(WR_SLTSL1, start, page);
    for (uint16_t addr = start; addr < start + size; addr += 16) {
        // Read 16 bytes
        for (int i = 0; i < 16; i++) {
            buffer[i] = read(RD_SLTSL1, addr + i);
            buffer2[i] = read(RD_SLTSL1, addr + i);
        }
        print_memory_dump(addr, buffer, buffer2, 16);
    }    
}

int main(int argc, char **argv)
{
    char *error;
    printf("%s\n", ZEMMIX_BUS);
    void *hDLL = OpenZemmix((char*)ZEMMIX_BUS, RTLD_LAZY);
    if (!hDLL)
    {
        printf("DLL open error!!\n");
        exit(1);
    }
    // init = (InitfnPtr)GetZemmixFunc(hDLL, (char*)MSXINIT);
    // if (!init)
    // {
    // #if ! defined(WIN32)
    //     if ((error = dlerror()) != NULL)  {
    //         fputs(error, stderr);
    //         fputc('\n', stderr);
    //         exit(1);
    //     }    
    // #endif
    // }
    reset = (ResetfnPtr)GetZemmixFunc(hDLL, (char*)MSXRESET);
    read = (ReadfnPtr)GetZemmixFunc(hDLL, (char*)MSXREAD);
    write = (WritefnPtr)GetZemmixFunc(hDLL, (char*)MSXWRITE);
    msxstatus = (StatusfnPtr)GetZemmixFunc(hDLL, (char*)MSXSTATUS);
    init = (InitfnPtr)GetZemmixFunc(hDLL, (char*)MSXINIT);
    printf("read:%llx\n", (long long unsigned int)read);
    printf("write:%llx\n", (long long unsigned int)write);
    printf("init:%llx\n", (long long unsigned int)init);
    printf("reset:%llx\n", (long long unsigned int)reset);
    printf("status:%llx\n", (long long unsigned int)msxstatus);
    // msxstatus();
    // exit(0);    
    // printf("status:%02x\n", msxstatus());
    if (init)
        init((char*)"sdcard");
    reset(5);
    // reset(5);
    if (argc > 1)
    {
        char c = argv[1][0]; 
        if (c == '1')
            dump(0x4000, 0x8000, 0);
        else if (c == '2')
        {
            FILE *f = fopen("rpmp.rom", "wb");
            for (int i = 0x4000; i < 0xc000; i++)
            {
                fprintf(f, "%c", read(RD_SLTSL1, i));                
            }
            fclose(f);
        }
    } 
    else// if (read)
    {
        // dump(0x4000, 0x8000, 2);
        dump(0x4000, 0x20, 0);
        printf(LINES);
        dump(0x6000, 0x20, 1);
        printf(LINES);
        dump(0x6000, 0x20, 2);
        printf(LINES);
        dump(0x6000, 0x20, 3);
        printf(LINES);
        dump(0x8000, 0x20, 1);
        printf(LINES);
        dump(0x8000, 0x20, 2);
        printf(LINES);
        dump(0x8000, 0x20, 3);
        printf(LINES);
        dump(0xa000, 0x20, 1);
        printf(LINES);
        dump(0xa000, 0x20, 2);
        printf(LINES);
        dump(0xa000, 0x20, 3);
        printf(LINES);
    }
    CloseZemmix(hDLL);
    return 0;
}
