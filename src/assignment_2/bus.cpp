#include "bus.h"


void Bus::execute()
{
    while(1) {
        wait(port_bus_inout->value_changed_event());

    }
}

int Bus::read(uint32_t addr)
{
    return 0;
}

int Bus::write(uint32_t addr, uint8_t data)
{
    return 0;
}