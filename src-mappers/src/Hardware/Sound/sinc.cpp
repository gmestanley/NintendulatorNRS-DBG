#include "sinc.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <stdexcept>
#include "../../interface.h"

    // Initialize the filter with source and target sample rates
    void SincResampler::initialize(double _sourceRate, double _targetRate, int _numTaps, double beta) {	
        if (_sourceRate <= 0 || _targetRate <= 0) {
            throw std::invalid_argument("Sample rates must be positive");
        }
        if (_targetRate <= _sourceRate) {
            throw std::invalid_argument("Target rate must be higher than source rate");
        }
	if (EMU) EMU->DbgOut(L"%f %f", _sourceRate, _targetRate);

	phase = 0.0;
	phaseIncrement = 0.0;
	numTaps = 0;
	bufferIndex = 0;

        sourceRate = _sourceRate;
        targetRate = _targetRate;
        numTaps = _numTaps;
        //phaseIncrement = sourceRate / targetRate;
	phaseIncrement = sourceRate / targetRate;
        filterTaps.clear();
        filterTaps.resize(numTaps);

        // Design the low-pass filter with cutoff at sourceRate/2
        double cutoff = sourceRate / 2.0;
        double normCutoff = cutoff / targetRate;
	if (EMU) EMU->DbgOut(L"%f %f", cutoff, normCutoff);

        // Generate Kaiser window
        std::vector<double> window(numTaps);
        double i0Beta = besselI0(beta);
        int halfTaps = numTaps / 2;

        for (int i = 0; i < numTaps; ++i) {
            double x = 2.0 * (i - halfTaps) / numTaps;
            window[i] = besselI0(beta * std::sqrt(1.0 - x * x)) / i0Beta;
        }

        // Generate sinc filter taps
        for (int i = 0; i < numTaps; ++i) {
            double t = i - halfTaps;
            if (t == 0) {
                filterTaps[i] = 2.0 * normCutoff * window[i];
            } else {
                filterTaps[i] = sin(2.0*M_PI*normCutoff*t) /(M_PI*t) *window[i];
            }	    
        }

        // Normalize filter taps for unity gain
        double sum = 0.0;
        for (double tap : filterTaps) {
            sum += tap;
        }
        for (double& tap : filterTaps) {
            tap /= sum;
        }
	
	sum = 0.0;
        for (double tap : filterTaps) {
            sum += tap;
	    if (EMU) EMU->DbgOut(L"%f", tap);
        }
	if (EMU) EMU->DbgOut(L"Tap sum: %f", sum);

        // Initialize buffer
        buffer.resize(numTaps, 0.0);
        bufferIndex = 0;
        phase = 0.0;
    }

    // Process one input sample and return the filtered output
    double SincResampler::process(double input) {
        // Store input sample in circular buffer
        buffer[bufferIndex] = input;
        bufferIndex = (bufferIndex + 1) % numTaps;

        // Compute output using polyphase filtering
        double output = 0.0;
        double currentPhase = phase;
        int baseIndex = static_cast<int>(currentPhase);
        double fractional = currentPhase - baseIndex;

        // Interpolate filter taps
        for (int i = 0; i < numTaps; ++i) {
            int bufIdx = (bufferIndex - i - 1 + numTaps) % numTaps;
            double tap = interpolateFilterTaps(i, fractional);
            output += buffer[bufIdx] * tap;
        }

        // Update phase for next sample
        phase += phaseIncrement;
        while (phase >= 1.0) {
            phase -= 1.0;
        }

        return output;
    }

    // Bessel function I0 for Kaiser window
    double SincResampler::besselI0(double x) {
	double r = 1.0f;
	double term = 1.0f;
	for( int k = 1; true; ++k )
	{
		float f = x / double(k);
		term *= 0.25f * f * f;
		double new_r = r + term;
		if( new_r == r )
		break;
		r = new_r;
	}
	return r;
    }

    // Linearly interpolate filter taps for fractional phase
    double SincResampler::interpolateFilterTaps(int tapIndex, double fractional) {
        if (fractional == 0.0) {
            return filterTaps[tapIndex];
        }
        int nextIndex = tapIndex + 1;
        if (nextIndex >= numTaps) {
            return filterTaps[tapIndex];
        }
        return filterTaps[tapIndex] * (1.0 - fractional) + filterTaps[nextIndex] * fractional;
    }
