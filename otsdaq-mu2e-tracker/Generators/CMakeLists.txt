
include(artdaq::commandableGenerator)

cet_build_plugin(TrackerVST artdaq::commandableGenerator LIBRARIES REG artdaq_core_mu2e::Overlays canvas::canvas
)
  
