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

  __CFG_COUT__ << "Constructor..." << __E__;

  try {
    inputTemp_ = getSelfNode().getNode("inputTemperature").getValue<double>();
  } catch (...) {
    __CFG_COUT__ << "inputTemperature field not defined. Defaulting..."
                 << __E__;
    inputTemp_ = 15.;
  }

  temp1_.noiseTemp(inputTemp_);
}

//==========================================================================================
ROCTrackerInterface::~ROCTrackerInterface(void) {}

//==================================================================================================
void ROCTrackerInterface::writeEmulatorRegister(unsigned address,
                                                unsigned data_to_write)
{


  __CFG_COUT__ << "emulator write" << __E__;

  return;

} // end writeRegister()

//==================================================================================================
int ROCTrackerInterface::readEmulatorRegister(unsigned address)
{
  __CFG_COUT__ << "emulator read" << __E__;

  if (address == ADDRESS_FIRMWARE_VERSION)
    return 0x5;
  else if (address == ADDRESS_MYREGISTER)
    return temp1_.GetBoardTempC();
  else
    return -1;

} // end readRegister()

//==================================================================================================
// return false to stop workloop thread
bool ROCTrackerInterface::emulatorWorkLoop(void)
{
  __CFG_COUT__ << "emulator working..." << __E__;

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
