
#pragma once

#include <complex>
#include <vector>
#include "FourierTransform.h"
namespace Platooning
{
    enum class IFT{
        IFT_Leader_Follower,
        IFT_Predecessor_Leader_Follower,
        IFT_Predecessor_Follower
    };

    /**
     * @brief Stablitätskriteria Liste
     * */ 
    enum class StabilityCriteria{
         /** Head Tail Stability*/
        HTS,
        /** Frequency Domain String Stable */
        FSS,
        /** Strong Frequency Domain String Stable */
        SFSS,
        /** Eventaul String Stable*/
        ESS
    };

    class Platoon
    {
    private:
        std::vector<double> mPositionData;
        IFT mIFT;
    public:
        Platoon(IFT& ift, const std::vector<double>& inputData);
        ~Platoon();
        void setData(const std::vector<double>& inputData);
    };


    class StabilityAnalyzer
    {
    private:
        /* data */
    public:
        StabilityAnalyzer(Platoon& platoon,StabilityCriteria& criteria);
        ~StabilityAnalyzer();
        double hInfintyNorm(const std::vector<double>& input,const std::vector<double>& output);

        double checkStability();
    };
    
} // namespace Platooning