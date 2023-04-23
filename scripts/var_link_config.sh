if [ $# -lt 2 ]; then
    echo "link_enable command expects two argument: LINK_NO and USE_LANE mask"
fi

# link     : the fiber number (0-5)
# use_lane : mask, defines lanes to be used (2 FPGA's x 2 lanes per FPGA)

LINK=$1
USE_LANE=$2

# disable markers to make DCS commands more robust
./ewm_disable.sh

echo " "
#reset link and reconfigure
echo "Resetting and configuring link $LINK"
rocUtil -a 14 -w 1 -l $LINK write_register > /dev/null
sleep 1


if [ $USE_LANE -eq 1 ]; then
    echo "to receive data only from CAL lane 0"
elif [ $USE_LANE -eq 5 ]; then 
    echo "to receive data from both CAL lanes"
elif [ $USE_LANE -eq 15 ]; then 
    echo "to receive data from all 4 lanes"
fi  


# after adding external clock and evmarker control to the ROC,
# one needs to write bit(8)=1 and bit(9)=1 on register 8, ie 0x300 (0r 768)
declare -i ENABLES=768      # this declare an integer
SET_LANE=$(( $USE_LANE + $ENABLES ))
#SET_LANE=$(($USE_LANE))
echo "SET_LANE=" $SET_LANE

# setup ROC for SERDES lane inputs      
rocUtil -a 8  -w $SET_LANE   -l $LINK write_register > /dev/null   # only lane 0


