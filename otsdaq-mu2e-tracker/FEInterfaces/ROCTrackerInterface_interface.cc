#include "otsdaq-mu2e-tracker/FEInterfaces/ROCTrackerInterface.h"

#include "otsdaq-core/Macros/InterfacePluginMacros.h"

using namespace ots;

#undef __MF_SUBJECT__
#define __MF_SUBJECT__ "FE-ROCTrackerInterface"

//=========================================================================================
ROCTrackerInterface::ROCTrackerInterface(
    const std::string &rocUID,
    const ConfigurationTree &theXDAQContextConfigTree,
    const std::string &theConfigurationPath)
    : ROCPolarFireCoreInterface(rocUID, theXDAQContextConfigTree,
                                theConfigurationPath) {
  INIT_MF("ROCTrackerInterface");

	__MCOUT_INFO__("ROCTrackerInterface instantiated with link: "
					<< linkID_ << " and EventWindowDelayOffset = " << delay_
					<< __E__);

 	__CFG_COUT__ << "Constructor..." << __E__;

	ConfigurationTree rocTypeLink =
		 Configurable::getSelfNode().getNode("ROCTypeLinkTable");


	TrackerParameter_1_ = rocTypeLink.getNode("NumberParam1").getValue<int>();
		
	TrackerParameter_2_ = rocTypeLink.getNode("TrueFalseParam2").getValue<bool>();
				
	//std::string STMParameter_3 = rocTypeLink.getNode("STMMustBeUniqueParam1").getValue<std::string>();
        
	__FE_COUTV__(TrackerParameter_1_);
	__FE_COUTV__(TrackerParameter_2_);
    //    __FE_COUTV__(STMParameter_3);                                


	registerFEMacroFunction("ReadROCTrackerFIFO",
	                        static_cast<FEVInterface::frontEndMacroFunction_t>(
	                            &ROCTrackerInterface::ReadTrackerFIFO),
	                        std::vector<std::string>{"NumberOfTimesToReadFIFO"}, //inputs parameters
	                        std::vector<std::string>{}, //output parameters
	                        1);  // requiredUserPermissions
 

  //try {
  //  inputTemp_ = getSelfNode().getNode("inputTemperature").getValue<double>();
  //} catch (...) {
  //  __CFG_COUT__ << "inputTemperature field not defined. Defaulting..."
  //               << __E__;
  //  inputTemp_ = 15.;
  //}

  //temp1_.noiseTemp(inputTemp_);
}
void ROCTrackerInterface::ReadTrackerFIFO(__ARGS__) 
{
	__FE_COUT__ << "# of input args = " << argsIn.size() << __E__;
	__FE_COUT__ << "# of output args = " << argsOut.size() << __E__;
	for(auto& argIn : argsIn)
		__FE_COUT__ << argIn.first << ": " << argIn.second << __E__;

	// macro commands section

	__FE_COUT__ << "# of input args = " << argsIn.size() << __E__;
	__FE_COUT__ << "# of output args = " << argsOut.size() << __E__;

	for(auto& argIn : argsIn)
		__FE_COUT__ << argIn.first << ": " << argIn.second << __E__;

	uint8_t  NumberOfTimesToReadFIFO   = __GET_ARG_IN__("NumberOfTimesToReadFIFO", uint8_t);

	__FE_COUTV__(NumberOfTimesToReadFIFO);


	std::ofstream datafile;

	std::stringstream filename;
	filename << "/home/mu2ehwdev/test_stand/ots/tracker_data.txt";
	std::string filenamestring = filename.str();
	datafile.open(filenamestring);

	for (unsigned i=0; i < NumberOfTimesToReadFIFO; i++) {

	  datafile << "============" << std::endl;
	  datafile << "Read FIFO " << std::dec << i << " times..." << std::endl; 
	  datafile << "============" << std::endl;

	  unsigned FIFOdepth = 0;
	  unsigned counter = 0;    // don't wait forever

	  while (FIFOdepth <= 0 && counter < 1000) {

	    if (counter % 100 == 0) __FE_COUT__ << "... waiting for non-zero depth" << __E__;
	    FIFOdepth = readRegister(0x35);
	    counter++;

	  }

	  if (FIFOdepth > 0) {

	    //writeRegister(0x42,1);
	    for (unsigned j=0;j < FIFOdepth; j++) {
	      datafile << "0x" << std::hex << readRegister(0x42) << "  -  " << std::dec << j << std::endl;

	    }

	  } else {
	    datafile << "... no data..." << std::endl;
	  }


	  datafile << "=================================================" << std::endl;

	}

	datafile.close();


	for(auto& argOut : argsOut)
		__FE_COUT__ << argOut.first << ": " << argOut.second << __E__;
}

//==========================================================================================
ROCTrackerInterface::~ROCTrackerInterface(void) {
	// NOTE:: be careful not to call __FE_COUT__ decoration because it uses the
  	// tree and it may already be destructed partially
  	__COUT__ << FEVInterface::interfaceUID_ << " Destructor" << __E__;
}

