
#include "pch.h"

#define FLOAT_T double
#define NUM_SAMPLES_TEST 30000

std::vector<FLOAT_T> vSampleRates{ 44100.0, 48000.0, 96000.0, 182000.0 };
std::vector<FLOAT_T> vFrequencies{ 10 };
std::vector<FLOAT_T> vAmplitudes{ 0.0, 0.2, 0.4, 0.6, 0.8, 1.0 };
size_t uNumSineCombinations{ vSampleRates.size() * 
                             vFrequencies.size() * 
                             vAmplitudes.size() };

std::vector<size_t> vNumHarmonics{ 2, 4, 6, 8, 10 };
size_t uNumComplexCombinations{ uNumSineCombinations * vNumHarmonics.size() };

void AddRandomFrequencies(std::vector<FLOAT_T>& vFreqs)
{
    std::default_random_engine rng;
    rng.seed((unsigned int)std::chrono::system_clock::now().time_since_epoch().count());
    std::uniform_real_distribution<FLOAT_T> dist;
    for (auto& f : vFreqs)
        f = dist(rng);
}

template <typename FloatType,
    typename = std::enable_if_t<std::is_same_v<float, FloatType>
    || std::is_same_v<double, FloatType>>>
    void CheckSine(osc::SineWave<FloatType> _sine)
{
    FloatType phase{ 0.0 };
    FloatType phaseDiff{ 2 * M_PI * _sine.GetFrequency() / _sine.GetSampleRate() };
    FloatType amplitude{ _sine.GetAmplitude() };

    for (size_t i{ 0 }; i < NUM_SAMPLES_TEST; ++i)
    {
        FloatType test = _sine.NextSample(), control = sin(phase) * amplitude;
        EXPECT_TRUE(test == control);
        phase += phaseDiff;
        if (phase > 2 * M_PI)
            phase -= 2 * M_PI;
    }
}

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

template <typename FloatType,
    typename = std::enable_if_t<std::is_same_v<float, FloatType>
    || std::is_same_v<double, FloatType>>>
    void CheckSquares(osc::SquareWave<FloatType> _square)
{
    struct Tone
    {
        FloatType phase = 0.0;
        FloatType frequency = 0.0;
        FloatType phaseDiff = 0.0;
        FloatType amplitude = 0.0;
    };
    
    const size_t uNumHarmonics{ _square.GetNumHarmonics() };
    std::vector<Tone> vWaveComponents{ uNumHarmonics + 1 };
    
    vWaveComponents.front().frequency = _square.GetFrequency();
    vWaveComponents.front().phaseDiff = 2.0 * M_PI * _square.GetFrequency() / _square.GetSampleRate();
    vWaveComponents.front().amplitude = _square.GetAmplitude();
    for (auto it{ vWaveComponents.begin() + 1 }; it != vWaveComponents.end(); ++it)
    {
        it->frequency = (it - 1)->frequency + 2.0 * _square.GetFrequency();
        it->phaseDiff = 2 * M_PI * it->frequency / _square.GetSampleRate();
        it->amplitude = _square.GetAmplitude() /
                        ((FloatType)std::distance(vWaveComponents.begin(), it) * 2.0 + 1.0);
    }

    for (size_t i{ 0 }; i < NUM_SAMPLES_TEST; ++i)
    {
        FloatType test{ _square.NextSample() };
        FloatType control{ 0.0 };
    
        for (auto& wc : vWaveComponents)
        {
            control += wc.amplitude * sin(wc.phase);
            wc.phase += wc.phaseDiff;
            if (wc.phase > 2.0 * M_PI)
                wc.phase -= 2.0 * M_PI;
        }
    
        EXPECT_TRUE(test == control);
    }
}

TEST(SquareTest, SampleTest)
{
    std::vector<osc::SquareWave<FLOAT_T>> vSquares;
    vSquares.reserve(uNumComplexCombinations);
    for (auto& sr : vSampleRates)
        for (auto& f : vFrequencies)
            for (auto& a : vAmplitudes)
                for (auto& n : vNumHarmonics)
                    vSquares.push_back({ sr, f, a , n});

    for (auto& s : vSquares)
        CheckSquares(s);
}