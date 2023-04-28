#ifndef mu2e_artdaq_Generators_TrackerVST_hh
#define mu2e_artdaq_Generators_TrackerVST_hh

// TrackerVST is designed to call the DTCInterfaceLibrary a certain number of times
// (set in the mu2eFragment header) and pack that data into DTCFragments contained
// in a single mu2eFragment object.

// Some C++ conventions used:

// -Append a "_" to every private member function and variable

#include "artdaq-core/Data/Fragment.hh"
#include "artdaq/Generators/CommandableFragmentGenerator.hh"
#include "fhiclcpp/fwd.h"
#include "artdaq-core-mu2e/Overlays/FragmentType.hh"

#include "dtcInterfaceLib/DTC.h"
#include "dtcInterfaceLib/DTCSoftwareCFO.h"

#include <atomic>
#include <chrono>
#include <fstream>
#include <iostream>
#include <vector>

namespace mu2e {
  class TrackerVST : public artdaq::CommandableFragmentGenerator {
  public:
    explicit TrackerVST(fhicl::ParameterSet const& ps);
    virtual ~TrackerVST();
    
  private:
    // The "getNext_" function is used to implement user-specific
    // functionality; it's a mandatory override of the pure virtual
    // getNext_ function declared in CommandableFragmentGenerator
    
    bool getNext_(artdaq::FragmentPtrs& output) override;

    bool sendEmpty_(artdaq::FragmentPtrs& output);

    void start      () override {}
    void stopNoMutex() override {}
    void stop       () override;

    void readSimFile_(std::string sim_file);

    // Like "getNext_", "fragmentIDs_" is a mandatory override; it
    // returns a vector of the fragment IDs an instance of this class
    // is responsible for (in the case of TrackerVST, this is just
    // the fragment_id_ variable declared in the parent
    // CommandableFragmentGenerator class)

    std::vector<artdaq::Fragment::fragment_id_t> fragmentIDs_() { return fragment_ids_; }

    // FHiCL-configurable variables. Note that the C++ variable names
    // are the FHiCL variable names with a "_" appended

    FragmentType const fragment_type_;  // Type of fragment (see FragmentType.hh)
    
    std::vector<artdaq::Fragment::fragment_id_t> fragment_ids_;

    // State
    size_t                                timestamps_read_;
    size_t                                highest_timestamp_seen_{0};
    size_t                                timestamp_loops_{0}; // For playback mode, so that we continually generate unique timestamps
    DTCLib::DTC_SimMode     mode_;
    uint8_t                 board_id_;
    bool                    simFileRead_;
    bool                    rawOutput_;
    std::string             rawOutputFile_;
    std::ofstream           rawOutputStream_;
    size_t                  nSkip_;
    bool                    sendEmpties_;
    bool                    verbose_;
    size_t                  nEvents_;
    size_t                  request_delay_;
    size_t                  heartbeats_after_;
    int                     dtc_id_;
    uint                    roc_mask_;
    
    DTCLib::DTC*            _dtc;
    DTCLib::DTCSoftwareCFO* _cfo;
    
    std::chrono::steady_clock::time_point lastReportTime_;
    std::chrono::steady_clock::time_point procStartTime_;

    double _timeSinceLastSend() {
      auto now    = std::chrono::steady_clock::now();
      auto deltaw =
	std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1>>>(now - lastReportTime_).count();
      lastReportTime_ = now;
      return deltaw;
    }
    
    void   _startProcTimer() { procStartTime_ = std::chrono::steady_clock::now(); }
    
    double _getProcTimerCount() {
      auto now = std::chrono::steady_clock::now();
      auto deltaw =
	std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1>>>(now - procStartTime_).count();
      return deltaw;
    }
  };
}  // namespace mu2e

#endif /* mu2e_artdaq_Generators_TrackerVST_hh */
