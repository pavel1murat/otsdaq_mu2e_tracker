#ifndef _ots_ROCTrackerInterface_h_
#define _ots_ROCTrackerInterface_h_

#include <fstream>
#include <iostream>
#include <string>

#include "otsdaq-mu2e/FEInterfaces/ROCPolarFireCoreInterface.h"

namespace ots {
class ROCTrackerInterface : public ROCPolarFireCoreInterface {
public:
  	ROCTrackerInterface(const std::string &rocUID,
						const ConfigurationTree &theXDAQContextConfigTree,
						const std::string &interfaceConfigurationPath);

	~ROCTrackerInterface(void);

	// write and read to registers
	virtual void writeEmulatorRegister(unsigned address,
										unsigned data_to_write) override;
	virtual int readEmulatorRegister(unsigned address) override;

	virtual void configure(void) override;

	bool emulatorWorkLoop(void) override;

	enum {
		ADDRESS_FIRMWARE_VERSION = 5,
		ADDRESS_MYREGISTER = 0x65,
  	};

  	//	temperature--
  	class Thermometer {
  		private:
    		double mnoiseTemp;

  		public:
    		void noiseTemp(double intemp) {
      			mnoiseTemp = (double)intemp +
                   0.05 * (intemp * ((double)rand() / (RAND_MAX)) - 0.5);
      			return;
    		}
    		double GetBoardTempC() { return mnoiseTemp; }
  	};

  	Thermometer temp1_;
  	double inputTemp_;

	private:
		unsigned int TrackerParameter_1_;
		bool TrackerParameter_2_;
  public:
	void ReadTrackerFIFO(__ARGS__);

};

} // namespace ots

#endif
