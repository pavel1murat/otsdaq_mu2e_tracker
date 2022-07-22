
include(otsdaq::FEInterface)

cet_build_plugin(ROCTrackerInterface otsdaq::FEInterface LIBRARIES REG otsdaq_mu2e::ROCPolarFireCoreInterface
 )
 

install_headers()
install_source()