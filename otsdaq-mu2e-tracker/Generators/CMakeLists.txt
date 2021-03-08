
simple_plugin(TrackerVST "generator"
  mu2e-artdaq-core_Overlays
  artdaq-utilities_Plugins
  artdaq-core_Utilities
  artdaq_Generators
  artdaq_DAQdata
  artdaq_DAQrate
  artdaq-core_Data
  ${CANVAS_LIB_LIST}
  ${FHICLCPP}
  ${CETLIB}
  ${CETLIB_EXCEPT}
  MF_MessageLogger
  DTCInterface
	${Boost_SYSTEM_LIBRARY}
)
  
