
#include "pch.h"
#include "OscillatorHelpers.h"

// Tests SineWave against samples generated in CheckSine();
TEST(SineTest, SampleTest)
{
    std::vector<osc::SineWave<FLOAT_T>> vSines;
    vSines.reserve(uNumSineCombinations);
    for (auto& sr : vSampleRates)
        for (auto& f : vFrequencies)
            for (auto& a : vAmplitudes)
                vSines.push_back({ sr, f, a });

    for(auto& s : vSines)
        CheckSine(s);
}

// Tests SquareWave against samples generated in CheckComplex();
TEST(SquareTest, SampleTest)
{
    std::vector<osc::SquareWave<FLOAT_T>> vSquares;
    CreateComplexWaveInstructions(vSquares);

    for (auto& s : vSquares)
        CheckComplex(s, SquareInstructions);
}
