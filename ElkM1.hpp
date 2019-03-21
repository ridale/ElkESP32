#pragma once
#include <string>

class ElkM1
{
private:
    /* data */
static size_t zoneState(const char* const format, std::string& input, uint8_t* const payload);
public:
    ElkM1(/* args */);
    ~ElkM1();

static size_t parse(std::string& input, uint8_t* const payload, size_t len);
};
