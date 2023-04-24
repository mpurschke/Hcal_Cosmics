//____________________________________________________________________________..
//
// This is a template for a Fun4All SubsysReco module with all methods from the
// $OFFLINE_MAIN/include/fun4all/SubsysReco.h baseclass
// You do not have to implement all of them, you can just remove unused methods
// here and in HCal_Cosmics.h.
//

#include <iostream>
#include <iomanip>

#include "HCal_Cosmics.h"

#include <fun4all/Fun4AllReturnCodes.h>

#include <phool/PHCompositeNode.h>

#include <fun4all/Fun4AllServer.h>

#include <phool/PHCompositeNode.h>
#include <phool/PHIODataNode.h>
#include <phool/PHNode.h>                               // for PHNode
#include <phool/PHNodeIterator.h>
#include <phool/PHObject.h>                             // for PHObject
#include <phool/getClass.h>

#include <TH1.h>
#include <TH2.h>


#include <fun4all/Fun4AllHistoManager.h>

#include <Event/Event.h>
#include <Event/EventTypes.h>
#include <Event/packet.h>

using namespace std;









//____________________________________________________________________________..
HCal_Cosmics::HCal_Cosmics(const std::string &name, const std::string &combined
			   ,  const std::string &file_west
			   ,  const std::string &file_east):
  SubsysReco(name)
{

  hm = new Fun4AllHistoManager(name);
  Fun4AllServer *se = Fun4AllServer::instance();
  se->registerHistoManager(hm);

  h1 =   new TH1F ( "h1","Packets per event", 17, -0.5, 16.5);  
  h1->GetXaxis()->SetTitle("Number of packets");
  //h1->GetXaxis()->SetTitleOffset(1.7);

  h1->GetYaxis()->SetTitle("Count");
  //h1->GetYaxis()->SetTitleOffset(1.7);

  h1_hival= new TH1F ( "h1_hival","high value ", 256, 0, 20000);  
  h1_hival_count = new TH1F ( "h1_hivalcount", "high value count", 20, -0.5, 19.5);  



  h2_persistency = new TH2F ( "h2_persistency","HCAL persistency plot", 31, -0.5, 30.5, 128, -800, 16000);  
  h2_persistency->GetXaxis()->SetTitle("Sample Nr");
  h2_persistency->GetYaxis()->SetTitle("Amlitude");

  h2_packet_vs_event = new TH2F ( "h2_packet_vs_event","Packets found vs event nr", 200, 0, 20000, 20, -0.5, 19.5);  
  h2_packet_vs_event->GetXaxis()->SetTitle("Event Nr");
  h2_packet_vs_event->GetYaxis()->SetTitle("Packets found");



  _combined_filename = combined;
  _file_east = file_east;
  _file_west = file_west;

  std::cout << "HCal_Cosmics::HCal_Cosmics(const std::string &name) Calling ctor" << std::endl;

  se = Fun4AllServer::instance();
  f_e = 0;
  f_w = 0;

}

//____________________________________________________________________________..
HCal_Cosmics::~HCal_Cosmics()
{
  std::cout << "HCal_Cosmics::~HCal_Cosmics() Calling dtor" << std::endl;
}

//____________________________________________________________________________..
int HCal_Cosmics::Init(PHCompositeNode *topNode)
{
  // std::cout << "HCal_Cosmics::Init(PHCompositeNode *topNode) Initializing" << std::endl;
  return Fun4AllReturnCodes::EVENT_OK;
}

//____________________________________________________________________________..
int HCal_Cosmics::InitRun(PHCompositeNode *topNode)
{
  std::cout << "HCal_Cosmics::InitRun(PHCompositeNode *topNode) Initializing for Run XXX" << std::endl;

  h1->Reset();

  current_evtnr = 0;

  int status;
  f_e  = new fileEventiterator ( _file_east.c_str(), status);
  if (status) 
    {
      delete f_e;
      std::cout << __FILE__ << " " << __LINE__ << " cannot open " << _file_east << std::endl;
      return Fun4AllReturnCodes::ABORTRUN;
    }

  f_w  = new fileEventiterator ( _file_west.c_str(), status);
  if (status) 
    {
      delete f_w;
      std::cout << __FILE__ << " " << __LINE__ << " cannot open " << _file_west << std::endl;
      return Fun4AllReturnCodes::ABORTRUN;
    }


  return Fun4AllReturnCodes::EVENT_OK;
}

