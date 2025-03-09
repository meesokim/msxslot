#include "zmx.h"
#include "zmxbus.h"
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>

using namespace std;

void print_memory_dump(uint16_t addr, uint8_t *data, int len) {
    printf("%04X: ", addr);
    for (int i = 0; i < len; i++) {
        printf("%02X ", data[i]);
    }
    printf("\n");
}

int main(int argc, char **argv)
{
    char *error;
    ReadfnPtr read = NULL;
    WritefnPtr write = NULL;
    InitfnPtr init = NULL;
    ResetfnPtr reset = NULL;
    StatusfnPtr status = NULL;
    printf("%s\n", ZEMMIX_BUS);
    void *hDLL = OpenZemmix((char*)ZEMMIX_BUS, RTLD_LAZY);
    if (!hDLL)
    {
        printf("DLL open error!!\n");
        exit(1);
    }
    init = (InitfnPtr)GetZemmixFunc(hDLL, (char*)MSXINIT);
    if (!init)
    {
    #if ! defined(WIN32)
        if ((error = dlerror()) != NULL)  {
            fputs(error, stderr);
            fputc('\n', stderr);
            exit(1);
        }    
    #endif
    }
    reset = (ResetfnPtr)GetZemmixFunc(hDLL, (char*)MSXRESET);
    read = (ReadfnPtr)GetZemmixFunc(hDLL, (char*)MSXREAD);
    write = (WritefnPtr)GetZemmixFunc(hDLL, (char*)MSXWRITE);
    status = (StatusfnPtr)GetZemmixFunc(hDLL, (char*)MSXSTATUS);
    printf("read:%llx\n", read);
    printf("write:%llx\n", write);
    printf("init:%llx\n", init);
    printf("reset:%llx\n", reset);
    printf("status:%llx\n", status);
    if (init)
        init((char*)"sdcard");
    if (read)
    {
        uint8_t buffer[16];
        for (uint16_t addr = 0x4000; addr < 0x4100; addr += 16) {
            // Read 16 bytes
            for (int i = 0; i < 16; i++) {
                buffer[i] = read(RD_SLTSL1, addr + i);
            }
            print_memory_dump(addr, buffer, 16);
        }
    }
    if (reset)
        reset(0);
        sleep(1);
        reset(1);
    if (write)
        write(WR_MEM, 0x4000, 0x1);
    CloseZemmix(hDLL);
    return 0;
}
