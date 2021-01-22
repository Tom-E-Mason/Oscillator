
#pragma once

#include <vector>
#include <utility>

#define M_PI 3.14159265358979323846

namespace osc
{
    // -----------------------------------------------------------------------------------
    // SineWave class. Can be used to produce a sine wave in terms of samples ranging
    // between -1.0 and 1.0. Samples are produced individually by NextSample() method.
    // The sample rate is const so cannot be changed once the SineWave object is
    // instantiated.
    // -----------------------------------------------------------------------------------
    template<typename FloatType>
    class SineWave
    {
    public:
        static_assert(std::is_same_v<float, FloatType>
                      || std::is_same_v<double, FloatType>,
            "SineWave class template argument must be of type float or double");
        
    public:
        SineWave() = delete;

        // -------------------------------------------------------------------------------
        // Constructor. Initialises m_SampleRate, and can optionally be used to set
        // m_Frequency and m_Amplitude.
        //
        // Arguments:
        //     _sampleRate - audio sample rate in Hz
        //     _frequency  - frequency of sine tone produced
        //     _amplitude  - amplitude of sine tone produced
        // -------------------------------------------------------------------------------
        SineWave(FloatType _sampleRate,
                 FloatType _frequency = 0.0,
                 FloatType _amplitude = 1.0) :
            m_SampleRate(_sampleRate),
            m_Amplitude(_amplitude)
        {
            SetFrequency(_frequency);
        };

        ~SineWave() = default;

    public:
        // -------------------------------------------------------------------------------
        // Sets m_Frequency to user input. m_PhaseDiff is calculated from this.
        // m_PhaseDiff is used in NextSample() to calculate the next sample value.
        //
        // Arguments:
        //     _frequency - new frequency to assign to m_Frequency
        //
        // Returns:
        //     void
        // -------------------------------------------------------------------------------
        void SetFrequency(const FloatType _frequency)
        {
            m_Frequency = _frequency;
            m_PhaseDiff = TWO_PI * m_Frequency / m_SampleRate;
        };
        FloatType GetFrequency() const { return m_Frequency; };

        void MultiplyFrequency(const FloatType _multiplier)
        {
            SetFrequency(m_Frequency * _multiplier);
        };

        void SetAmplitude(const FloatType _amplitude) { m_Amplitude = _amplitude; };
        FloatType GetAmplitude() { return m_Amplitude; };
        FloatType GetSampleRate() const { return m_SampleRate; };

        // -------------------------------------------------------------------------------
        // Calculates the next sample value. The sample is 'muted' if m_Frequency is above
        // the nyquist limit. m_Phase is always incremented by m_PhaseDiff and wrapped
        // around 2 * pi. This ensures the oscillators stay in relative phase.
        //
        // Returns:
        //     the next sample value
        // -------------------------------------------------------------------------------
        FloatType NextSample()
        {
            double dSample{ 0 };
            if (m_Frequency < m_SampleRate / 2.0)
                dSample = m_Amplitude * sin(m_Phase);

            m_Phase += m_PhaseDiff;

            if (m_Phase > TWO_PI)
                m_Phase -= TWO_PI;

            return dSample;
        }

    private:
        const FloatType m_SampleRate;
        FloatType m_Amplitude;
        FloatType m_Frequency;

        FloatType m_Phase = 0.0;
        FloatType m_PhaseDiff = 0.0;

    private:
        static constexpr FloatType TWO_PI = 2 * M_PI;
    };

    template<typename FloatType>
    class ComplexWave
    {
    public:
        static_assert(std::is_same_v<float, FloatType>
                      || std::is_same_v<double, FloatType>,
            "ComplexWave class template argument must be of type float or double");

    public:
        ComplexWave() = delete;

        // -------------------------------------------------------------------------------
        // Constructor. Initialises m_SampleRate and the number of harmonics, and can
        // optionally be used to set m_Frequency and m_Amplitude.
        //
        // Arguments:
        //     _sampleRate    - audio sample rate in Hz
        //     _uNumHarmonics - number of sine waves used to create the complex wave
        //     _frequency     - frequency of sine tone produced
        //     _amplitude     - amplitude of sine tone produced
        // -------------------------------------------------------------------------------
        ComplexWave(FloatType _sampleRate,
                    FloatType _frequency = 0.0,
                    FloatType _amplitude = 1.0,
                    size_t _uNumHarmonics = 10) :
            m_SampleRate(_sampleRate),
            m_Amplitude(_amplitude),
            m_Frequency(_frequency)
        {
            SetNumHarmonics(_uNumHarmonics);
        };

