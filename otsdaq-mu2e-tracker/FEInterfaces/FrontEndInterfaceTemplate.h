#ifndef _ots_FrontEndInterfaceTemplate_h_
#define _ots_FrontEndInterfaceTemplate_h_

#include "otsdaq/DetectorWriter/FrontEndVirtualInterface.h"
#include <map>
#include <string>

namespace ots {
class FrontEndHardwareTemplate;
class FrontEndFirmwareTemplate;

class FrontEndInterfaceTemplate : public FEVInterface {
public:
  FrontEndInterfaceTemplate(std::string name);

  virtual ~FrontEndInterfaceTemplate(void);

  void configure(void);
  void halt(void);
  void pause(void);
  void resume(void);
  void start(std::string runNumber);
  void stop(void);
  bool running(void);

  int universalRead(char *address, char *readValue) override;
  void universalWrite(char *address, char *writeValue) override;

protected:
  FrontEndHardwareTemplate *theFrontEndHardware_;
  FrontEndFirmwareTemplate *theFrontEndFirmware_;
};
} // namespace ots

#endif
