
#include <fstream>

#include "olcNoiseMaker.h"
#include "Oscillator.h"

double dSampleRate = 44100.0;
double dFreq{ 10 };
//static osc::SquareWave<double> s(dSampleRate, dFreq, 1.0, 10);
static osc::SineWave<double> s(dSampleRate, dFreq, 1.0);

double Next(double)
{
    s.MultiplyFrequency(1.00001);
    return s.NextSample();
}

void PrintWave(size_t _length)
{
    std::ofstream os("Samples.csv");
    if (!os.is_open())
    {
        std::cout << "Failed to open file\n";
        return;
    }

    for (size_t i{ 0 }; i < _length; ++i)
    {
        os << i << "," << s.NextSample() << "\n";
        s.MultiplyFrequency(1.0001);
    }
}

int main()
{
    auto devs{ olcNoiseMaker<short>::Enumerate() };
    olcNoiseMaker<short> nm(devs[0]);

    PrintWave((size_t)dSampleRate);

    nm.SetUserFunction(Next);

    while (1)
    {

    }
}