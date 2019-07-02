#ifndef _ots_ROCTrackerInterface_h_
#define _ots_ROCTrackerInterface_h_

#include <fstream>
#include <iostream>
#include <string>

#include "otsdaq-mu2e/FEInterfaces/ROCPolarFireCoreInterface.h"

namespace ots
{
class ROCTrackerInterface : public ROCPolarFireCoreInterface
{
	// clang-format off
public:
  	ROCTrackerInterface(const std::string &rocUID,
						const ConfigurationTree &theXDAQContextConfigTree,
						const std::string &interfaceConfigurationPath);

	~ROCTrackerInterface(void);

	// state machine
  	//----------------
  	void 									configure				(void) override;
  	void 									start					(std::string runNumber) override;
  	void 									stop					(void) override;
  	bool 									running					(void) override;

  	// write and read to registers
  	virtual void 							writeEmulatorRegister	(uint16_t address, uint16_t data_to_write) override;
  	virtual int 							readEmulatorRegister	(uint16_t address) override;



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

		unsigned int number_of_good_events_;
		unsigned int number_of_bad_events_;
		unsigned int number_of_empty_events_;
		std::ofstream datafile_;
		unsigned int event_number_;

  public:
	void ReadTrackerFIFO(__ARGS__);

	// clang-format on
};

}  // namespace ots

#endif
