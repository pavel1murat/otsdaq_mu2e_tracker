#include "otsdaq-mu2e-tracker/FEInterfaces/ROCTrackerInterface.h"
#include "otsdaq-core/DataManager/DataProducer.h"
#include "otsdaq-core/DataManager/RawDataSaverConsumerBase.h"

#include "otsdaq-core/Macros/InterfacePluginMacros.h"

using namespace ots;


#undef __MF_SUBJECT__
#define __MF_SUBJECT__ "FE-ROCTrackerInterface"

//=========================================================================================
ROCTrackerInterface::ROCTrackerInterface(
    const std::string&       rocUID,
    const ConfigurationTree& theXDAQContextConfigTree,
    const std::string&       theConfigurationPath)
    : ROCPolarFireCoreInterface(rocUID, theXDAQContextConfigTree, theConfigurationPath)
{
	INIT_MF("ROCTrackerInterface");

	__MCOUT_INFO__("ROCTrackerInterface instantiated with link: "
	               << linkID_ << " and EventWindowDelayOffset = " << delay_ << __E__);

	__CFG_COUT__ << "Constructor..." << __E__;

	ConfigurationTree rocTypeLink =
	    Configurable::getSelfNode().getNode("ROCTypeLinkTable");

	TrackerParameter_1_ = rocTypeLink.getNode("NumberParam1").getValue<int>();

	TrackerParameter_2_ = rocTypeLink.getNode("TrueFalseParam2").getValue<bool>();

	__FE_COUTV__(TrackerParameter_1_);
	__FE_COUTV__(TrackerParameter_2_);

	registerFEMacroFunction(
	    "ReadROCTrackerFIFO",
	    static_cast<FEProducerVInterface::frontEndMacroFunction_t>(
	        &ROCTrackerInterface::ReadTrackerFIFO),
	    std::vector<std::string>{"NumberOfTimesToReadFIFO"},  // inputs parameters
	    std::vector<std::string>{},                           // output parameters
	    1);                                                   // requiredUserPermissions

	 try {
	  inputTemp_ = getSelfNode().getNode("inputTemperature").getValue<double>();
	} catch (...) {
	  __CFG_COUT__ << "inputTemperature field not defined. Defaulting..."
	               << __E__;
	  inputTemp_ = 15.;
	}

	 temp1_.noiseTemp(inputTemp_);
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

	uint8_t NumberOfTimesToReadFIFO = __GET_ARG_IN__("NumberOfTimesToReadFIFO", uint8_t);

	__FE_COUTV__(NumberOfTimesToReadFIFO);


	for(unsigned i = 0; i < NumberOfTimesToReadFIFO; i++)
	{

		unsigned FIFOdepth = 0;
		unsigned counter   = 0;  // don't wait forever

		while(FIFOdepth <= 0 && counter < 1000)
		{
			if(counter % 100 == 0)
				__FE_COUT__ << "... waiting for non-zero depth" << __E__;
			FIFOdepth = readRegister(0x35);
			counter++;
		}

	}


	for(auto& argOut : argsOut)
		__FE_COUT__ << argOut.first << ": " << argOut.second << __E__;
}

//==========================================================================================
ROCTrackerInterface::~ROCTrackerInterface(void)
{
	// NOTE:: be careful not to call __FE_COUT__ decoration because it uses the
	// tree and it may already be destructed partially
	__COUT__ << FEProducerVInterface::interfaceUID_ << " Destructor" << __E__;
}

//==================================================================================================
void ROCTrackerInterface::writeEmulatorRegister(uint16_t address, uint16_t data_to_write)
{
	__FE_COUT__ << "Calling Tracker write ROC Emulator register: link number " << std::dec
	            << linkID_ << ", address = " << address
	            << ", write data = " << data_to_write << __E__;

	return;

}  // end writeEmulatorRegister()

//==================================================================================================
uint16_t ROCTrackerInterface::readEmulatorRegister(uint16_t address)
{
	__CFG_COUT__ << "Tracker emulator read" << __E__;

        if(address == 6 || address == 7)
		return ROCPolarFireCoreInterface::readEmulatorRegister(address);	
	if(address == ADDRESS_FIRMWARE_VERSION)
		return 0x5;
	else if(address == ADDRESS_MYREGISTER)
	 	return temp1_.GetBoardTempC();
	else
		return 0xBAFD;

}  // end readEmulatorRegister()

