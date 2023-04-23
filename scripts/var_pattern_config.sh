# definition of success: read back reg 8 , see 16
LINK=$1
MODE=$2

echo " "
if [ $# -lt 2 ]; then
    echo "Need LINK and MODE argument"
    echo " "
    exit 2
elif [ $LINK -lt 0 -o $LINK -gt 5 ]; then
    echo "Bad LINK: range 0 to 5"
    echo " "
    exit 2
elif [ $MODE -lt 0 -o $MODE -gt 3 ]; then
    echo "Bad MODE: range 0 to 3"
    echo " "
    exit 2
fi


nseconds=10; if [ ".$3" != "." ] ; then nseconds=$3 ; fi

# ./ewm_disable.sh
# disable markers to make DCS commands more robust
echo "Disabling EWMs"

# default after reboot is 0x20
my_cntl write 0x91a8 0x0
echo "EWM deltaT set to "`my_cntl read 0x91a8`

## setup ROC for simulated increasing counter pattern      
echo " "
#reset link and reconfigure
echo "Resetting $LINK"
rocUtil -a 14 -w 1 -l $LINK write_register > /dev/null
sleep 1

echo "Configuring link $LINK to send patterns"
rocUtil -a 8  -w 16   -l $LINK write_register > /dev/null

echo "Configuring STATUS_BIT in MODE $MODE::"
if [ $MODE -eq 0 ]; then
    echo " ie STATUS_BIT=0x55"
elif [ $MODE -eq 1 ]; then
    echo " ie STATUS_BIT[0]=DREQ_EMPY, STATUS_BIT[0]=DREQ_EMPY, BIT[1]=DREQ_FULL, BIT[2]=ETFIFO_FULL, STATUS_BIT[3]=DDR_WRAP"
elif [ $MODE -eq 2 ]; then
    echo " ie STATUS_BIT=DREQ_WRCNT"
elif [ $MODE -eq 3 ]; then
    echo " ie STATUS_BIT=EWM_COUNTER"
fi
rocUtil -a 30  -w $MODE  -l $LINK write_register > /dev/null
echo ... now sleep for $nseconds seconds
sleep $nseconds
