echo " "
echo "Clear Link0 counters:"
my_cntl write 0x9630 0x1 > /dev/null
my_cntl write 0x9650 0x1 > /dev/null
my_cntl write 0x9670 0x1 > /dev/null
my_cntl write 0x9690 0x1 > /dev/null
my_cntl write 0x9560 0x1 > /dev/null

#echo " "
#echo "Clear Link1 counters:"
#my_cntl write 0x9634 0x0 > /dev/null
#my_cntl write 0x9654 0x0 > /dev/null
#my_cntl write 0x9674 0x0 > /dev/null
#my_cntl write 0x9694 0x0 > /dev/null



