///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
#include "artdaq/DAQdata/Globals.hh"
#define TRACE_NAME (app_name + "_TrackerVST").c_str()

// #include "otsdaq-mu2e-tracker/Generators/TrackerVST.hh"

#include "canvas/Utilities/Exception.h"

#include "artdaq-core/Utilities/SimpleLookupPolicy.hh"
#include "artdaq/Generators/GeneratorMacros.hh"
#include "cetlib_except/exception.h"
#include "fhiclcpp/ParameterSet.h"
#include "artdaq-core-mu2e/Overlays/FragmentType.hh"
#include "artdaq-core-mu2e/Overlays/mu2eFragment.hh"
#include "artdaq-core-mu2e/Overlays/mu2eFragmentWriter.hh"

#include <fstream>
#include <iomanip>
#include <iterator>

#include <unistd.h>

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

using namespace DTCLib;

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

    void printROCRegisters();
    void printDTCRegisters();

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
    size_t          timestamps_read_;
    size_t          highest_timestamp_seen_{0};
    size_t          timestamp_loops_{0}; // For playback mode, so that we continually generate unique timestamps
    DTC_SimMode     mode_;
    uint8_t         board_id_;
    bool            simFileRead_;
    bool            rawOutput_;
    std::string     rawOutputFile_;
    std::ofstream   rawOutputStream_;
    size_t          nSkip_;
    bool            sendEmpties_;
    bool            verbose_;
    size_t          nEvents_;
    size_t          request_delay_;
    size_t          _heartbeatsAfter;
    int             dtc_id_;
    uint            roc_mask_;
    
    DTC*            _dtc;
    DTCSoftwareCFO* _cfo;
    int             _firstTime;
    
    std::chrono::steady_clock::time_point lastReportTime_;
    std::chrono::steady_clock::time_point procStartTime_;

    double _timeSinceLastSend() {
      auto now        = std::chrono::steady_clock::now();
      auto deltaw     = std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1>>>(now - lastReportTime_).count();
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

//-----------------------------------------------------------------------------
mu2e::TrackerVST::TrackerVST(fhicl::ParameterSet const& ps) : 
  CommandableFragmentGenerator(ps)
  , fragment_type_   (toFragmentType("MU2E"))
  , fragment_ids_    {static_cast<artdaq::Fragment::fragment_id_t>(fragment_id())}
  , timestamps_read_ (0)
  , mode_            (DTC_SimModeConverter::ConvertToSimMode(ps.get<std::string>("sim_mode", "Disabled")))
  , board_id_        (static_cast<uint8_t>(ps.get<int>("board_id", 0)))
  , rawOutput_       (ps.get<bool>       ("raw_output_enable", false))
  , rawOutputFile_   (ps.get<std::string>("raw_output_file", "/tmp/TrackerVST.bin"))
  , nSkip_           (ps.get<size_t>     ("fragment_receiver_count", 1))
  , sendEmpties_     (ps.get<bool>       ("send_empty_fragments", false))
  , verbose_         (ps.get<bool>       ("verbose", false))
  , nEvents_         (ps.get<size_t>     ("number_of_events_to_generate"  ,    10))
  , request_delay_   (ps.get<size_t>     ("delay_between_requests_ticks"  , 20000))
  , _heartbeatsAfter (ps.get<size_t>     ("null_heartbeats_after_requests",    16)) 
  , dtc_id_          (ps.get<int>        ("dtc_id"                        ,    -1)) 
  , roc_mask_        (ps.get<int>        ("roc_mask"                      ,     0))
  , lastReportTime_  (std::chrono::steady_clock::now()) {
    
    TLOG(TLVL_DEBUG) << "TrackerVST_generator CONSTRUCTOR";
    // mode_ can still be overridden by environment!
    
    roc_mask_ = 1; // first link
    _dtc = new DTC(mode_,dtc_id_,roc_mask_,
		   "", 
		   false, 
		   ps.get<std::string>("simulator_memory_file_name","mu2esim.bin"));
    
    fhicl::ParameterSet cfoConfig = ps.get<fhicl::ParameterSet>("cfo_config", fhicl::ParameterSet());
    
    _cfo = new DTCSoftwareCFO(_dtc, 
			      cfoConfig.get<bool>("use_dtc_cfo_emulator", true), 
			      cfoConfig.get<size_t>("debug_packet_count", 0), 
			      DTC_DebugTypeConverter::ConvertToDebugType(cfoConfig.get<std::string>("debug_type", "2")), 
			      cfoConfig.get<bool>("sticky_debug_type", false), 
			      cfoConfig.get<bool>("quiet", false), 
			      cfoConfig.get<bool>("asyncRR", false), 
			      cfoConfig.get<bool>("force_no_debug_mode", false), 
			      cfoConfig.get<bool>("useCFODRP", false));
    mode_ = _dtc->ReadSimMode();
    
    TLOG(TLVL_INFO) << "The DTC Firmware version string is: " << _dtc->ReadDesignVersion();
    
    if (ps.get<bool>("load_sim_file", false)) {

      _dtc->SetDetectorEmulatorInUse();
      _dtc->ResetDDR();
      _dtc->ResetDTC();
      
      char* file_c = getenv("DTCLIB_SIM_FILE");
      
      auto sim_file = ps.get<std::string>("sim_file", "");
      if (file_c != nullptr) {
	sim_file = std::string(file_c);
      }
      if (sim_file.size() > 0) {
	simFileRead_ = false;
	std::thread reader(&mu2e::TrackerVST::readSimFile_, this, sim_file);
	reader.detach();
      }
    }
    else {
      _dtc->ClearDetectorEmulatorInUse();  // Needed if we're doing ROC Emulator...make sure Detector Emulation
					   // is disabled
      simFileRead_ = true;
    }
    
    if (rawOutput_) rawOutputStream_.open(rawOutputFile_, std::ios::out | std::ios::app | std::ios::binary);

    _firstTime = 1;

    TLOG(TLVL_INFO) << "P,Murat: VST board reader created" ;
  }

//-----------------------------------------------------------------------------
void mu2e::TrackerVST::readSimFile_(std::string sim_file) {
  TLOG(TLVL_INFO) << "Starting read of simulation file " << sim_file << "."
		  << " Please wait to start the run until finished.";
  _dtc->WriteSimFileToDTC(sim_file, true, true);
  simFileRead_ = true;
  TLOG(TLVL_INFO) << "Done reading simulation file into DTC memory.";
}

//-----------------------------------------------------------------------------
mu2e::TrackerVST::~TrackerVST() {
  rawOutputStream_.close();
  delete _cfo;
  delete _dtc;
}

//-----------------------------------------------------------------------------
void mu2e::TrackerVST::stop() {
  _dtc->DisableDetectorEmulator();
  _dtc->DisableCFOEmulation();
}

//-----------------------------------------------------------------------------
bool mu2e::TrackerVST::getNext_(artdaq::FragmentPtrs& frags) {
  // auto link = DTCLib::DTC_Link_0;  // ROC is link=0

  const char* oname = "mu2e::TrackerVST::getNext_: ";

  TLOG(TLVL_DEBUG) << oname << "P.Murat: START";

  while (!simFileRead_ and !should_stop()) {
    usleep(5000);
  }

  if (should_stop() or ev_counter() > nEvents_) return false;

  if (sendEmpties_) {
    int mod = ev_counter() % nSkip_;
    if (mod == board_id_ || (mod == 0 && board_id_ == nSkip_)) {
      // TLOG(TLVL_DEBUG) << "Sending Data  Fragment for sequence id " << ev_counter() << " (board_id " <<
      // std::to_string(board_id_) << ")" ;
    }
    else {
      // TLOG(TLVL_DEBUG) << "Sending Empty Fragment for sequence id " << ev_counter() << " (board_id " <<
      // std::to_string(board_id_) << ")" ;
      return sendEmpty_(frags);
    }
  }
  
  _startProcTimer();
  
  TLOG(TLVL_DEBUG    ) << oname << "after startProcTimer";
  
  TLOG(TLVL_TRACE + 5) << oname << "Starting CFO thread";

//   uint64_t z = 0;
//   DTCLib::DTC_EventWindowTag zero(z);

//   if (mode_ != 0) {
// #if 0
//     //_dtc->ReleaseAllBuffers();
//     TLOG(TLVL_DEBUG) << "Sending requests for " << mu2e::BLOCK_COUNT_MAX << " timestamps, starting at " << mu2e::BLOCK_COUNT_MAX * (ev_counter() - 1);
//     theCFO_->SendRequestsForRange(mu2e::BLOCK_COUNT_MAX, DTCLib::DTC_EventWindowTag(mu2e::BLOCK_COUNT_MAX * (ev_counter() - 1)));
// #else
//     int loc = mu2e::BLOCK_COUNT_MAX * (ev_counter() - 1);
//     TLOG(TLVL_DEBUG) << "P.Murat  Sending requests for " << mu2e::BLOCK_COUNT_MAX 
// 		     << " timestamps, starting at loc=" << loc;
    
//     theCFO_->SendRequestsForRange(-1, 
// 				  DTCLib::DTC_EventWindowTag(loc), 
// 				  true, request_delay_, 1, _heartbeatsAfter);
// #endif
//   }

//   _dtc->WriteROCRegister(DTCLib::DTC_Link_0,11,1,true, 0);

//   DTCLib::roc_data_t r11 = _dtc->ReadROCRegister(DTCLib::DTC_Link_0, 11, 0);
//   TLOG(TLVL_DEBUG) << "pasha reading regster 11: value=" << r11 ; 
	

//   TLOG(TLVL_TRACE + 5) << oname << "Initializing mu2eFragment metadata";
//   mu2eFragment::Metadata metadata;
//   metadata.sim_mode   = static_cast<int>(mode_);
//   metadata.run_number = run_number();
//   metadata.board_id   = board_id_;
// //-----------------------------------------------------------------------------
// // And use it, along with the artdaq::Fragment header information
// // (fragment id, sequence id, and user type) to create a fragment
// //-----------------------------------------------------------------------------
//   TLOG(TLVL_TRACE + 5) << oname << "Creating new mu2eFragment!";
//   frags.emplace_back(new artdaq::Fragment(0, ev_counter(), fragment_ids_[0], fragment_type_, metadata));

//   // Now we make an instance of the overlay to put the data into...
//   TLOG(TLVL_TRACE + 5) << oname << "Making mu2eFragmentWriter";
//   mu2eFragmentWriter newfrag(*frags.back());

//   TLOG(TLVL_TRACE + 5) << oname << "Reserving space for 16 * 201 * BLOCK_COUNT_MAX bytes";
//   newfrag.addSpace(mu2e::BLOCK_COUNT_MAX * 16 * 201);

//   // Get data from Mu2eReceiver
//   TLOG(TLVL_TRACE + 5) << oname << "Starting DTCFragment Loop";
//   _dtc->GetDevice()->ResetDeviceTime();
//   size_t totalSize = 0;
//   bool first = true;
//   while (newfrag.hdr_block_count() < mu2e::BLOCK_COUNT_MAX) {
//     if (should_stop()) {
//       break;
//     }

//     TLOG(TLVL_TRACE + 5) << oname << "Getting DTC Data for block " << newfrag.hdr_block_count() 
// 			 << "/" << mu2e::BLOCK_COUNT_MAX
// 			 << ", sz=" << totalSize;

//     std::vector<std::unique_ptr<DTCLib::DTC_Event>> data;
//     int retryCount = 5;
    
//     while (data.size() == 0 && retryCount >= 0) {
//       try {
// 	TLOG(TLVL_DEBUG) << oname << "Calling theInterface->GetData(zero)";
// 	data = _dtc->GetData(zero);
// 	TLOG(TLVL_DEBUG) << oname << "Done calling theInterface->GetData(zero)";
//       }
//       catch (std::exception const& ex) {
// 	TLOG(TLVL_ERROR) << "There was an error in the DTC Library: " << ex.what();
//       }
//       retryCount--;
//       // if (data.size() == 0){usleep(10000);}
//     }
    
//     if (retryCount < 0 && data.size() == 0) {
//       TLOG(TLVL_DEBUG) << oname << "Retry count exceeded. Something is very wrong indeed";
//       TLOG(TLVL_ERROR) << oname << "Had an error with block " << newfrag.hdr_block_count() << " of event "
// 		       << ev_counter();
//       if (newfrag.hdr_block_count() == 0) {
// 	throw cet::exception("DTC Retry count exceeded in first block of Event. Probably something is very wrong, aborting");
//       }
//       break;
//     }
    
//     TLOG(TLVL_TRACE + 6) << "Allocating space in Mu2eFragment for DTC packets";
//     totalSize = 0;
//     for (size_t i = 0; i < data.size(); ++i) {
//       totalSize += data[i]->GetEventByteCount();
//     }
    
//     int64_t diff = totalSize + newfrag.dataEndBytes() - newfrag.dataSize();
//     TLOG(TLVL_TRACE + 7) << "diff=" << diff << ", totalSize=" << totalSize << ", dataSize=" << newfrag.dataEndBytes()
// 			 << ", fragSize=" << newfrag.dataSize();
//     if (diff > 0) {
//       auto currSize = newfrag.dataSize();
//       auto remaining = 1.0 - (newfrag.hdr_block_count() / static_cast<double>(BLOCK_COUNT_MAX));
      
//       auto newSize = static_cast<size_t>(currSize * remaining);
//       TLOG(TLVL_TRACE + 8) << "mu2eReceiver::getNext: " << totalSize << " + " << newfrag.dataEndBytes() << " > "
// 			   << newfrag.dataSize() << ", allocating space for " << newSize + diff << " more bytes";
//       newfrag.addSpace(diff + newSize);
//     }

//     auto tag = data[0]->GetEventWindowTag();

//     auto fragment_timestamp = tag.GetEventWindowTag(true);
//     if (fragment_timestamp > highest_timestamp_seen_) {
//       highest_timestamp_seen_ = fragment_timestamp;
//     }
    
//     if (first) {
//       first = false;
//       if (fragment_timestamp < highest_timestamp_seen_) {
// 	if (fragment_timestamp == 0) { timestamp_loops_++; }
// 	// Timestamps start at 0, so make sure to offset by one so we don't repeat highest_timestamp_seen_
// 	fragment_timestamp += timestamp_loops_ * (highest_timestamp_seen_ + 1);
//       }
//       frags.back()->setTimestamp(fragment_timestamp);
//     }
    
//     TLOG(TLVL_TRACE + 9) << "Copying DTC packets into Mu2eFragment";
//     // auto offset = newfrag.dataBegin() + newfrag.blockSizeBytes();
//     size_t offset = newfrag.dataEndBytes();
//     for (size_t i = 0; i < data.size(); ++i) {
//       auto begin = reinterpret_cast<const uint8_t*>(data[i]->GetRawBufferPointer());
//       auto size = data[i]->GetEventByteCount();
      
//       while (data[i + 1]->GetRawBufferPointer() == static_cast<const void*>(begin + size)) {
// 	size += data[++i]->GetEventByteCount();
//       }
      
//       TLOG(TLVL_TRACE + 11) << "Copying data from " << begin << " to "
// 			    << reinterpret_cast<void*>(newfrag.dataAtBytes(offset)) << " (sz=" << size << ")";
//       // memcpy(reinterpret_cast<void*>(offset + intraBlockOffset), data[i].blockPointer, data[i].byteSize);
//       memcpy(newfrag.dataAtBytes(offset), begin, size);
//       TLOG(TLVL_TRACE + 11) << "Done with copy";
//       //std::copy(begin, reinterpret_cast<const uint8_t*>(begin) + size, newfrag.dataAtBytes(offset));
//       if (rawOutput_) rawOutputStream_.write((char*)begin, size);
//       offset += size;
//     }
    
//     TLOG(TLVL_TRACE + 12) << oname << "Ending SubEvt " << newfrag.hdr_block_count();
//     newfrag.endSubEvt(offset - newfrag.dataEndBytes());
//   }
  
//   TLOG(TLVL_TRACE + 5) << oname << "Incrementing event counter";
//   ev_counter_inc();
  
//   TLOG(TLVL_TRACE + 5) << oname << "Reporting Metrics";
//   timestamps_read_ += newfrag.hdr_block_count();
//   auto hwTime = _dtc->GetDevice()->GetDeviceTime();
  
//   double processing_rate = newfrag.hdr_block_count() / _getProcTimerCount();
//   double timestamp_rate = newfrag.hdr_block_count() / _timeSinceLastSend();
//   double hw_timestamp_rate = newfrag.hdr_block_count() / hwTime;
//   double hw_data_rate = newfrag.dataEndBytes() / hwTime;
  
//   metricMan->sendMetric("Timestamp Count", timestamps_read_, "timestamps", 1, artdaq::MetricMode::LastPoint);
//   metricMan->sendMetric("Timestamp Rate", timestamp_rate, "timestamps/s", 1, artdaq::MetricMode::Average);
//   metricMan->sendMetric("Generator Timestamp Rate", processing_rate, "timestamps/s", 1, artdaq::MetricMode::Average);
//   metricMan->sendMetric("HW Timestamp Rate", hw_timestamp_rate, "timestamps/s", 1, artdaq::MetricMode::Average);
//   metricMan->sendMetric("PCIe Transfer Rate", hw_data_rate, "B/s", 1, artdaq::MetricMode::Average);
//-----------------------------------------------------------------------------
// try to go simple way first - just copy code from Mu2eUtil
//-----------------------------------------------------------------------------
  unsigned long timestampOffset = 1;

  //  _cfo->SendRequestForTimestamp(DTC_EventWindowTag(timestampOffset + 1, _heartbeatsAfter));

  if (_firstTime) {
    int  requests_ahead(0);
    bool increment_time_stamp(true);
    uint cfo_delay(200);

    _cfo->SendRequestsForRange(nEvents_, 
			       DTC_EventWindowTag(timestampOffset), 
			       increment_time_stamp, 
			       cfo_delay, 
			       requests_ahead, 
			       _heartbeatsAfter);
    _firstTime = 0;
  }
  

  auto   tmo_ms  = 1500;
  bool   readSuccess = false;
  bool   timeout = false;
  size_t sts     = 0;

  auto   device  = _dtc->GetDevice();
  //  auto readoutRequestTime = device->GetDeviceTime();
  device->ResetDeviceTime();
  // auto afterRequests = std::chrono::steady_clock::now();

  mu2e_databuff_t* buffer;

  printROCRegisters();

  TLOG(TLVL_TRACE) << "util - before read for DAQ";
  sts = device->read_data(DTC_DMA_Engine_DAQ, reinterpret_cast<void**>(&buffer), tmo_ms);
  TLOG(TLVL_TRACE) << "util - after read for DAQ sts=" << sts << ", buffer=" << (void*)buffer;
  
  if (sts > 0)    {
    readSuccess = true;
    void* readPtr = &buffer[0];
    // uint16_t bufSize = static_cast<uint16_t>(*static_cast<uint64_t*>(readPtr));
    readPtr = static_cast<uint8_t*>(readPtr) + 8;

    if (sts > sizeof(DTC_EventHeader) + sizeof(DTC_SubEventHeader) + 8) {
      // Check for dead or cafe in first packet
      readPtr = static_cast<uint8_t*>(readPtr) + sizeof(DTC_EventHeader) + sizeof(DTC_SubEventHeader);
      std::vector<size_t> wordsToCheck{ 1, 2, 3, 7, 8 };
      for (auto& word : wordsToCheck) 	{
	auto wordPtr = static_cast<uint16_t*>(readPtr) + (word - 1);
	TLOG(TLVL_TRACE + 1) << word << (word == 1 ? "st" : word == 2 ? "nd"
					 : word == 3 ? "rd"
					 : "th")
			     << " word of buffer: " << *wordPtr;
	if (*wordPtr == 0xcafe || *wordPtr == 0xdead) 	  {
	  TLOG(TLVL_WARNING) << "Buffer Timeout detected! " 
			     << word << (word == 1 ? "st" : word == 2 ? "nd"
					 : word == 3 ? "rd" : "th")
			     << " word of buffer is 0x" << std::hex << *wordPtr;
	  DTCLib::Utilities::PrintBuffer(readPtr, 16, 0, TLVL_TRACE + 3);
	  timeout = true;
	  break;
	}
      }
    }
  }
  
  device->read_release(DTC_DMA_Engine_DAQ, 1);

  device->release_all(DTC_DMA_Engine_DAQ);
  
  // auto totalBytesRead    = device->GetReadSize();
  // auto totalBytesWritten = device->GetWriteSize();

  // auto readDevTime       = device->GetDeviceTime();
  // auto doneTime          = std::chrono::steady_clock::now();
  // auto totalTime         = std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1>>>(doneTime - startTime).count();
  // auto totalInitTime     = std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1>>>(afterInit - startTime).count();
  // auto totalRequestTime  = std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1>>>(afterRequests - afterInit).count();
  // auto totalReadTime     = std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1>>>(doneTime - afterRequests).count();
  
  // TLOG(TLVL_INFO) << "Total Elapsed Time: " << Utilities::FormatTimeString(totalTime) << "." << std::endl
  // 		  << "Total Init Time: " << Utilities::FormatTimeString(totalInitTime) << "." << std::endl
  // 		  << "Total Readout Request Time: " << Utilities::FormatTimeString(totalRequestTime) << "." << std::endl
  // 		  << "Total Read Time: " << Utilities::FormatTimeString(totalReadTime) << "." << std::endl;

  // TLOG(TLVL_INFO) << "Device Init Time: "    << Utilities::FormatTimeString(initTime) << "." << std::endl
  // 		  << "Device Request Time: " << Utilities::FormatTimeString(readoutRequestTime) << "." << std::endl
  // 		  << "Device Read Time: "    << Utilities::FormatTimeString(readDevTime) << "." << std::endl;

  // TLOG(TLVL_INFO) << "Total Bytes Written: " << Utilities::FormatByteString(static_cast<double>(totalBytesWritten), "")
  // 		  << "." << std::endl
  // 		  << "Total Bytes Read: " << Utilities::FormatByteString(static_cast<double>(totalBytesRead), "") << "."
  // 		  << std::endl;

  // TLOG(TLVL_INFO) << "Total PCIe Rate: "
  // 		  << Utilities::FormatByteString((totalBytesWritten + totalBytesRead) / totalTime, "/s") << std::endl
  // 		  << "Read Rate: " << Utilities::FormatByteString(totalBytesRead / totalReadTime, "/s") << std::endl
  // 		  << "Device Read Rate: " << Utilities::FormatByteString(totalBytesRead / readDevTime, "/s") << std::endl;


  // sts = device->read_data(DTC_DMA_Engine_DAQ, reinterpret_cast<void**>(&buffer), tmo_ms);

  // if (sts > 0) {
  //   void* readPtr = &buffer[0];
    
  //   readPtr = static_cast<uint8_t*>(readPtr) + 8;

  //   // TLOG((reallyQuiet ? TLVL_DEBUG + 4 : TLVL_INFO)) 
  //   //   << "Buffer reports DMA size of " 
  //   //   << std::dec << bufSize << " bytes. Device driver reports read of "
  //   //   << sts << " bytes," << std::endl;

  //   // TLOG(TLVL_TRACE) << "util - bufSize is " << bufSize;
    
  //   timeout = false;
     
  //   // Check for dead or cafe in first packet
  //   readPtr = static_cast<uint8_t*>(readPtr) + sizeof(DTCLib::DTC_EventHeader) + sizeof(DTCLib::DTC_SubEventHeader);
  //   std::vector<size_t> wordsToCheck{ 1, 2, 3, 7, 8 };
  //   for (auto& word : wordsToCheck) {
  //     auto wordPtr = static_cast<uint16_t*>(readPtr) + (word - 1);
  //     // TLOG(TLVL_TRACE + 1) << word << (word == 1 ? "st" : word == 2 ? "nd"
  //     // 				     : word == 3 ? "rd"
  //     // 				     : "th")
  //     // 			 << " word of buffer: " << *wordPtr;
  //     if (*wordPtr == 0xcafe || *wordPtr == 0xdead) {
  // 	// TLOG(TLVL_WARNING) << "Buffer Timeout detected! " << word << (word == 1 ? "st" : word == 2 ? "nd"
  // 	// 							      : word == 3 ? "rd"
  // 	// 							      : "th")
  // 	// 		   << " word of buffer is 0x" << std::hex << *wordPtr;
  // 	DTCLib::Utilities::PrintBuffer(readPtr, 16, 0, TLVL_TRACE + 3);
  // 	timeout = true;
  // 	break;
  //     }
  //   }
  // }
  
  TLOG(TLVL_DEBUG) << oname << "after readDTC: suceess=" << readSuccess << " timeout: "<< timeout;

  // device->read_release(DTC_DMA_Engine_DAQ, 1);
  
  TLOG(TLVL_TRACE + 5) << oname << "P.Murat: END of getNext_, return true";
  return true;
}



//-----------------------------------------------------------------------------
bool mu2e::TrackerVST::sendEmpty_(artdaq::FragmentPtrs& frags) {
  frags.emplace_back(new artdaq::Fragment());
  frags.back()->setSystemType(artdaq::Fragment::EmptyFragmentType);
  frags.back()->setSequenceID(ev_counter());
  frags.back()->setFragmentID(fragment_ids_[0]);
  ev_counter_inc();
  return true;
}

//-----------------------------------------------------------------------------
// following Monica's script var_read_all.sh
//-----------------------------------------------------------------------------
void mu2e::TrackerVST::printROCRegisters() {

  TLOG(TLVL_DEBUG) << "-------------- mu2e::TrackerVst::" << __func__ ;

  // Monica starts from disabling the EWM's
  // my_cntl write 0x91a8 0x0
  _dtc->WriteRegister_(0,DTC_Register_CFOEmulation_HeartbeatInterval); // 0x91a8

// step 1 : read everything : registers 0,8,18,23-59,65,66

  roc_data_t r[256]; 

  int tmo_ms(10);

  r[ 0] = _dtc->ReadROCRegister(DTC_Link_0,  0, tmo_ms);
  r[ 8] = _dtc->ReadROCRegister(DTC_Link_0,  8, tmo_ms);
  r[18] = _dtc->ReadROCRegister(DTC_Link_0, 18, tmo_ms);

  for (int i=23; i<60; i++) {
    r[i] = _dtc->ReadROCRegister(DTC_Link_0, i, tmo_ms);
  }

  r[65] = _dtc->ReadROCRegister(DTC_Link_0, 65, tmo_ms);
  r[66] = _dtc->ReadROCRegister(DTC_Link_0, 66, tmo_ms);
//-----------------------------------------------------------------------------
// now the hard part - formatted printout
//-----------------------------------------------------------------------------
  TLOG(TLVL_DEBUG) << "ROC reg[ 0] : 0x" << std::hex << r[ 0] 
		   << "ROC reg[ 8] : 0x" << std::hex << r[ 8] 
		   << "ROC reg[18] : 0x" << std::hex << r[18];

  int w = (r[24]<<16) | r[23] ;
  TLOG(TLVL_DEBUG) << "SIZE_FIFO_FULL[28]+STORE_POS[25:24]+STORE_CNT[19:0]: 0x" << std::hex << w;
}

//-----------------------------------------------------------------------------
void mu2e::TrackerVST::printDTCRegisters() {
  TLOG(TLVL_DEBUG) << "-------------- mu2e::TrackerVst::" << __func__ ;
}




// The following macro is defined in artdaq's GeneratorMacros.hh header
DEFINE_ARTDAQ_COMMANDABLE_GENERATOR(mu2e::TrackerVST)
