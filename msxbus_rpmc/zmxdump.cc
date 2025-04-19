#include "zmx.h"
#include "zmxbus.h"
#include <iostream>
#include <string.h>
using namespace std;


int main(int argc, char **argv)
{
    char *error;
    char dir[256];
    unsigned short err_addr[0x100];
    unsigned short eaddr;
    bool output = true;
    ReadfnPtr read = NULL;
    WritefnPtr write = NULL;
    InitfnPtr init = NULL;
    ResetfnPtr reset = NULL;
    if (argc > 1 && argv[1][0] == '-') {
        output = false;
    } else {
        strcpy(dir, argc > 1 ? argv[1] : "sdcard");
    }
    void *hDLL = OpenZemmix((char*)ZEMMIX_BUS, RTLD_LAZY);
    if (!hDLL)
    {
        printf("DLL open error!! %s\n", ZEMMIX_BUS);
        exit(1);
    }
    init = (InitfnPtr)GetZemmixFunc(hDLL, (char*)MSXINIT);
    reset = (ResetfnPtr)GetZemmixFunc(hDLL, (char*)MSXRESET);
    read = (ReadfnPtr)GetZemmixFunc(hDLL, (char*)MSXREAD);
    write = (WritefnPtr)GetZemmixFunc(hDLL, (char*)MSXWRITE);
    printf("read:%llx\n", read);
    printf("write:%llx\n", write);
    printf("init:%llx\n", init);
    printf("reset:%llx\n", reset);
    int nerror = 0;
    if (init) {
        init(dir);
        if (read) {
            unsigned char b, c, data[10];
            for (int i = 0x4000; i < 0xc000; i++)
            {
                if (i % 16 == 0 && output)
                    printf("%04x:", i);
		        int sum = 0;
                bool e = false;
                for (int j = 0; j < 50; j++)
                {
                    b = read(RD_SLTSL1, i);
                    if ( j > 0 && c != b)
                    {
                        e = true;
			eaddr = i;
                    }
                    c = b;
                }
                if (e)
		{
		    if (nerror < 0x100)
		       err_addr[nerror] = eaddr;
                    nerror++;
		}
                if (output)
                {
                    printf(e ? "\u001b[31m%02x\u001b[0m " : "\u001b[0m%02x\u001b[0m ", b);
                }
                if ((i + 1) % 16 == 0 && output)
                    printf("\n");
            }
        }
        if (reset)
            reset(500);
        if (write)
            write(WR_SLTSL1, 0x4000, 0x1);
    }
    CloseZemmix(hDLL);
    printf("\nError:%d\n", nerror);
    for(int i = 0; i < nerror; i++)
        printf("%04x,", err_addr[i]);
    printf("\n");
    return 0;
}
