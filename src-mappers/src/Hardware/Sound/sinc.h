#include <vector>

class SincResampler {
public:
    void initialize(double, double, int, double);
    double process(double);
private:
    std::vector<double> filterTaps;
    std::vector<double> buffer;
    double sourceRate;
    double targetRate;
    double phase;
    double phaseIncrement;
    int numTaps;
    int bufferIndex;
    double besselI0(double);
    double interpolateFilterTaps(int, double);
};
