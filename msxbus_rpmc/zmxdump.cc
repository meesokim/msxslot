#include "zmx.h"
#include "zmxbus.h"
#include <iostream>
#include <string.h>
using namespace std;


int main(int argc, char **argv)
{
    char *error;
    char dir[256];
    ReadfnPtr read = NULL;
    WritefnPtr write = NULL;
    InitfnPtr init = NULL;
    ResetfnPtr reset = NULL;
    strcpy(dir, argc > 1 ? argv[1] : "sdcard");
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
            unsigned char b, data[10];
            for (int i = 0x4000; i < 0xc000; i++)
            {
                if (i % 16 == 0)
                    printf("%04x:", i);
		int sum = 0;
                for (int j = 0; j < 20; j++)
                {
                    b = read(RD_SLTSL1, i);
                    sum += b;
                }
		if (sum/20 == b)
                	printf("\u001b[0m%02x\u001b[0m ", b);
		else
		{
			printf("\u001b[31m%02x\u001b[0m ", b);
			nerror++;
		}
                if ((i + 1) % 16 == 0)
                    printf("\n");
            }
        }
        if (reset)
            reset(500);
        if (write)
            write(WR_SLTSL1, 0x4000, 0x1);
    }
    CloseZemmix(hDLL);
    printf("Ërror:%d\n", nerror);
    return 0;
}
