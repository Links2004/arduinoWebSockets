
#include "WebSockets.h"

#include <stdarg.h>

#ifdef DEBUG_PORT

void websocket_debug_printf(const char * format, ...) {
    va_list arg;
    va_start(arg, format);
    char temp[64];
    char * buffer = temp;
    size_t len    = vsnprintf(temp, sizeof(temp), format, arg);
    va_end(arg);
    if(len > sizeof(temp) - 1) {
        buffer = new(std::nothrow) char[len + 1];
        if(!buffer) {
            return 0;
        }
        va_start(arg, format);
        vsnprintf(buffer, len + 1, format, arg);
        va_end(arg);
    }
    len = DEBUG_PORT.write((const uint8_t *)buffer, len);
    if(buffer != temp) {
        delete[] buffer;
    }
    DEBUG_PORT.flush();
    return len;
}

#endif