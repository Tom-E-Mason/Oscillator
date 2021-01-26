
#pragma once

#define NUM_SAMPLES_TEST 30000
#define FLOAT_T double

// ---------------------------------------------------------------------------------------
// Instruction vectors. These are used to create the different testing conditions.
// ---------------------------------------------------------------------------------------
std::vector<FLOAT_T> vSampleRates{ 44100.0, 48000.0, 96000.0, 182000.0 };
std::vector<FLOAT_T> vFrequencies{ 10 };
std::vector<FLOAT_T> vAmplitudes{ 0.0, 0.2, 0.4, 0.6, 0.8, 1.0 };
size_t uNumSineCombinations{ vSampleRates.size() *
                             vFrequencies.size() *
                             vAmplitudes.size() };

std::vector<size_t> vNumHarmonics{ 2, 4, 6, 8, 10 };
size_t uNumComplexCombinations{ uNumSineCombinations * vNumHarmonics.size() };

// ---------------------------------------------------------------------------------------
// Used by testing oscillators to generate waves.
// ---------------------------------------------------------------------------------------
template<typename FloatType>
struct Tone
{
    FloatType phase = 0.0;
    FloatType frequency = 0.0;
    FloatType phaseDiff = 0.0;
    FloatType amplitude = 0.0;
};

// ---------------------------------------------------------------------------------------
// Fills a vector of floats with random numbers between 20 and 20000.
//
// Arguments:
//     _vFreqs - vector to fill
//
// Returns:
//     void
// ---------------------------------------------------------------------------------------
void AddRandomFrequencies(std::vector<FLOAT_T>& _vFreqs)
{
    std::default_random_engine rng;
    rng.seed((unsigned int)std::chrono::system_clock::now().time_since_epoch().count());
    std::uniform_real_distribution<FLOAT_T> dist;
    for (auto& f : _vFreqs)
        f = dist(rng);
}

// ---------------------------------------------------------------------------------------
// Takes in a SineWave object and tests it against the testing oscillator.
//
// Arguments:
//     _sine - SineWave object to test
//
// Returns void
// ---------------------------------------------------------------------------------------
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

// ---------------------------------------------------------------------------------------
// Takes a ComplexWave and a function pointer. The function should generate
// characteristics for the testing oscillator so it can produce the correct wave to test
// against the ComplexWave passed in.
//
// Arguments:
//     _complex - ComplexWave object (currently only derived from by SquareWave)
//     _func    - function pointer to give testing oscillator instructions
//
// Returns:
//     void
// ---------------------------------------------------------------------------------------
template <typename FloatType,
          typename F,
          typename = std::enable_if_t<std::is_same_v<float, FloatType>
                                      || std::is_same_v<double, FloatType>
                                      && std::is_invocable_v<F(std::vector<Tone<FloatType>>&,
                                                               osc::ComplexWave<FloatType>&),
                                                               std::vector<Tone<FloatType>>&,
                                                               osc::ComplexWave<FLOAT_T>&>>>
    void CheckComplex(osc::ComplexWave<FloatType>& _complex, F& _func)
{
    const size_t uNumHarmonics{ _complex.GetNumHarmonics() };
    std::vector<Tone<FLOAT_T>> vWaveComponents{ uNumHarmonics + 1 };

    vWaveComponents.front().frequency = _complex.GetFrequency();
    vWaveComponents.front().phaseDiff = 2.0 * M_PI * _complex.GetFrequency() /
                                        _complex.GetSampleRate();
    vWaveComponents.front().amplitude = _complex.GetAmplitude();
    for (auto it{ vWaveComponents.begin() + 1 }; it != vWaveComponents.end(); ++it)
    {
        _func(vWaveComponents, _complex);
    }

    for (size_t i{ 0 }; i < NUM_SAMPLES_TEST; ++i)
    {
        FloatType test{ _complex.NextSample() };
        FloatType control{ 0.0 };

        control = ComplexWaveNextSample(vWaveComponents);
        EXPECT_TRUE(test == control);
    }
}

// ---------------------------------------------------------------------------------------
// Fills vector of derivatives of ComplexWave with instructions for testing.
//
// Arguments:
//     _vWaves - vector of ComplexWave subobjects
//
// Returns:
//     void
// ---------------------------------------------------------------------------------------
template<typename WaveType,
         typename = std::enable_if_t<std::is_convertible_v<WaveType*, osc::ComplexWave<FLOAT_T>*>>>
    void CreateComplexWaveInstructions(std::vector<WaveType>& _vWaves)
{
    _vWaves.reserve(uNumComplexCombinations);
    for (auto& sr : vSampleRates)
        for (auto& f : vFrequencies)
            for (auto& a : vAmplitudes)
                for (auto& n : vNumHarmonics)
                    _vWaves.push_back({ sr, f, a , n });
}

// ---------------------------------------------------------------------------------------
// Returns the next sample from the testing oscillator.
//
// Arguments:
//     _vWaveComponents - vector of Tones which each act as their own sine oscillator
//
// Returns:
//     sample value
// ---------------------------------------------------------------------------------------
template<typename FloatType>
FloatType ComplexWaveNextSample(std::vector<Tone<FloatType>>& _vWaveComponents)
{
    FloatType sample{ 0.0 };
    for (auto& wc : _vWaveComponents)
    {
        sample += wc.amplitude * sin(wc.phase);
        wc.phase += wc.phaseDiff;
        if (wc.phase > 2.0 * M_PI)
            wc.phase -= 2.0 * M_PI;
    }

    return sample;
}