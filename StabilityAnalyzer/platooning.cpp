#include "platooning.h"
namespace Platooning
{
    Platoon::Platoon(IFT& ift, const std::vector<double>& inputData)
    {
        mIFT = ift;
        mPositionData = inputData;
    }

    Platoon::~Platoon()
    {
    }

    void Platoon::setData(const std::vector<double>& inputData)
    {
        mPositionData = inputData;
    }

    StabilityAnalyzer::StabilityAnalyzer(Platoon& platoon, StabilityCriteria& criteria)
    {
        if (criteria == StabilityCriteria::ESS){

        }
    }

    double StabilityAnalyzer::checkStability(){
        return 0.0;
    }

    double StabilityAnalyzer::hInfintyNorm(const std::vector<double>& input,const std::vector<double>& output){
        std::vector<std::complex<double>> Y = FourierTransform::transform(output);
        std::vector<std::complex<double>> U = FourierTransform::transform(input);
        
        int N_1 = input.size();
        int N_2 = output.size();
        if (N_1 == N_2){
            std::vector<double> G(N_1);
            double max = -1;
            for (auto i = 0; i < N_1; i++){
                G[i] = std::abs(Y[i]) / std::abs(U[i]);
                max = G[i] > max ? G[i] : max;
            }
            return max;
        }else{
            return -1;
        }
        
    }

    StabilityAnalyzer::~StabilityAnalyzer()
    {

    }

} // namespace Platooning
