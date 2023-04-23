echo "Disabling EWMs"

# default after reboot is 0x20
my_cntl write 0x91a8 0x0
echo "reg 0x91a8 : EWM deltaT set to "`my_cntl read 0x91a8`
echo " "

# default after reboot is 0x800
#my_cntl write 0x9144 0x1000
#echo "DTC timeout set to "`my_cntl read 0x9144`
#echo " "