//==================================================================================================
void ROCTrackerInterface::readEmulatorBlock(std::vector<uint16_t>& 	data,
						uint16_t 		address,
						uint16_t		wordCount,
						bool			incrementAddress)
{
	__CFG_COUT__ << "Tracker emulator block read " << "wordCount= "<<wordCount<< __E__;

// make up some bogus data. Right now hardwired, could be read in as a parameter...
    double input_data = 15;  

	for(unsigned int i=0;i<wordCount;++i) {
          	double rand_data = input_data + 0.5 * (input_data * (((double)rand() / (RAND_MAX)) - 0.5));
	  	__CFG_COUT__ << "rand_data= "<<rand_data<< __E__;
		data.push_back(rand_data);
//		data.push_back(address + (incrementAddress?i:0));
}		
}  // end readEmulatorBlock()


//==================================================================================================
void ROCTrackerInterface::configure(void) try
{
	__CFG_COUT__
	    << "Tracker configure, first configure back-end communication with DTC... "
	    << __E__;
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
	std::cout << "TRK Inside start, opening data file " << std::endl;
	std::stringstream filename;
	filename << "/home/mu2etrk/test_stand/ots/Run" << runNumber << ".txt";
	std::string filenamestring = filename.str();
	datafile_.open(filenamestring);
	std::cout << "TRK Data file opened " << filenamestring << std::endl;

	number_of_good_events_  = 0;
	number_of_bad_events_   = 0;
	number_of_empty_events_ = 0;
	event_number_           = 0;

	//	DataProducerBase::registerToBuffer();



	return;
}

//========================================================================================================================
bool ROCTrackerInterface::running(void)
{
	event_number_++;

	if(event_number_ % 1000 == 0)
	{
		__MCOUT_INFO__("Running event number " << std::dec << event_number_ << __E__);
	}

	// make sure we have an empty buffer to put data in.
	// we may want to try a few times, then return an error

	//	if(DataProducerBase::attachToEmptySubBuffer(dataP_, headerP_) < 0)
	//{
	//__CFG_COUT__ << "There are no available buffers! Retrying...after "
	//			                "waiting 10 milliseconds!"
	//			             << std::endl;
	// usleep(10000);
	// return true;
	// }

	//
	std::vector<uint16_t> val;

	// is this really what we want? How do we know how much to read in readblock?
	unsigned FIFOdepth = 0;
	FIFOdepth          = readRegister(35);

	std::cout << "TRK FIFOdepth " << FIFOdepth << " Event number " << event_number_
	          << std::endl;

	unsigned counter = 0;  // don't wait forever

	while((FIFOdepth == 65535 || FIFOdepth == 0) && counter < 1000)
	{
		readRegister(6);
		if(counter % 100 == 0)
		{
			__FE_COUT__ << "... waiting for non-zero depth" << __E__;
		}
		FIFOdepth = readRegister(35);
		counter++;
	}

	if(FIFOdepth > 0 && FIFOdepth != 65535)
	{

	  // this is actually DCS values, I think. I'm not sure.

	 
		readBlock(val, 42 , FIFOdepth, 0);

		std::string* buffer;
		buffer = FEProducerVInterface::getNextBuffer();

		//std::string        buffer;
		buffer->resize(8);  // NOTE: this is inexpensive according to
		                   // Lorenzo/documentation in C++11 (only increases size once
		                   // and doesn't decrease size)
		memcpy((void*)buffer /*dest*/, (void*)&val /*src*/, FIFOdepth /*numOfBytes*/);

	    	// size() and length() are equivalent
		__FE_COUT__ << "Writing to buffer " << buffer->size() << " bytes!" << __E__;
		__FE_COUT__ << "Writing to buffer length " << buffer->length() << " bytes!"
		            << __E__;

		__FE_COUT__ << "Buffer Data: "
		            << BinaryStringMacros::binaryNumberToHexString(*buffer) << __E__;

		FEProducerVInterface::copyToNextBuffer(*buffer);
	}
	else
	{
		number_of_empty_events_++;
	}

	if(0)
	{
		unsigned data_to_check = readRegister(0x6);

		while(data_to_check != 4860)
		{
			data_to_check = readRegister(0x6);
		}

		data_to_check = readRegister(0x7);

		while(data_to_check != delay_)
		{
			data_to_check = readRegister(0x7);
		}
	}

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
	// int startIndex = getIterationIndex();

	// indicateIterationWork();  // I still need to be touched

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
	return true;  // true to keep workloop going

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
}  // end emulatorWorkLoop()

DEFINE_OTS_INTERFACE(ROCTrackerInterface)
