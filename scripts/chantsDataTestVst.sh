#!/usr/bin/bash 
# adapted from Monica ( ~mu2etrk/test_stand/ots_mrb5/chantsDataTestVst.sh )
# assume that OTS and DTC have been setup
# MU2E_PCIE_UTILS_FQ_DIR=/cvmfs/mu2e.opensciencegrid.org/artexternals/mu2e_pcie_utils/v2_08_00/slf7.x86_64.e20.s118.prof
# export DTCLIB_DTC=0 # for plane setup
# export DTCLIB_DTC=1 # for annex setup
# source Setup_DTC.sh
#------------------------------------------------------------------------------
# configure the jitter attenuation
#------------------------------------------------------------------------------
source JAConfig.sh

echo "Finished configuring the jitter attenuator"
sleep 5

## change refclock to 200mhz
#mu2eUtil program_clock -C 0 -F 200000000
#sleep 5
#------------------------------------------------------------------------------
# enable CFO Emulator EVM and CLK markers
#------------------------------------------------------------------------------
my_cntl write 0x91f8 0x00003f3f >/dev/null

## enable RX and TX links for CFO + link 5, 4, 3, 2, 1, 0
#my_cntl write 0x9114 0x00007f7f >/dev/null

my_cntl write 0x9114 0x00000000 >/dev/null  # not needed for CFO emulation?? For sure EWM happen anyway...
my_cntl write 0x9144 0x00014141 >/dev/null  # set DMA timeout time (in 4 ns unit: 0x14141 = 0.33 ms)
my_cntl write 0x91BC 0x10       >/dev/null  # set number of null heartbeats at start
#------------------------------------------------------------------------------
# new Event Table data from Rick
#------------------------------------------------------------------------------
my_cntl write 0xa000 0

ii=$((0xa004))

while [ $ii -le $((0xa3FC)) ]; do
  my_cntl write $ii 1
  ii=$(( $ii + 4 ))
done
#------------------------------------------------------------------------------
# done setting the event table
#------------------------------------------------------------------------------
my_cntl write 0x9158 0x1      > /dev/null # set num of EVB destination nodes
my_cntl write 0x9100 0x808404 > /dev/null # avoid enabling CFO emulator

## now reset the ROC. This assumes ROC link 0.
#rocUtil -a 0 -w 0 write_register
## if you are using ROC link 1, in principle this should work (not tested)
#rocUtil -l 0x10 -a 0 -w 0 write_register

# added on 04/20/2021 otherwise no data request is sent!
# set emulator event mode bits (for non-null heartbeats)
my_cntl write 0x91c0 0xffffffff
my_cntl write 0x91c4 0xffffffff

DTCLIB_SIM_ENABLE=N 