//==================================================================================================
void ROCTrackerInterface::writeEmulatorRegister(unsigned address,
                                                unsigned data_to_write)
{

  __FE_COUT__ << "Calling Tracker write ROC Emulator register: link number " << std::dec
              << linkID_ << ", address = " << address
              << ", write data = " << data_to_write << __E__;

  return;

} // end writeRegister()

//==================================================================================================
int ROCTrackerInterface::readEmulatorRegister(unsigned address)
{
  __CFG_COUT__ << "Tracker emulator read" << __E__;

  if (address == ADDRESS_FIRMWARE_VERSION)
    return 0x5;
  else if (address == ADDRESS_MYREGISTER)
    return temp1_.GetBoardTempC();
  else
    return -1;

} // end readRegister()

//==================================================================================================
void ROCTrackerInterface::configure(void) try
{
	
	__CFG_COUT__ << "Tracker configure, first configure back-end communication with DTC... " << __E__;
	ROCPolarFireCoreInterface::configure();

	//__MCOUT_INFO__("Tracker configure, next configure front-end... " << __E__);
	//__MCOUT_INFO__("..... write parameter 1 = " << TrackerParameter_1_ << __E__);
	//__MCOUT_INFO__("..... followed by parameter 2 = " << TrackerParameter_2_ << __E__);

}
catch(const std::runtime_error& e)
{
	__FE_MOUT__ << "Error caught: " << e.what() << __E__;
	throw;
}
catch(...)
{
	__FE_SS__ << "Unknown error caught. Check printouts!" << __E__;
	__FE_MOUT__ << ss.str();
	__FE_SS_THROW__;
}

//========================================================================================================================
void ROCTrackerInterface::start(std::string runNumber)
{
	
  std::stringstream filename;
  filename << "/home/mu2etrk/test_stand/ots/Run" << runNumber << ".txt";
  std::string filenamestring = filename.str();
  datafile_.open(filenamestring);

  number_of_good_events_ = 0;
  number_of_bad_events_ = 0;
  number_of_empty_events_ = 0;
  event_number_ = 0;
  return;
}

