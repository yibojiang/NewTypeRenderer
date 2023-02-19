#pragma once
#include <chrono>
#include <iomanip>

#define PROFILE(X) Profiler p(X)

namespace new_type_renderer
{
    using std::chrono::high_resolution_clock;
    using std::chrono::duration;
    using std::chrono::duration_cast;
    using std::string;

    class Profiler
    {
    public:
        Profiler(string name) : m_Name(name)
        {
            m_Start = high_resolution_clock::now();
        }

        ~Profiler()
        {
            m_End = high_resolution_clock::now();
            auto timeSpan = duration_cast<std::chrono::milliseconds>(m_End - m_Start).count();
            std::cout << "[PROFILE]" << m_Name << " takes " << timeSpan << " ms" << std::endl;
        }

    private:
        high_resolution_clock::time_point m_Start{};

        high_resolution_clock::time_point m_End{};

        string m_Name{};
    };
}