//____________________________________________________________________________..
int HCal_Cosmics::process_event(PHCompositeNode *topNode)
{
  //  std::cout << "HCal_Cosmics::process_event(PHCompositeNode *topNode) Processing Event" << std::endl;

  // we get one event each from both streams

  Event *e_e = f_e->getNextEvent();
  Event *e_w = f_w->getNextEvent();

  //one or both are exhausted? Stop.
  if ( e_e == 0 || e_w == 0)
    {
      delete e_e;
      delete e_w;
      return Fun4AllReturnCodes::ABORTRUN;
    }

  // we skip all non-data events here
  if ( e_e->getEvtType() !=1  || e_w->getEvtType() != 1)
    {

        if ( e_e->getEvtType() == 9)
	  {
	    h2_persistency->Reset();
	  }
      delete e_e;
      delete e_w;
      return Fun4AllReturnCodes::EVENT_OK;
    }

  if ( Verbosity() >= VERBOSITY_MORE)
    {
      // let's see what we got here
      e_e->identify();
      e_w->identify();
    }

  unsigned int min_depth  = 1000;  // some large number

  addPackets (e_e, min_depth);
  addPackets (e_w, min_depth);


  delete e_e;
  delete e_w;
  
  if ( min_depth >= 100)
    {
      process();
    }

  return Fun4AllReturnCodes::EVENT_OK;
}

//____________________________________________________________________________..
int HCal_Cosmics::addPackets(Event *e, unsigned int &min_depth)
{

  // I know we have only 8 packets, so 32 is plenty
  Packet *packetList[32];
  int np;
  int i;


  // get a vector with all Packet * pointers in one fell swoop
  // np says how many we got
  np  = e->getPacketList(packetList, 32);

  if (np &&  Verbosity() >= VERBOSITY_EVEN_MORE)
    {
       cout << " packet evt nr " << packetList[0]->iValue(0,"EVTNR") 
	    << " clock  " << hex << packetList[0]->iValue(0,"CLOCK") << dec << endl; 
    }

  for ( i = 0; i < np; i++)
    {
      if ( Verbosity() >= VERBOSITY_MORE)
	{
	  packetList[i]->identify();
	}
      // we need to convert since we have most likely gotten "light" packets
      // Light packets just leave the raw data in the raw data buffer.
      // Good if you just process one event after the other, but since we keep the
      // packets objects around, we make them swallow their raw data into their own
      // memory.
      packetList[i]->convert();
      
      // ok, now lets find the place where the packet goes
      int p_id = packetList[i]->getIdentifier(); // saves some typing
      auto itr = packet_pool.find(p_id);
      //      cout << __FILE__ << " " << __LINE__ << " new packetid " << p_id << endl;  
      
      if ( itr == packet_pool.end() )
	{
	  std::set<Packet *, SortByEvtNr> x;
	  //	  x.insert( packetList[i] );
	  packet_pool[p_id] = x;
	}

      // accept only packets with ok checksums 
      if ( packetList[i]->iValue(0,"EVENCHECKSUMOK") != 0 && packetList[i]->iValue(0,"ODDCHECKSUMOK") != 0)
	{
	  packet_pool[p_id].insert ( packetList[i] );
	  if ( Verbosity() >= VERBOSITY_EVEN_MORE)
	    {
	      cout << __FILE__ << " " << __LINE__ << " adding packet " << p_id  << endl;
	    }
	}
      else
	{
	  if ( Verbosity() >= VERBOSITY_MORE)
	    {
	      cout << "**** wrong checksum  ";
	      packetList[i]->identify();
	    }
	  delete packetList[i];
	}
      //cout << __FILE__ << " " << __LINE__ << " size of " << p_id << " is "  << packet_pool[p_id].size() << endl;
      if (  packet_pool[p_id].size() < min_depth ) min_depth = packet_pool[p_id].size();
      
    }


  return 0;
}

//____________________________________________________________________________..
int HCal_Cosmics::process(const unsigned int min_depth)
{

  while ( getMinDepth() >= min_depth ) 
    {

      std::vector<Packet *> aligned_packets;

      auto itr = packet_pool.begin();

      for ( ; itr != packet_pool.end(); ++itr)
	{
	  auto p_itr = itr->second.begin();

	  // if ( Verbosity() >= VERBOSITY_SOME)
	  // 	{
	  // 	  cout << " packet evt nr " << (*p_itr)->iValue(0,"EVTNR") << "  "
	  // 	       << hex << (*p_itr)->iValue(0,"CLOCK") << dec <<"   "; 
	  // 	  (*p_itr)->identify();
	  // 	}

	  if ( p_itr != itr->second.end())
	    {
	      if (  (*p_itr)->iValue(0,"EVTNR") ==   current_evtnr )
		{
		  if ( Verbosity() >= VERBOSITY_MORE)
		    {
		      cout << " packet evt nr " << (*p_itr)->iValue(0,"EVTNR") << "  "
			   << hex << (*p_itr)->iValue(0,"CLOCK") << dec <<"   "; 
		      (*p_itr)->identify();
		    }
		  aligned_packets.push_back(*p_itr);
		  itr->second.erase(p_itr);
		}
	      else if ( (*p_itr)->iValue(0,"EVTNR") <  current_evtnr ) // clean up stuff that's done
		{
		  itr->second.erase(p_itr);
		}
	    }

	}

      h1->Fill(aligned_packets.size());

      if ( Verbosity() >= VERBOSITY_SOME)
	{
	  cout << " found " << aligned_packets.size() << " packets for event nr " << setw(5) << current_evtnr << " depth= " << getMinDepth() << endl;
	}

      h2_packet_vs_event->Fill(current_evtnr, aligned_packets.size() );
      // this is the place where we call the analysis
      //      int status = Analysis();
      Analysis(aligned_packets);

      auto it = aligned_packets.begin();
      for ( ; it != aligned_packets.end(); ++it)
	{
	  if ( *it) delete (*it);
	}

      current_evtnr++;
    }
  return 0;
}