        virtual ~ComplexWave() = default;

        virtual void SetFrequency() = 0;
        FloatType GetFrequency() const { return m_vSines.front().GetFrequency(); };
        void MultiplyFrequency(const FloatType _multipler)
        {
            for (auto& s : m_vSines)
                s.MultiplyFrequency(_multipler);
        }

        // -------------------------------------------------------------------------------
        // Sums the sample values from calling NextSample on the SineWaves inside
        // m_vSines;
        //
        // Returns:
        //     the sum of the sample values
        // -------------------------------------------------------------------------------
        FloatType NextSample()
        {
            FloatType sample{ 0.0 };
            for (auto& s : m_vSines)
                sample += s.NextSample();

            return sample;
        }

        // -------------------------------------------------------------------------------
        // Sets the number of harmonics produced. If this is larger than the previous
        // number then the new SineWave objects have the correct amplitude and frequency
        // assigned.
        //
        // Arguments:
        //     _uNumHarmonics - the number of harmonics additional to the fundamental
        //
        // Returns:
        //     void
        // -------------------------------------------------------------------------------
        void SetNumHarmonics(size_t _uNumHarmonics)
        {
            const size_t uNumTones{ _uNumHarmonics + 1 };
            const size_t uOldSize{ m_vSines.size() };
            m_vSines.resize(uNumTones, m_SampleRate);

            if (uNumTones > uOldSize)
            {
                CalcAmplitudes();
                SetFrequency(m_Frequency);
            }
        }

        size_t GetNumHarmonics() const { return m_vSines.size() - 1; };
        FloatType GetAmplitude() const { return m_Amplitude; };
        FloatType GetSampleRate() const { return m_SampleRate; };

        virtual void SetAmplitude() = 0;

    protected:
        const FloatType m_SampleRate;
        FloatType m_Frequency;
        FloatType m_Amplitude;
        std::vector<SineWave<FloatType>> m_vSines;
    };

    template<typename FloatType>
    class SquareWave : public ComplexWave<FloatType>
    {
    public:
        static_assert(std::is_same_v<float, FloatType>
                      || std::is_same_v<double, FloatType>,
            "SquareWave class template argument must be of type float or double");

    public:
        SquareWave() = delete;

        // -------------------------------------------------------------------------------
        // Constructor. Initialises m_SampleRate and the number of harmonics, and can
        // optionally be used to set m_Frequency and m_Amplitude.
        //
        // Arguments:
        //     _sampleRate    - audio sample rate in Hz
        //     _uNumHarmonics - number of sine waves used to create the square wave
        //     _frequency     - frequency of sine tone produced
        //     _amplitude     - amplitude of sine tone produced
        // -------------------------------------------------------------------------------
        SquareWave(FloatType _sampleRate,
                   FloatType _frequency = 0.0,
                   FloatType _amplitude = 1.0,
                   size_t _uNumHarmonics = 10) :
            ComplexWave(_sampleRate,
                        _frequency,
                        _amplitude,
                        _uNumHarmonics) {};

    public:
        // -------------------------------------------------------------------------------
        // Sets the frequency for the fundamental and each harmonic. For a square wave
        // this is odd harmonics of the fundamental frequency. The fundamental is set, and
        // then the remaining harmonics are looped through.
        //
        // Arguments:
        //     _frequency  - fundamental frequency of square wave produced
        //
        // Returns:
        //     void
        // -------------------------------------------------------------------------------
        void SetFrequency(const FloatType _frequency) override
        {
            if (m_vSines.empty()) return;

            m_vSines.front().SetFrequency(_frequency);

            FloatType prevFrequency{ _frequency };

            for (auto it{ m_vSines.begin() + 1 }; it != m_vSines.end(); ++it)
            {
                it->SetFrequency(prevFrequency + 2 * _frequency);
                prevFrequency = it->GetFrequency();
            }
        }

        void SetAmplitude() override
        {
            for (FloatType i{ 0 }; i < m_vSines.size(); ++i)
                m_vSines.at(i).SetAmplitude(m_Amplitude / (1 + (i * 2)));
        }
    };

}
