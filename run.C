#include <fun4all/Fun4AllServer.h>
#include "HCal_Cosmics.h"

R__LOAD_LIBRARY($OFFLINE_MAIN/lib/libfun4all.so)
R__LOAD_LIBRARY(/phenix/u/purschke/analysis/f4a/HCal_Cosmics/install/lib/libHCal_Cosmics.so)

  HCal_Cosmics * s;
int run()
{
  Fun4AllServer *se = Fun4AllServer::instance();
  se->Verbosity(0);
  s = new HCal_Cosmics("cosmics", "xx.prdf", "/sphenix/lustre01/sphnxpro/commissioning/HCal/cosmics/cosmics_East-00004861-0000.prdf" , "/sphenix/lustre01/sphnxpro/commissioning/HCal/cosmics/cosmics_West-00004861-0000.prdf");
  s->Verbosity(1);
  se->registerSubsystem(s);
  // se->Print();
  se->run(2);
  return 0;
}

