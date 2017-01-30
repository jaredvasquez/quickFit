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
    bool _nllOffset, _useHESSE, _useMINOS, _useSIMPLEX;
    int _minStrat, _optConst, _printLevel; 

  public:
    fitTool();
    void useHESSE( bool flag ) { _useHESSE = flag; };
    void useMINOS( bool flag ) { _useMINOS = flag; };
    void useSIMPLEX( bool flag ) { _useSIMPLEX = flag; };
    void setNLLOffset( bool flag ) { _nllOffset = flag; };
    void setTolerance( float val ) { _minTolerance = val; };
    void setStrategy( int val ) { _minStrat = val; };
    void setOptConst( int val ) { _optConst = val; };
    void setPrintLevel( int val ) { _printLevel = val; };
    
    bool checkModel(const RooStats::ModelConfig &model, bool throwOnFail=false) ;
    int profileToData(ModelConfig *mc, RooAbsData *data);
};

#endif
