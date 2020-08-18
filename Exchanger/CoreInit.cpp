#include "CoreInit.h"
#include <tool_enabler.h> // EnableMathModule()

bool CoreInit(std::string name, std::string key)
{
    EnableMathModules(name.c_str(), (int)name.length(), key.c_str(), (int)key.length());
    
    return IsMathConverterEnable();
}
