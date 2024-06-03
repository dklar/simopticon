#include <map>
#include <vector>
#include <cmath>
#include <limits>
#include <algorithm>
#include <numeric>
#include "types.h"
#include "FourierTransform.h"

class MagnitudeSpectrumBuilder
{
private:
    std::vector<double> frequencies;
public:
    MagnitudeSpectrumBuilder();
    ImageDomainVector getImageDomainData(TimeDomainVector& data);
    std::vector<double>  getSpectrumData(TimeDomainVector& data);
    std::vector<double>& getFrequencies(int vectorsize,double fa);
    ~MagnitudeSpectrumBuilder();
};
