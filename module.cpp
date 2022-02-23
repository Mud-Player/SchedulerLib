#include "module.h"

Module::Module()
{

}

Core *Module::core()
{
    return dynamic_cast<Core*>(parent());
}

bool Module::registerUDP(int identity)
{
    return core()->registerUDP(this, identity);
}
