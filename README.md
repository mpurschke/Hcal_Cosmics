This project tries to aligh HCal packets (currently hard-coded to 2 streams but could be expanded to more, like for EmCal).
It maintains a std::set ordered by packet-internal event number for each packet number.

Run it like this (and adjust run.C to your realities/wishes):

$ root -l run.C

root [2] Fun4AllServer *se = Fun4AllServer::instance();
root [3] se->run(102)


The sets maintain a "pool" of 100 PRDF-level events deep so it is virtually guaranteed that we find all packets 
for a given packet evt number / clock. That's why nothing really happens before the pool fill status is reached.

The way the run.C is set up is that the class' verbosity is 1, 0 is no output, 1,2,3 increase verbosity more and more. 
run.C gives you a pointer "s" to the class, so s->Verbosity(0) will make it quiet.

To do:
- there are a number of hard-coded thresholds etc in there, make them configurable
- add the code to write out the events that have been aligned, or flagged as complete, or otherwise selected
