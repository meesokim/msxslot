#include "zmx.h"
#include <stdio.h>
#include <link.h>
#include <elf.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    if (argc < 2) 
    {
        printf("%s: zmx_filename\n", argv[0]);
        exit(0);
    }
    auto library = dlopen(argv[1], RTLD_LAZY | RTLD_GLOBAL);
    // const char * libname = "lib.so";
    struct link_map * map = nullptr;
    dlinfo(library, RTLD_DI_LINKMAP, &map);

    Elf64_Sym * symtab = nullptr;
    char * strtab = nullptr;
    int symentries = 0;
    for (auto section = map->l_ld; section->d_tag != DT_NULL; ++section)
    {
        if (section->d_tag == DT_SYMTAB)
        {
            symtab = (Elf64_Sym *)section->d_un.d_ptr;
        }
        if (section->d_tag == DT_STRTAB)
        {
            strtab = (char*)section->d_un.d_ptr;
        }
        if (section->d_tag == DT_SYMENT)
        {
            symentries = section->d_un.d_val;
        }
    }
    int size = strtab - (char *)symtab;
    for (int k = 0; k < size / symentries; ++k)
    {
        auto sym = &symtab[k];
        // If sym is function
        if (ELF64_ST_TYPE(symtab[k].st_info) == STT_FUNC)
        {
            //str is name of each symbol
            auto str = &strtab[sym->st_name];
            printf("%s\n", str);
        }
    }
}
