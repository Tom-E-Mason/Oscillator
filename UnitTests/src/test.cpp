
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

    auto squareInstructions = [](std::vector<Tone<FLOAT_T>>& _vSquareComponents,
                                 osc::ComplexWave<FLOAT_T>& _square)
    {
        for (auto it{ _vSquareComponents.begin() + 1 }; it != _vSquareComponents.end(); ++it)
        {
            it->frequency = (it - 1)->frequency + 2.0 * _square.GetFrequency();
            it->phaseDiff = 2 * M_PI * it->frequency / _square.GetSampleRate();
            it->amplitude = _square.GetAmplitude() /
                            ((FLOAT_T)std::distance(_vSquareComponents.begin(), it) * 2.0 + 1.0);
        }
    };
    
    for (auto& s : vSquares)
        CheckComplex(s, squareInstructions);
}
