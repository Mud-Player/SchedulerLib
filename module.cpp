#include "module.h"

Module::Module()
{

}

Core *Module::core()
{
    return dynamic_cast<Core*>(parent());
}

