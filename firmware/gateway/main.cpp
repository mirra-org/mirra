#include "gateway.h"

using namespace mirra;

void setup(void)
{
    MIRRAModule::prepare();
    Gateway gateway{};
    gateway.wake();
}

void loop(void) {}
