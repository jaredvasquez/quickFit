#ifndef FITUTIL_HEADER
#define FITUTIL_HEADER

#include "CommonHead.h"
#include "RooFitHead.h"
#include "RooStatsHead.h"
#include "utils.h"

using namespace std;
using namespace RooFit;
using namespace RooStats;

class fitTool : public TObject{
  private:
    TString _minAlgo;
    float _minTolerance;
    bool _nllOffset, _useHESSE, _useMINOS;
    int _minStrat, _optConst, _printLevel; 

  public:
    fitTool();
    bool checkModel(const RooStats::ModelConfig &model, bool throwOnFail=false) ;
    int profileToData(ModelConfig *mc, RooAbsData *data);
};

#endif
