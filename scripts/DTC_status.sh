echo " "
echo "Reading DTC version (reg 0x9004)  : "`my_cntl read 0x9004`
echo " "
echo "Reading DTC enables (reg 0x9100)  : "`my_cntl read 0x9100`
echo " "
echo "Reading fiber locked (reg 0x9140) : "`my_cntl read 0x9140`
echo " "
echo "Reading DTC timeout (reg 0x9144) : "`my_cntl read 0x9144`
echo " "
echo "Reading EW marker deltaT (reg 0x91A8)  : "`my_cntl read 0x91A8`
echo " "
echo "Reading #DREQs (reg 0x91AC)            : "`my_cntl read 0x91AC`
echo " "
echo "Reading Clk marker deltaT (reg 0x91F4)  : "`my_cntl read 0x91F4`
echo " "
echo "Reading data timeout length (reg 0x9188): "`my_cntl read 0x9188`
echo " "
