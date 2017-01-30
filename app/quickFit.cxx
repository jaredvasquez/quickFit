#include "inc/CommonHead.h"
#include "inc/RooFitHead.h"
#include "inc/RooStatsHead.h"

#include <boost/program_options.hpp>

#include "inc/fitTool.h"
#include "inc/auxUtils.h"


std::string _inputFile = "";
std::string _minAlgo  = "Minuit2";
std::string _dataName = "combData";
std::string _wsName = "combWS";
std::string _mcName = "ModelConfig";

std::string _poiStr = "";
std::string _fixNPStr = "";

int procedure_ = 0;
bool useHesse_ = false;
float minTolerance_ = 0.001;


int main( int argc, char** argv )
{
  namespace po = boost::program_options;
  po::options_description desc( "Main options" );
  desc.add_options()
    // Input Options 
    ( "inputFile,f", po::value<std::string>(&_inputFile),  "Specify the input TFile (REQUIRED)" )
    ( "dataName,d",  po::value<std::string>(&_dataName),   "Name of the dataset" )
    ( "wsName,w",    po::value<std::string>(&_wsName),     "Name of the workspace" )
    ( "mcName,m",    po::value<std::string>(&_mcName),     "Name of the model config" )
    // Model Options
    ( "poi,p",       po::value<std::string>(&_poiStr),     "Specify POIs to be used in fit" )
    ( "fixNP,n",     po::value<std::string>(&_fixNPStr),   "Specify NPs to be used in fit" )
    // Fit Options
    // Other
    ( "help,h",  "Print help message")
    ;

  po::variables_map vm;
  try
  {
    po::store( po::command_line_parser( argc, argv ).options( desc ).run(), vm );
    po::notify( vm );
  }
  catch ( std::exception& ex )
  {
    std::cerr << "Invalid options: " << ex.what() << std::endl;
    std::cout << "Invalid options: " << ex.what() << std::endl;
    std::cout << "Use manager --help to get a list of all the allowed options"  << std::endl;
    return 999;
  }
  catch ( ... )
  {
    std::cerr << "Unidentified error parsing options." << std::endl;
    return 1000;
  }

  // if help, print help
  if ( !vm.count("inputFile") || vm.count( "help" ) )
  {
    std::cout << "Usage: manager [options]\n";
    std::cout << desc;
    return 0;
  }

  RooMsgService::instance().getStream(1).removeTopic(RooFit::NumIntegration) ;
  RooMsgService::instance().getStream(1).removeTopic(RooFit::Fitting) ;
  RooMsgService::instance().getStream(1).removeTopic(RooFit::Minimization) ;
  RooMsgService::instance().getStream(1).removeTopic(RooFit::InputArguments) ;
  RooMsgService::instance().getStream(1).removeTopic(RooFit::Eval) ;
  RooMsgService::instance().setGlobalKillBelow(RooFit::ERROR);
  
  fitTool *fitter = new fitTool();

  // Get workspace, model, and data from file
  TFile *tf = new TFile( (TString) _inputFile );
  RooWorkspace *ws = (RooWorkspace*)tf->Get( (TString) _wsName );
  RooStats::ModelConfig *mc = (RooStats::ModelConfig*)ws->obj( (TString) _mcName );
  RooAbsData *data = ws->data( (TString) _dataName );

  // Prepare model as expected
  utils::setAllConstant( mc->GetGlobalObservables(), true );
  utils::setAllConstant( mc->GetNuisanceParameters(), false );
  utils::setAllConstant( mc->GetParametersOfInterest(), true );

  // Sanity checks on model 
  cout << "Performing sanity checks on model..." << endl;
  bool validModel = fitter->checkModel( *mc, true );
  cout << "Sanity checks on the model: " << (validModel ? "OK" : "FAIL") << endl;

  // Fix nuisance narameters
  if ( vm.count("fixNP") ) {
    cout << endl << "Fixing nuisance parameters : " << endl;
    std::vector<std::string> fixNPStrs = auxUtils::Tokenize( _fixNPStr, "," );
    for( unsigned int inp(0); inp < fixNPStrs.size(); inp++ ) {
      RooAbsCollection *fixNPs = mc->GetNuisanceParameters()->selectByName( (TString) fixNPStrs[inp]);
      for (RooLinkedListIter it = fixNPs->iterator(); RooRealVar* NP = dynamic_cast<RooRealVar*>(it.Next());) {
        cout << "   Fixing nuisance parameter " << NP->GetName() << endl;
        NP->setConstant( kTRUE );
      }
    }
  }

  // Prepare parameters of interest
  if ( vm.count("poi") ) {
    cout << endl << "Preparing parameters of interest :" << endl;
    std::vector<std::string> poiStrs = auxUtils::Tokenize( _poiStr, "," );
    for( unsigned int ipoi(0); ipoi < poiStrs.size(); ipoi++ ) {
      std::vector<std::string> poiTerms = auxUtils::Tokenize( poiStrs[ipoi], "=" );
      TString poiName = (TString) poiTerms[0];
      // should add check that POI is part of model config
      if (poiTerms.size() > 1) {
        std::vector<std::string> poiVals = auxUtils::Tokenize( poiTerms[1], "_" );
        if (poiVals.size() == 3) {
          ws->var(poiName)->setConstant( kFALSE );
          ws->var(poiName)->setVal( std::stof(poiVals[0]) );
          ws->var(poiName)->setRange( std::stof(poiVals[1]), std::stof(poiVals[2]) );
        } else {
          ws->var(poiName)->setVal( std::stof(poiVals[0]) );
        }
        ws->var(poiName)->Print();
      } else {
        cout << "Releasing POI " << poiName << " in the fit." << endl;
        ws->var(poiName)->setConstant( kFALSE );
        ws->var(poiName)->Print();
      }
    }
  } else {
    RooRealVar *firstPOI = (RooRealVar*)mc->GetParametersOfInterest()->first();
    cout << endl << "No POIs specified. Will only float the first POI " << firstPOI->GetName() << endl;
    firstPOI->setConstant(kFALSE);
    firstPOI->Print();
  }

  // Fitting 
  TStopwatch timer;
  cout << endl << "Starting fit..." << endl;
  int status = fitter->profileToData( mc, data ); // Perform fit
  timer.Stop();
  double t_cpu_ = timer.CpuTime()/60.;
  double t_real_ = timer.RealTime()/60.;
  printf("All fits done in %.2f min (cpu), %.2f min (real)\n", t_cpu_, t_real_);

  // Print summary 
  cout << endl << "  Fit Summary of POIs: " << endl;
  cout << "------------------------------------------------" << endl;
  for (RooLinkedListIter it = mc->GetParametersOfInterest()->iterator(); RooRealVar* POI = dynamic_cast<RooRealVar*>(it.Next());) {
    if (POI->isConstant()) continue;
    POI->Print();
  }

  if (status) {
    cout << "*****************************************" << endl;
    cout << "  WARNING: Fit status failed.            " << endl;
    cout << "*****************************************" << endl;
  }

  cout << endl;
  return 1;
}
