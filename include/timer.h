#pragma once
#include <stdint.h>
namespace timer {
constexpr uint32_t HZ = 100;
void init();
uint32_t ticks();
void sleep_ms(uint32_t ms);
}
