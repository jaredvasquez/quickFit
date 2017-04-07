#include "inc/RooStatsHead.h"
#include "inc/RooFitHead.h"
#include "inc/CommonHead.h"

using namespace std;
using namespace RooFit;
using namespace RooStats;

namespace utils {

  static void collectEverything( ModelConfig *mc, RooArgSet *set ) {
    set->add(*mc->GetNuisanceParameters());
    set->add(*mc->GetGlobalObservables());
    set->add(*mc->GetParametersOfInterest());
  }

  static void setAllConstant( RooArgSet *set, bool flag ) {
    TIterator *iter = set->createIterator();
    RooRealVar *parg = NULL;
    while ( (parg=(RooRealVar*)iter->Next()) ) parg->setConstant(flag);
    SafeDelete(iter);
  }
  
  static void setAllConstant( const RooArgSet *set, bool flag ){
    TIterator *iter = set->createIterator();
    RooRealVar *parg = NULL;
    while ( (parg=(RooRealVar*)iter->Next()) ) parg->setConstant(flag);
    SafeDelete(iter);
  }

  static void fixRooStarCache( RooWorkspace *ws ) {
    cout << "Fixing cache in RooStarMomentMorph.." << endl;
    RooFIter iter = ws->components().fwdIterator();  
    RooAbsArg* arg;
    while ((arg = iter.next())) {
      if (arg->IsA() == RooStarMomentMorph::Class()) {
        ((RooStarMomentMorph*)arg)->fixCache();
      }
    }
    SafeDelete(arg);
  }

}
