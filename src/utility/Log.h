#pragma once
#include <cstdarg>
#include <iostream>
#include <ostream>

#define LOG_INFO(...) Log::Print(LogLevel::Info, __VA_ARGS__)
#define LOG_WARN(...) Log::Print(LogLevel::Warning, __VA_ARGS__)
#define LOG_ERR(...) Log::Print(LogLevel::Error, __VA_ARGS__)

namespace new_type_renderer
{
    enum LogLevel
    {
        Error   = 0,
        Warning = 1,
        Info    = 2,
        Debug   = 3
    };

    struct Log
    {
    public:
        static void Print(LogLevel log_level, const char* fmt...)
        {
            using namespace std;

            if (log_level > m_LogLevel)
            {
                return;
            }

            if (log_level == LogLevel::Info)
            {
                cout << "[Info]";
            }

            if (log_level == LogLevel::Warning)
            {
                cout << "[Warning]";
            }

            if (log_level == LogLevel::Error)
            {
                cout << "[Error]";
            }

            if (log_level == LogLevel::Debug)
            {
                cout << "[Debug]";
            }

            va_list args;
            va_start(args, fmt);
            while (*fmt != '\0')
            {
                if (*fmt == '%')
                {
                    // consume the char and read the next char
                    fmt++;
                    if (*fmt == 's')
                    {
                        const char*  s = va_arg(args, const char*);
                        std::cout << s;
                    }
                    else if (*fmt == 'c')
                    {
                        int c = va_arg(args, int);
                        std::cout << static_cast<char>(c);
                    }
                    else if (*fmt == 'f')
                    {
                        float f = va_arg(args, float);
                        std::cout << f;
                    }
                    else if (*fmt == 'l')
                    {
                        fmt++;
                        if (*fmt == 'f')
                        {
                            double lf = va_arg(args, double);
                            std::cout << lf;
                        }
                    }
                    else if (*fmt == 'd')
                    {
                        int i = va_arg(args, int);
                        std::cout << i;
                    }
                }
                else
                {
                    char ch = *fmt;
                    std::cout << ch;
                }
                
                ++fmt;
            }

            va_end(args);
            std::cout << endl;
        }

    public:
        static LogLevel m_LogLevel;
    };
}
