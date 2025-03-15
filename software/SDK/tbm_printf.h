#pragma once

#include <stdint.h>
#include <stddef.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdbool.h>

inline void tbm_reverse(char* str, size_t len) {
    for (size_t i = 0; i < len / 2; i++) {
        char tmp = str[i];
        str[i] = str[len - i - 1];
        str[len - i - 1] = tmp;
    }
}

inline size_t tbm_strlen(const char* str)
{
    size_t len = 0;
    for (; str[len] != 0; len++);
    return len;
}

inline uint64_t tbm_pow10(int n)
{
    const int pow10table[10] = { 1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000 };
    return pow10table[n];
}

inline double tbm_round(double value)
{
    uint64_t integer_part = (uint64_t)value;
    double rest = (value - integer_part);
    if (value > 0) {
        if (rest >= 0.5)
            return (double)integer_part + 1.0;
    }
    else {
        if (rest <= -0.5)
            return (double)integer_part - 1.0;
    }
    return (double)integer_part;
}

inline int tbm_lltoa(int64_t value, char* buf, size_t size) {
    // handle sign
    bool negative = false;
    if (value < 0) {
        negative = true;
        value = -value;
    }
    // convert integer to string
    size_t pos = 0;
    do {
        if (pos + 1 >= size)
            return -1; // check for buffer overflow
        int digit = value % 10;
        buf[pos++] = (digit < 10 ? '0' + digit : 'a' + digit - 10);
        value /= 10;
    } while (value > 0);
    if (negative) {
        if (pos + 1 >= size)
            return -1; // check for buffer overflow
        buf[pos++] = '-';
    }
    if (pos + 1 >= size)
        return -1; // check for buffer overflow
    buf[pos] = '\0';
    tbm_reverse(buf, pos);
    return (int)pos;
}

inline int tbm_ulltoa(uint64_t value, char* buf, size_t size, size_t base, const char* _digits) {
    const char* hexdigits = "0123456789abcdef";
    const char *digits = _digits != NULL ? _digits : hexdigits;
    // check if base is valid
    if (base < 2 || base > tbm_strlen(digits))
        return -1;
    // convert integer to string
    size_t pos = 0;
    do {
        if (pos >= size)
            return -1; // check for buffer overflow
        uint64_t digit = value % base;
        buf[pos++] = digits[digit];
        value /= base;
    } while (value > 0);
    if (pos >= size)
        return -1; // check for buffer overflow
    buf[pos] = '\0';
    tbm_reverse(buf, pos);
    return (int)pos;
}

inline int tbm_ftoa(double value, char* buf, size_t size, int precision) {
    if (size < 1)
        return -1;

    // handle special cases (NaN, infinity)
    if (isnan(value)) {
        if (size < 3)
            return -1;
        buf[0] = 'n'; buf[1] = 'a'; buf[2] = 'n';
        return 3;
    }
    if (isinf(value)) {
        if (value < 0) {
            if (size < 6)
                return -1;
            buf[0] = '-'; buf[1] = 'i'; buf[2] = 's'; buf[3] = 'i'; buf[4] = 'n'; buf[5] = 'f';
            return 6;
        }
        else {
            if (size < 5)
                return -1;
            buf[0] = 'i'; buf[1] = 's'; buf[2] = 'i'; buf[3] = 'n'; buf[4] = 'f';
            return 5;
        }
    }

    size_t pos = 0;
    // handle sign
    int sign = value < 0;
    if (sign) {
        value = -value;
        buf[pos++] = '-';
        if (pos >= size)
            return -1; // check for buffer overflow
    }

    // split value into integer and fractional parts
    uint64_t integer_part = (uint64_t)value;
    uint64_t fractional_part = (uint64_t)tbm_round(tbm_pow10(precision) * (value - integer_part));

    // convert integer part to string
    pos += tbm_ulltoa(integer_part, &buf[pos], size - pos, 10, NULL);
    if (pos >= size)
        return -1; // check for buffer overflow

    // convert fractional part to string
    if (precision > 0) {
        buf[pos++] = '.';
        if (pos >= size)
            return -1; // check for buffer overflow

        // Add leading zeros to the fractional part
        uint64_t pow10_prec = tbm_pow10(precision);
        for (int i = precision - 1; i > 0 && fractional_part < pow10_prec / 10; --i) {
            buf[pos++] = '0';
            if (pos >= size)
                return -1; // check for buffer overflow
            pow10_prec /= 10;
        }

        pos += tbm_ulltoa(fractional_part, &buf[pos], size - pos, 10, NULL);
        if (pos >= size)
            return -1; // check for buffer overflow
    }

    buf[pos] = '\0';
    return (int)pos;
}


