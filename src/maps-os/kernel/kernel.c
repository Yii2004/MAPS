#include "maps_os/kernel.h"

#include "maps_os/console.h"

int kmain(void) {
    console_init();
    console_write_cstr("maps-os boot\n");

    const int rc = user_main();
    if (rc == 0) {
        console_write_cstr("maps-os done\n");
    } else {
        console_write_cstr("maps-os failed\n");
    }
    return rc;
}

