// Tell emacs that this is a C++ source
//  -*- C++ -*-.
#ifndef HCAL_COSMICS_H
#define HCAL_COSMICS_H

#include <fun4all/SubsysReco.h>

#include <vector>
#include <string>
#include <set>
#include <tuple>

#include <math.h>

#include <fun4all/Fun4AllServer.h>
#include <Event/fileEventiterator.h>

class Fun4AllHistoManager;
class TH1F;
class TH2F;

class PHCompositeNode;

class HCal_Cosmics : public SubsysReco
{
 public:

  HCal_Cosmics(const std::string &name
	       , const std::string &combined
	       , const std::string &file_west
	       , const std::string &file_east);

  ~HCal_Cosmics() override;

  /** Called during initialization.
      Typically this is where you can book histograms, and e.g.
      register them to Fun4AllServer (so they can be output to file
      using Fun4AllServer::dumpHistos() method).
   */
  int Init(PHCompositeNode *topNode) override;

  /** Called for first event when run number is known.
      Typically this is where you may want to fetch data from
      database, because you know the run number. A place
      to book histograms which have to know the run number.
   */
  int InitRun(PHCompositeNode *topNode) override;

  /** Called for each event.
      This is where you do the real work.
   */
  int process_event(PHCompositeNode *topNode) override;

  /// Clean up internals after each event.
  int ResetEvent(PHCompositeNode *topNode) override;

  /// Called at the end of each run.
  int EndRun(const int runnumber) override;

  /// Called at the end of all processing.
  int End(PHCompositeNode *topNode) override;

  /// Reset
  int Reset(PHCompositeNode * /*topNode*/) override;

  void Print(const std::string &what = "ALL") const override;
  int process( const unsigned int min_depth=100);

 private:

  int addPackets(Event *, unsigned int &);
  unsigned int getMinDepth() const;

  int Analysis(std::vector<Packet *> &);
  int fillHist(std::vector<Packet *> &);

  Fun4AllServer *se;

  int current_evtnr;


  std::string _combined_filename;
  std::string _file_east;
  std::string _file_west;

  Eventiterator * f_e;
  Eventiterator * f_w;


  struct SortByEvtNr
  {
    bool operator ()(Packet *lhs, Packet *rhs) const
    {
      return lhs->iValue(0,"EVTNR") < rhs->iValue(0,"EVTNR");
    }
  };



  std::map<int, std::set<Packet *, SortByEvtNr> > packet_pool;

  Fun4AllHistoManager* hm;
  TH1F * h1;
  TH2F * h2_persistency;
  TH2F * h2_packet_vs_event;
  TH1F *h1_hival;
  TH1F *h1_hival_count;
  TH2F *hcalHist;

  const double pi = 4. * atan2(1.,1.);
  const double eta_start = -1. * ( 90-36.82) *pi/180.;
  const double eta_end = ( 90-36.82) *pi/180.;
  const double eta_range = ( 90-36.82) *pi/90.;

};

#endif // HCAL_COSMICS_H