inline int tbm_vsnprintf(char* str, size_t size, const char* format, va_list ap) {
    size_t pos = 0;
    const char* p = format;
    while (*p != '\0') {
        if (*p == '%') {
            p++;


            // handle flags
            bool left_justify = false;
            bool show_sign = false;
            bool space = false;
            //            bool alternate_form = false;
            bool zero_padding = false;
            while (*p == '-' || *p == '+' || *p == ' ' || *p == '#' || *p == '0') {
                if (*p == '-') left_justify = true;
                if (*p == '+') show_sign = true;
                if (*p == ' ') space = true;
                //                if (*p == '#') alternate_form = true;
                if (*p == '0') zero_padding = true;
                p++;
            }
            // handle field width
            int width = 0;
            if (*p >= '1' && *p <= '9') {
                width = *p++ - '0';
                while (*p >= '0' && *p <= '9') {
                    width = 10 * width + (*p++ - '0');
                }
            }
            else if (*p == '*') {
                width = va_arg(ap, int);
                p++;
            }
            // handle precision
            int precision = -1;
            if (*p == '.') {
                p++;
                precision = 0;
                if (*p >= '0' && *p <= '9') {
                    precision = *p++ - '0';
                    while (*p >= '0' && *p <= '9') {
                        precision = 10 * precision + (*p++ - '0');
                    }
                }
                else if (*p == '*') {
                    precision = va_arg(ap, int);
                    p++;
                }
            }
            // handle length modifier
//            bool length_char = false;
            bool length_short = false;
            bool length_long = false;
            bool length_long_long = false;
            while (*p == 'h' || *p == 'l' || *p == 'j' || *p == 'z' || *p == 't' || *p == 'L') {
                if (*p == 'h') length_short = !length_short;
                else if (*p == 'l') length_long_long |= length_long, length_long ^= 1;
                else if (*p == 'j' || *p == 'z' || *p == 't') length_long_long = 1;
                //                else if (*p == 'L') length_char = 1;
                p++;
            }



            switch (*p) {
            case 'd':
            case 'i': {
                int64_t value;
                if (length_long)
                    value = va_arg(ap, long);
                else if (length_long_long)
                    value = va_arg(ap, long long);
                else
                    value = va_arg(ap, int);

                // format and write value to str
                char buf[32];
                int n = tbm_lltoa(value, buf, sizeof(buf));
                if (n < 0) {
                    // handle error, just print the error
                    for (size_t j = 0; buf[j] != '\0' && pos < size - 1; j++)
                        str[pos++] = buf[j];
                    break;
                }

                size_t len = tbm_strlen(buf) + ((show_sign || space) ? 1 : 0);
                int padding_len = (width - (int)len) > 0 ? (width - (int)len) : 0;
                char padding_char = zero_padding ? '0' : ' ';
                if (!left_justify)
                    for (int i = 0; i < padding_len && pos < size - 1; i++)
                        str[pos++] = padding_char;

                if (show_sign && pos < size - 1)
                    str[pos++] = '+';
                else if (space && pos < size - 1)
                    str[pos++] = ' ';

                for (size_t j = 0; buf[j] != '\0' && pos < size - 1; j++)
                    str[pos++] = buf[j];

                if (left_justify)
                    for (int i = 0; i < padding_len; i++)
                        str[pos++] = ' ';
                break;
            }
            case 'u':
            case 'o':
            case 'x':
            case 'X': {
                uint64_t value;
                if (length_long) value =
                    va_arg(ap, unsigned long);
                else if (length_long_long)
                    value = va_arg(ap, unsigned long long);
                else
                    value = va_arg(ap, unsigned int);

                char buf[32];
                int base = 10;
                const char* digits = "0123456789abcdef";
                if (*p == 'o')
                    base = 8;
                else if (*p == 'x')
                    base = 16;
                else if (*p == 'X') {
                    base = 16;
                    digits = "0123456789ABCDEF";
                }
                int n = tbm_ulltoa(value, buf, sizeof(buf), base, digits);
                if (n < 0) {
                    // handle error, just print the error
                    for (size_t j = 0; buf[j] != '\0' && pos < size - 1; j++)
                        str[pos++] = buf[j];
                    break;
                }

                size_t len = tbm_strlen(buf) + ((show_sign || space) ? 1 : 0);
                int padding_len = (width - (int)len) > 0 ? (width - (int)len) : 0;
                char padding_char = zero_padding ? '0' : ' ';
                if (!left_justify)
                    for (int i = 0; i < padding_len && pos < size - 1; i++)
                        str[pos++] = padding_char;

                if (show_sign && pos < size - 1)
                    str[pos++] = '+';
                else if (space && pos < size - 1)
                    str[pos++] = ' ';

                for (size_t j = 0; buf[j] != '\0' && pos < size - 1; j++)
                    str[pos++] = buf[j];

                if (left_justify)
                    for (int i = 0; i < padding_len; i++)
                        str[pos++] = ' ';
                break;
            }
            case 'f':
            case 'F':
            case 'e':
            case 'E':
            case 'g':
            case 'G': {
                double value = va_arg(ap, double);
                if (precision < 0)
                    precision = 6;
                char buf[32];
                int n = tbm_ftoa(value, buf, sizeof(buf), precision);
                if (n < 0) {
                    // handle error, just print the error
                    for (size_t j = 0; buf[j] != '\0' && pos < size - 1; j++)
                        str[pos++] = buf[j];
                    break;
                }

                size_t len = tbm_strlen(buf) + ((show_sign || space) ? 1 : 0);
                int padding_len = (width - (int)len) > 0 ? (width - (int)len) : 0;
                char padding_char = zero_padding ? '0' : ' ';
                if (!left_justify)
                    for (int i = 0; i < padding_len && pos < size - 1; i++)
                        str[pos++] = padding_char;

                if (show_sign && pos < size - 1)
                    str[pos++] = '+';
                else if (space && pos < size - 1)
                    str[pos++] = ' ';

                for (size_t j = 0; buf[j] != '\0' && pos < size - 1; j++)
                    str[pos++] = buf[j];

                if (left_justify)
                    for (int i = 0; i < padding_len; i++)
                        str[pos++] = ' ';
                break;
            }
            case 'c': {
                int value = va_arg(ap, int);

                size_t len = 1;
                int padding_len = (width - (int)len) > 0 ? (width - (int)len) : 0;
                if (!left_justify)
                    for (int i = 0; i < padding_len && pos < size - 1; i++)
                        str[pos++] = ' ';

                if (pos < size - 1)
                    str[pos++] = value;

                if (left_justify)
                    for (int i = 0; i < padding_len; i++)
                        str[pos++] = ' ';
                break;
            }
            case 's': {
                const char* value = va_arg(ap, const char*);
                const char* buf = value;

                size_t len = tbm_strlen(value);
                int padding_len = (width - (int)len) > 0 ? (width - (int)len) : 0;
                if (!left_justify)
                    for (int i = 0; i < padding_len && pos < size - 1; i++)
                        str[pos++] = ' ';

                for (size_t j = 0; buf[j] != '\0' && pos < size - 1; j++)
                    str[pos++] = buf[j];

                if (left_justify)
                    for (int i = 0; i < padding_len; i++)
                        str[pos++] = ' ';

                break;
            }
            case '%': {
                size_t len = 1;
                int padding_len = (width - (int)len) > 0 ? (width - (int)len) : 0;
                if (!left_justify)
                    for (int i = 0; i < padding_len && pos < size - 1; i++)
                        str[pos++] = ' ';

                if (pos < size - 1)
                    str[pos++] = '%';

                if (left_justify)
                    for (int i = 0; i < padding_len; i++)
                        str[pos++] = ' ';

                break;
            }
            }
            p++;
        }
        else {
            if (pos < size - 1)
                str[pos++] = *p;
            p++;
        }
    }

    str[pos] = '\0';
    return (int)pos;
}
 
inline int tbm_snprintf(char* str, size_t size, const char* format, ...)
{
	 va_list args;
	 va_start(args, format);
	 int ret = tbm_vsnprintf(str, size, format, args);
	 va_end(args);
	 if (ret < 0 || ret >= (int)size)
	 {
		 str[size - 1] = '\0';
		 return -1;
	 }
	 return ret;
}

