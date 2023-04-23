echo "Enabling EWM with 25.6 us internal"

my_cntl write 0x91a8 0x400
echo "0x91a8 set to "`my_cntl read 0x91a8`
echo " "
