#define MSXREAD "msxread"
#define MSXWRITE "msxwrite"
#define MSXRESET "reset"
#define MSXSTATUS "msxstatus"
#define MSXINIT "init"

typedef unsigned char (*ReadfnPtr)(int, unsigned short);
typedef void (*WritefnPtr)(int, unsigned short, unsigned char);
typedef void (*ResetfnPtr)(char );
typedef void (*InitfnPtr)(char *);
typedef unsigned char (*StatusfnPtr)(void);

#if defined(_MSC_VER)
    //  Microsoft 
    #define EXPORT __declspec(dllexport)
    #define IMPORT __declspec(dllimport)
#elif defined(__GNUC__)
    //  GCC
    #define EXPORT __attribute__((visibility("default")))
    #define IMPORT
#else
    //  do nothing and hope for the best?
    #define EXPORT
    #define IMPORT
    #pragma warning Unknown dynamic link import/export semantics.
#endif

#define RD_SLTSL1 0x00
#define RD_SLTSL2 0x10
#define RD_MEM    0x20
#define RD_IO     0x02
#define WR_SLTSL1 0x01
#define WR_SLTSL2 0x11
#define WR_MEM    0x21
#define WR_IO     0x03
