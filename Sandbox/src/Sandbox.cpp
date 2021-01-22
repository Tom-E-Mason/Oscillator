
#include <iostream>

#include "Oscillator.h"

int main()
{
    osc::SineWave<double> s(44100.0, 440.0);

    std::cout << "Hello World!\n";
    std::cout << s.NextSample() << "\n";
}