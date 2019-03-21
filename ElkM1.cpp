#include "ElkM1.hpp"
#include <stdio.h>
#include <time.h>
#include <HardwareSerial.h>

ElkM1::ElkM1(/* args */)
{
}

ElkM1::~ElkM1()
{
}
/**
 */
size_t ElkM1::zoneState(const char* const format, std::string& input, uint8_t* const payload) {
    uint8_t zone = 0, state = 0;
    sscanf(input.c_str(), format, &zone, &state);
    payload[0] = zone;
    payload[1] = state;
    return 2U; 
}
/**
 */
size_t ElkM1::parse(std::string& input, uint8_t* const payload, size_t len) {
    size_t written = 0;
    if (NULL == payload || len == 0 || input.empty()) {
        return written;
    }

    if (input.rfind("16XK",0) == 0) {
        // command 0x06 - PERIODIC UPDATE
        payload[written] = 0x06; written++;
        //16XK29460642003191010066
        //sec:29 mn:46 hr:6 wd:4 day:20 mon:3 yr:19 dst:1 c:0 t:1
        //epoch: 1553064389
        //16XK59460642003191010063
        //16XK29470642003191010065
        //16XK0048064200319101006F
        uint8_t ss = 0, mm = 0, hh = 0, wd = 0, d =0, m = 0, y = 0, dst = 0, c = 0, t = 0;
        sscanf(input.c_str(), "%*4s%2hhu%2hhu%2hhu%1hhu%2hhu%2hhu%2hhu%1hhu%1hhu%1hhu", &ss, &mm, &hh, &wd, &d, &m, &y, &dst, &c, &t);
        struct tm  mytime = {0};
        mytime.tm_sec   = ss;       /* Seconds (0-60) */
        mytime.tm_min   = mm;       /* Minutes (0-59) */
        mytime.tm_hour  = hh;       /* Hours (0-23) */
        mytime.tm_mday  = d;        /* Day of the month (1-31) */
        mytime.tm_mon   = m - 1;    /* Month (0-11) */
        mytime.tm_year  = y + 100;  /* Year - 1900 */
        mytime.tm_isdst = dst;      /* Daylight saving time */

        time_t epoch = mktime(&mytime);
        memcpy(&payload[written], &epoch, sizeof(epoch));
        written += sizeof(epoch);
    }
    else if (input.rfind("1BSD",0) == 0) {
        // command 0x10 - STRING DESCRIPTION
        payload[written] = 0x10; written++;

        written += zoneState("%*4s%2hhu%3hhu", input, &payload[written]);
        std::string desc = input.substr(10,16); // 16 chars array
        if (desc.length() > 0 && desc.length() <= 16) {
            memcpy(&payload[written], desc.c_str(), desc.length());
            written += desc.length();
        }
    }
    else if (input.rfind("0AZC",0) == 0) {
        //0AZC003900C6
        //0AZC003200CD
        // command 0x01 - input zone
        payload[written] = 0x01; written++;
        written += zoneState("%*4s%3hhu%1hhu", input, &payload[written]);
    }
    else if (input.rfind("0AZB",0) == 0) {
        // command 0x02 - ZONES BYPASSED
        payload[written] = 0x02; written++;
        written += zoneState("%*4s%3hhu%1hhu", input, &payload[written]);
    }
    else if (input.rfind("0CZV",0) == 0) {
        // command 0x03 - VOLTAGE
        payload[written] = 0x03; written++;
        written += zoneState("%*4s%3hhu%3hhu", input, &payload[written]);
    }
    else if (input.rfind("0ACC",0) == 0) {
        // command 0x04 - OUTPUT on/off
        payload[written] = 0x04; written++;
        written += zoneState("%*4s%3hhu%1hhu", input, &payload[written]);
    }
    else if (input.rfind("0CST",0) == 0) {
        // command 0x05 - PROBE TEMPERATURE
        payload[written] = 0x05; written++;
        uint8_t type = 0, zone = 0, val = 0;
        sscanf(input.c_str(), "%*4s%1hhu%2hhu%3hhu", &type, &zone, &val);
        payload[written] = type; written++;
        payload[written] = zone; written++;
        payload[written] = val;  written++;
    }
    else if (input.rfind("16RR",0) == 0) {
        // command 0x07 - TIME
        payload[written] = 0x07; written++;
    }
    else if (input.rfind("06IE00AC",0) == 0) {
        // command 0x08 - PROGRAMMING FINISHED
        payload[written] = 0x08; written++;
    }
    else if (input.rfind("1CLD",0) == 0) {
        // command 0x09 - ZONE STATUS // not decoding time just event and (zone|user)
        payload[written] = 0x09; written++;
        written += zoneState("%*4s%4hhu%3hhu", input, &payload[written]);
    }
    else if (input.rfind("19KC",0) == 0) {
        // command 0x0A - KEYPAD KEYS
        payload[written] = 0x0A; // just say that the keypad has been pressed
        written++;
    }
    else if (input.rfind("17IC",0) == 0) {
        // command 0x0B - KEYPAD AUTH [user/keypad] user0 == fail
        payload[written] = 0x0B; written++;
        written += zoneState("%*16s%3hhu%2hhu", input, &payload[written]);
    }
    else if (input.rfind("1EAS",0) == 0) {
        //1EAS000000002111111100000000000D
        //1EAS000000001111111100000000000E
        // command 0x0C - status
        payload[written] = 0x0C; written++;
    	payload[written] = (uint8_t) input[4] - '0'; // 0..6 arm state (disarmed etc)
        written++;
        payload[written] = (uint8_t) input[12] - '0';// 0..6 current availability (ready to arm etc)
        written++;
        payload[written] = (uint8_t) input[20] - '0';// 0..19 alarm state (alarmed etc)
        written++;
    }
    else if (input.rfind("0ATC",0) == 0) {
        // command 0x0D - RUN TASK
        payload[written] = 0x0D; written++;
        uint8_t task = 0;
        sscanf(input.c_str(), "%*4s%3hhu", &task);
        payload[written] = task; written++;
    }
    else if (input.rfind("11KF",0) == 0) {
        // command 0x0E - KEYPAD CHIME/VOICE
        payload[written] = 0x0E; written++;
        written += zoneState("%*4s%2hhu%1hhu", input, &payload[written]);
    }
    else if (input.rfind("28SS",0) == 0) {
        // command 0x0F - ZONE TROUBLE
        uint32_t bitmask = 0;
        payload[written] = 0x0F; written++;
        if (input[4] != '0') {
            bitmask = 0xFFFFFFFF; // all zones
        }
        else {
            for (size_t idx = 5; idx < 37; idx++ ) {
                // 1..32 zones + 0 = all zones
                if (input[idx] != '0') {
                    bitmask += 0x01;
                }
                bitmask = bitmask << 0x01;
            }
        }
        memcpy(&payload[written], &bitmask, sizeof(bitmask));
        written += sizeof(bitmask);
        if (input[37] != '0') {
           payload[written] += (uint8_t) input[38] - '0';
           written++;
        }
    }
    return written;
}
