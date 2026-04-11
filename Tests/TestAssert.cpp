#include "TestAssert.h"

#include <stdexcept>

void TestAssert_Fail(const char* message)
{
    throw std::runtime_error(message);
}
