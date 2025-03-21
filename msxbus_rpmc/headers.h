#include <stdbool.h> // C standard needed for bool
#include <stdint.h>  // C standard for uint8_t, uint16_t, uint32_t etc
#include <string.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#define printf(...) (0)
#if defined(RASPPI)
#include "fb.h"
#include "console.h"
#include "fatfs/ff.h"
#include "i2s_audio.h"
#endif
#include "sha1.h"
    const unsigned char sha0[] = {
#include "cartsha0.h"
    };
    const unsigned char sha0disk[] = {
#include "disksha0.inc"
    };
    unsigned char baseROM16k[] = {
#include "baserom16.b"
    };
extern "C"
{
    extern int mount(const char *source);

}

#include "rpmp.h"
#include "RPMPMapper.h"
#include "RPMPLoader.h"
#include "RPMPROM.h"