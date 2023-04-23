
LINK=$1

echo " "
if [ $# -lt 1 ]; then
    echo "Need LINK argument"
    echo " "
    exit 2
elif [ $LINK -lt 0 -o $LINK -gt 5 ]; then
    echo "Bad LINK: range 0 to 5"
    echo " "
    exit 2
fi


# disable markers to make DCS commands more robust
source ./scripts/ewm_disable.sh


for REG in 0 8 18
do
  RD=`rocUtil -a $REG -l $LINK  simple_read`
  echo "Register $REG link $LINK reads $RD"
done

echo " "

WR0=`rocUtil -a 23 -l $LINK  simple_read`
WR1=`rocUtil -a 24 -l $LINK  simple_read`
declare -a MYWR0
MYWR0=($WR0)
declare -a MYWR1
MYWR1=($WR1)
printf -v wrhex '0x%x' $(($((65536*${MYWR1[1]}))+${MYWR0[1]}))
echo "SIZE_FIFO_FULL[28]+STORE_POS[25:24]+STORE_CNT[19:0]:" $wrhex

RD0=`rocUtil -a 25 -l $LINK  simple_read`
RD1=`rocUtil -a 26 -l $LINK  simple_read`
declare -a MYRD0
MYRD0=($RD0)
declare -a MYRD1
MYRD1=($RD1)
#echo "MYRD1 has ${#MYRD1[*]} elements"
#echo "MYRD1 first element is ${MYWR1[0]}"
#echo "MYRD1 second element is ${MYRD1[1]}"
printf -v rdhex '0x%x' $(($((65536*${MYRD1[1]}))+${MYRD0[1]}))
echo "SIZE_FIFO_EMPTY[28]+FETCH_POS[25:24]+FETCH_CNT[19:0]:" $rdhex

echo " "

EVM0=`rocUtil -a 64 -l $LINK  simple_read`
EVM1=`rocUtil -a 65 -l $LINK  simple_read`
declare -a MYEVM0
MYEVM0=($EVM0)
declare -a MYEVM1
MYEVM1=($EVM1)
printf -v evmhex '%d' $(($((65536*${MYEVM1[1]}))+${MYEVM0[1]}))
#printf -v evmhex '0x%x' $(($((65536*${MYEVM1[1]}))+${MYEVM0[1]}))
echo "no. EVM seen:" $evmhex

HBS0=`rocUtil -a 27 -l $LINK  simple_read`
HBS1=`rocUtil -a 28 -l $LINK  simple_read`
declare -a MYHBS0
MYHBS0=($HBS0)
declare -a MYHBS1
MYHBS1=($HBS1)
printf -v hbshex '%d' $(($((65536*${MYHBS1[1]}))+${MYHBS0[1]}))
#printf -v hbshex '0x%x' $(($((65536*${MYHBS1[1]}))+${MYHBS0[1]}))
echo "no. HB seen:" $hbshex


HBN0=`rocUtil -a 29 -l $LINK  simple_read`
HBN1=`rocUtil -a 30 -l $LINK  simple_read`
declare -a MYHBN0
MYHBN0=($HBN0)
declare -a MYHBN1
MYHBN1=($HBN1)
printf -v hbnhex '%d' $(($((65536*${MYHBN1[1]}))+${MYHBN0[1]}))
#printf -v hbnhex '0x%x' $(($((65536*${MYHBN1[1]}))+${MYHBN0[1]}))
echo "no. null HB seen:" $hbnhex

HBH0=`rocUtil -a 31 -l $LINK  simple_read`
HBH1=`rocUtil -a 32 -l $LINK  simple_read`
declare -a MYHBH0
MYHBH0=($HBH0)
declare -a MYHBH1
MYHBH1=($HBH1)
printf -v hbhhex '%d' $(($((65536*${MYHBH1[1]}))+${MYHBH0[1]}))
#printf -v hbhhex '0x%x' $(($((65536*${MYHBH1[1]}))+${MYHBH0[1]}))
echo "no. HB on hold:" $hbhhex

PRE0=`rocUtil -a 33 -l $LINK  simple_read`
PRE1=`rocUtil -a 34 -l $LINK  simple_read`
declare -a MYPRE0
MYPRE0=($PRE0)
declare -a MYPRE1
MYPRE1=($PRE1)
printf -v prehex '%d' $(($((65536*${MYPRE1[1]}))+${MYPRE0[1]}))
#printf -v prehex '0x%x' $(($((65536*${MYPRE1[1]}))+${MYPRE0[1]}))
echo "no. PREFETCH seen:" $prehex

DRE0=`rocUtil -a 35 -l $LINK  simple_read`
DRE1=`rocUtil -a 36 -l $LINK  simple_read`
declare -a MYDRE0
MYDRE0=($DRE0)
declare -a MYDRE1
MYDRE1=($DRE1)
printf -v drehex '%d' $(($((65536*${MYDRE1[1]}))+${MYDRE0[1]}))
#printf -v drehex '0x%x' $(($((65536*${MYDRE1[1]}))+${MYDRE0[1]}))
echo "no. DATA REQ seen:" $drehex

DRER0=`rocUtil -a 37 -l $LINK  simple_read`
DRER1=`rocUtil -a 38 -l $LINK  simple_read`
declare -a MYDRER0
MYDRER0=($DRER0)
declare -a MYDRER1
MYDRER1=($DRER1)
printf -v drerhex '%d' $(($((65536*${MYDRER1[1]}))+${MYDRER0[1]}))
#printf -v drerhex '0x%x' $(($((65536*${MYDRER1[1]}))+${MYDRER0[1]}))
echo "no. DATA REQ read from DDR:" $drerhex

DRES0=`rocUtil -a 39 -l $LINK  simple_read`
DRES1=`rocUtil -a 40 -l $LINK  simple_read`
declare -a MYDRES0
MYDRES0=($DRES0)
declare -a MYDRES1
MYDRES1=($DRES1)
printf -v dreshex '%d' $(($((65536*${MYDRES1[1]}))+${MYDRES0[1]}))
#printf -v dreshex '0x%x' $(($((65536*${MYDRES1[1]}))+${MYDRES0[1]}))
echo "no. DATA REQ sent to DTC:" $dreshex

DREN0=`rocUtil -a 41 -l $LINK  simple_read`
DREN1=`rocUtil -a 42 -l $LINK  simple_read`
declare -a MYDREN0
MYDREN0=($DREN0)
declare -a MYDREN1
MYDREN1=($DREN1)
printf -v drenhex '%d' $(($((65536*${MYDREN1[1]}))+${MYDREN0[1]}))
#printf -v drenhex '0x%x' $(($((65536*${MYDREN1[1]}))+${MYDREN0[1]}))
echo "no. DATA REQ with null data:" $drenhex

echo " "

SPILLTAG0=`rocUtil -a 43 -l $LINK  simple_read`
SPILLTAG1=`rocUtil -a 44 -l $LINK  simple_read`
declare -a MYSPILL0
MYSPILL0=($SPILLTAG0)
declare -a MYSPILL1
MYSPILL1=($SPILLTAG1)
printf -v spillhex '0x%x' $(($((65536*${MYSPILL1[1]}))+${MYSPILL0[1]}))
echo "last SPILL TAG:" $spillhex


HBTAG0=`rocUtil -a 45 -l $LINK  simple_read`
HBTAG1=`rocUtil -a 46 -l $LINK  simple_read`
HBTAG2=`rocUtil -a 47 -l $LINK  simple_read`
declare -a MYHBTAG0
MYHBTAG0=($HBTAG0)
declare -a MYHBTAG1
MYHBTAG1=($HBTAG1)
printf -v hbtaghex '0x%x' $(($((65536*${MYHBTAG1[1]}))+${MYHBTAG0[1]}))
echo "last HB tag:" $hbtaghex


PRETAG0=`rocUtil -a 48 -l $LINK  simple_read`
PRETAG1=`rocUtil -a 49 -l $LINK  simple_read`
PRETAG2=`rocUtil -a 50 -l $LINK  simple_read`
declare -a MYPRETAG0
MYPRETAG0=($PRETAG0)
declare -a MYPRETAG1
MYPRETAG1=($PRETAG1)
printf -v pretaghex '0x%x' $(($((65536*${MYPRETAG1[1]}))+${MYPRETAG0[1]}))
echo "last PREFETCH tag:" $pretaghex


FETAG0=`rocUtil -a 51 -l $LINK  simple_read`
FETAG1=`rocUtil -a 52 -l $LINK  simple_read`
FETAG2=`rocUtil -a 53 -l $LINK  simple_read`
declare -a MYFETAG0
MYFETAG0=($FETAG0)
declare -a MYFETAG1
MYFETAG1=($FETAG1)
printf -v fetaghex '0x%x' $(($((65536*${MYFETAG1[1]}))+${MYFETAG0[1]}))
echo "last FETCHED tag:" $fetaghex

DRETAG0=`rocUtil -a 54 -l $LINK  simple_read`
DRETAG1=`rocUtil -a 55 -l $LINK  simple_read`
DRETAG2=`rocUtil -a 56 -l $LINK  simple_read`
declare -a MYDRETAG0
MYDRETAG0=($DRETAG0)
declare -a MYDRETAG1
MYDRETAG1=($DRETAG1)
printf -v dretaghex '0x%x' $(($((65536*${MYDRETAG1[1]}))+${MYDRETAG0[1]}))
echo "last DATA REQ tag:" $dretaghex


OFFTAG0=`rocUtil -a 57 -l $LINK  simple_read`
OFFTAG1=`rocUtil -a 58 -l $LINK  simple_read`
OFFTAG2=`rocUtil -a 59 -l $LINK  simple_read`
declare -a MYOFFTAG0
MYOFFTAG0=($OFFTAG0)
declare -a MYOFFTAG1
MYOFFTAG1=($OFFTAG1)
printf -v offtaghex '0x%x' $(($((65536*${MYOFFTAG1[1]}))+${MYOFFTAG0[1]}))
echo "OFFSET tag:" $offtaghex


#FULLTAG0=`rocUtil -a 66 -l $LINK  simple_read`
#FULLTAG1=`rocUtil -a 67 -l $LINK  simple_read`
#FULLTAG2=`rocUtil -a 68 -l $LINK  simple_read`
#declare -a MYFULLTAG0
#MYFULLTAG0=($FULLTAG0)
#declare -a MYFULLTAG1
#MYFULLTAG1=($FULLTAG1)
#printf -v fulltaghex '0x%x' $(($((65536*${MYFULLTAG1[1]}))+${MYFULLTAG0[1]}))
#echo "DREQ_FULL tag:" $fulltaghex