//========================================================================================================================
bool ROCTrackerInterface::running(void) {

	event_number_++;

	if (event_number_ % 1000 == 0) {
	  __MCOUT_INFO__("Running event number " << std::dec << event_number_ << __E__);
	}

	datafile_ << "# Event " << event_number_;

	int fail = 0;

	std::vector<uint16_t> val;

	unsigned FIFOdepth = 0;
	FIFOdepth = readRegister(35);


	unsigned counter = 0;    // don't wait forever
	    
	while ((FIFOdepth == 65535 || FIFOdepth == 0) && counter < 1000) {
	
	  readRegister(6);
	  if (counter % 100 == 0) {
	    __FE_COUT__ << "... waiting for non-zero depth" << __E__;
	  }
	  FIFOdepth = readRegister(35);
	  counter++;
       
	}
	
	if (FIFOdepth > 0 && FIFOdepth != 65535) {
			
	  unsigned depth_to_read = 200;
	  if (FIFOdepth < depth_to_read) {
	  	depth_to_read = FIFOdepth;	
	  }
	  

	  datafile_ << " ==> FIFOdepth = " << FIFOdepth << "... Number of words to read = " 
	            << depth_to_read << std::endl;
	      
	  //readBlock(val, 42 , FIFOdepth, 0);
	  readBlock(val, 42 , depth_to_read, 0);

	  if (abs(float( val.size() - depth_to_read)) < 0.5f) {
	      
	    for(size_t rr = 0; rr < val.size(); rr++)
	      {
		    
		if ( (rr+1)%5 == 0) {
		  datafile_ << " " << std::hex << val[rr] << std::dec << std::endl;
		} else {
		  datafile_ << " " << std::hex << val[rr] << std::dec;
		}
		    
		// check for data integrity here...
		//	    if(val[rr] != correct[j])
		//	      {
		//	    	      fail++;
		//	    	      __MCOUT__("... fail on read " << rr 
		//	    			<< ":  read = " << val[rr] 
		//	    			<< ", expected = " << correct[j] << __E__);
		//	    	      // __SS__ << roc->interfaceUID_ << i << "\tx " << r << " :\t "
		//	    //	    //	    //	    //	    //	    //	    //	    //	    	      //	   << "read register " << baseAddress + j << ". Mismatch on read " << val[rr]
		//	   << " vs " << correct[j] << ". Read failed on read number "
		//	   << cnt << __E__;
		//__MOUT__ << ss.str();
		//__SS_THROW__;
		//	    	    }
	      }
	  } else {
	    
	    datafile_ << "# ERROR --> Number of words returned by DTC = " << val.size() << std::endl;
	    
	    fail++;
	        
	    //	    __MCOUT__("... DTC returns " << val.size() << " words instead of " 
	    //	  		<< r << "... punt on this event" << __E__); 

	  }

	} else {

	  datafile_ << std::endl << "# EMPTY... FIFO did not report data for this event " << std::endl;
	  number_of_empty_events_++;

	}

	if (fail > 0) {
	  number_of_bad_events_++;
	} else {
	  number_of_good_events_++;
	}


	if (0){
	  unsigned data_to_check = readRegister(0x6);

	  while (data_to_check != 4860) {
	    data_to_check = readRegister(0x6);
	  }

	  data_to_check = readRegister(0x7);
	  
	  while (data_to_check != delay_) {
	    data_to_check = readRegister(0x7);
	  }
	}

	//		unsigned int          r;
	//       	//int          loops  = loops;//10 * 1000;
	//	int          cnt    = 0;
	//	int          cnts[] = {0, 0};
	//	
	//	int baseAddress = 6;
	//	unsigned int correctRegisterValue0 = 4860;
	//	unsigned int correctRegisterValue1 = delay_;
	//	
	//	unsigned int correct[] = {correctRegisterValue0,correctRegisterValue1};//{4860, 10};
	//	
	    //	for(unsigned int j = 0; j < 2; j++)
	    //	  {
	    //	    r = (rand() % 100) + 1;  //avoid calling block reads "0" times by adding 1
	    //
	    //	    //__MCOUT__(interfaceUID_ << " :\t read register " << baseAddress + j 
	    //	      << " " << r << " times" << __E__);
	    //
	    //	    readBlock(val, baseAddress + j,r,0);
	    //
	    //	    datafile_ << " ==> Number of expected words = " << r << std::endl;
	    //
	    //	    int fail = 0;

	    //	    if (abs( val.size() - r) < 0.5) {
	      //	      
	      //	      for(size_t rr = 0; rr < val.size(); rr++)
	      //	      {
	      //	        ++cnt;
	    //	    	  ++cnts[j];
	    //	    
	    //	    	  if ( (rr+1)%5 == 0) {
	    //	    	    datafile_ << std::hex << val[rr] << std::endl;
	    //	    	  } else {
	    //	    	    datafile_ << std::hex << val[rr];
	    //	    	  }
	    //	    			
	    //	    	  if(val[rr] != correct[j])
	    //	    	    {
	    //	    	      fail++;
	    //	    	      __MCOUT__("... fail on read " << rr 
	    //	    			<< ":  read = " << val[rr] 
	    //	    			<< ", expected = " << correct[j] << __E__);
	    //	    	      // __SS__ << roc->interfaceUID_ << i << "\tx " << r << " :\t "
	    //	    //	    //	    //	    //	    //	    //	    //	    //	    	      //	   << "read register " << baseAddress + j << ". Mismatch on read " << val[rr]
		      //	   << " vs " << correct[j] << ". Read failed on read number "
		      //	   << cnt << __E__;
		      //__MOUT__ << ss.str();
		      //__SS_THROW__;
	    //	    	    }
	    //	    	}
	    //	  } else {
	    //	    
	    //	    datafile_ << "#ERROR --> Number of words returned by DTC = " << val.size() << std::endl;
	    //	    
	    //	    fail++;
	    //	    
	    //	    __MCOUT__("... DTC returns " << val.size() << " words instead of " 
	    //	    		<< r << "... punt on this event" << __E__); 
	    //	    
	    //	  }


	return false; 
}


void ROCTrackerInterface::stop()  // runNumber)
{

  //  __FE_COUTV__(number_of_good_events_);
  //  __FE_COUTV__(number_of_bad_events_);


  __MCOUT__("RUN END" << __E__);
  __MCOUT__("--> number of good events = " << number_of_good_events_ << __E__);
  __MCOUT__("--> number of bad events = " << number_of_bad_events_ << __E__);
  __MCOUT__("--> number of empty events = " << number_of_empty_events_ << __E__);
	//int startIndex = getIterationIndex();

	//indicateIterationWork();  // I still need to be touched

  datafile_ << "RUN END" << std::endl;
  datafile_ << "--> number of good events = " << number_of_good_events_ << std::endl;
  datafile_ << "--> number of bad events = " << number_of_bad_events_ << std::endl;
  datafile_ << "--> number of empty events = " << number_of_empty_events_ << std::endl;

  datafile_.close();

	return;
}

//==================================================================================================
// return false to stop workloop thread
bool ROCTrackerInterface::emulatorWorkLoop(void)
{
  //__CFG_COUT__ << "emulator working..." << __E__;

  temp1_.noiseTemp(inputTemp_);
  return true; // true to keep workloop going

  //	float input, inputTemp;
  //	int addBoard, a;
  //	//
  //	addBoard = 105;
  //	inputTemp = 15.;
  //	a = 0;
  //	while( a < 20 ) {
  //		temp1.noiseTemp(inputTemp);
  //		temperature = temp1.GetBoardTempC();
  //		a++;
  //		return temperature;
  //		usleep(1000000);
  //		return true;
  //	}
} // end emulatorWorkLoop()

DEFINE_OTS_INTERFACE(ROCTrackerInterface)