//____________________________________________________________________________..
int HCal_Cosmics::Analysis(std::vector<Packet *> & aligned_packets)
{

  int highval_count = 0;

  auto it = aligned_packets.begin();
  for ( ; it != aligned_packets.end(); ++it)
    {
      Packet *p = *it;  // easier to remember what is what

      // now we make a "trigger"
      double high_val = 0;

      for ( int c = 0; c <  p->iValue(0,"CHANNELS"); c++)
	{
	  // calculate the baseline. Bcause we don't know  where a pulse is, 
	  // we calculate  at the low and high sample range  
	  double baseline_low  = 0.;	
	  double baseline_high = 0.;	
	  double baseline = 0.;	
	  for ( int s = 0;  s< 3; s++)
	    {
	      baseline_low += p->iValue(s,c);
	    }
	  baseline_low /= 3.;

	  for ( int s = p->iValue(0,"SAMPLES") -3;  s<p->iValue(0,"CHANNELS") ; s++)
	    {
	      baseline_high += p->iValue(s,c);
	    }
	  baseline_high /= 3.;

	  baseline = baseline_high;
	  if ( baseline_low < baseline_high) baseline = baseline_low;


	  // cout << "baselines: " << baseline_low <<" " <<  baseline_high << " " <<  baseline << endl;
	  for ( int s = 0;  s< p->iValue(0,"SAMPLES"); s++)
	    {
	      double sample = p->iValue(s,c) - baseline; 
	      if ( sample  > high_val ) high_val  = sample;

	      //h2->Fill(s, c, p->iValue(s,c) );
	      //h2_baseline->Fill(s, c, p->iValue(s,c) - baseline );
	      h2_persistency->Fill(s, sample);
	      
	      //if ( p->iValue(s,c) -lowest >= 300  ) h2_persistency2->Fill(c, p->iValue(s,c) - baseline);
	      //if ( threshold && p->iValue(s,c) -lowest >= threshold  ) returnval  =1;
	    }

	}

      h1_hival->Fill(high_val);
      if ( high_val > 1500) highval_count++;


    }
  h1_hival_count->Fill(highval_count);  

  return 0;

}

//____________________________________________________________________________..
int HCal_Cosmics::ResetEvent(PHCompositeNode *topNode)
{
  //  std::cout << "HCal_Cosmics::ResetEvent(PHCompositeNode *topNode) Resetting internal structures, prepare for next event" << std::endl;
  return Fun4AllReturnCodes::EVENT_OK;
}


//____________________________________________________________________________..
unsigned int HCal_Cosmics::getMinDepth() const
{
  // calculates the lowest depth of any of the packet vectors

  unsigned int min_depth = 100000; // some impossible large number

  auto itr = packet_pool.begin();

  for ( ; itr != packet_pool.end(); ++itr)
    {
      if (  itr->second.size() < min_depth ) min_depth = itr->second.size();
    }
  return min_depth;
} 

//____________________________________________________________________________..
int HCal_Cosmics::EndRun(const int runnumber)
{
  //  std::cout << "HCal_Cosmics::EndRun(const int runnumber) Ending Run for Run " << runnumber << std::endl;
  delete f_e;
  delete f_w;

  return Fun4AllReturnCodes::EVENT_OK;
}

//____________________________________________________________________________..
int HCal_Cosmics::End(PHCompositeNode *topNode)
{
  //  std::cout << "HCal_Cosmics::End(PHCompositeNode *topNode) This is the End..." << std::endl;
  return Fun4AllReturnCodes::EVENT_OK;
}

//____________________________________________________________________________..
int HCal_Cosmics::Reset(PHCompositeNode *topNode)
{
 std::cout << "HCal_Cosmics::Reset(PHCompositeNode *topNode) being Reset" << std::endl;
  return Fun4AllReturnCodes::EVENT_OK;
}

//____________________________________________________________________________..
void HCal_Cosmics::Print(const std::string &what) const
{

  // here we ad a spy probe to give some info about our various containers

  auto itr = packet_pool.begin();

  for ( ; itr != packet_pool.end(); ++itr)
    {
      cout << "pool depth: Packet " << itr->first << setw(4) << "  depth " << itr->second.size();
      auto p_itr = itr->second.begin();
      if ( p_itr != itr->second.end() ) cout << " 1st evt nr  " << (*p_itr)->iValue(0,"EVTNR");
      cout << endl;

    }

  //std::cout << "HCal_Cosmics::Print(const std::string &what) const Printing info for " << what << std::endl;
}
