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
 

  //try {
  //  inputTemp_ = getSelfNode().getNode("inputTemperature").getValue<double>();
  //} catch (...) {
  //  __CFG_COUT__ << "inputTemperature field not defined. Defaulting..."
  //               << __E__;
  //  inputTemp_ = 15.;
  //}

  //temp1_.noiseTemp(inputTemp_);
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

	__CFG_COUT__ << "Tracker configure, next configure front-end... " << __E__;
	__MCOUT_INFO__("..... Vadim, if you tell me what register to write, I could write parameter 1 = " << TrackerParameter_1_ << __E__);
	__MCOUT_INFO__("..... followed by parameter 2 = " << TrackerParameter_2_ << __E__);

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
