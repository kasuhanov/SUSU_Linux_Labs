#!/bin/bash

FIRSTROW=0
FIRSTCOL=0
LASTROW=11
LASTCOL=11
MSGROW=13
DIRECTIONROW=12
SCOREROW=14
POSX=1
POSY=1

DELAY=0.2

OBJX=5
OBJY=5

SCORE=0

DrawField()
{
	tput setf 3
	tput cup $FIRSTROW $FIRSTCOL
	cat field.gamefile
}

ShowSCORE()
{	
	tput cup $SCOREROW 0
	echo "           "
	tput setf 1
	echo -n "@: "
	tput setf 7
	echo "$SCORE" | cut -c1-10
}

DrawPlayer()
{
	tput setf 2
	tput cup $POSY $POSX
	echo "&"
}

ClearPlayer()
{
	tput cup $POSY $POSX
	echo " "
}

DrawObject()
{
	tput setf 1
	tput cup $OBJY $OBJX
	echo "@"
}

ClearObject()
{
	tput cup $OBJY $OBJX
	echo " "
	OBJX=$LASTCOL
	OBJY=$LASTROW	
}

Move()
{
	NEW_POSX=$POSX
	NEW_POSY=$POSY
	case "$DIRECTION" in
		u)	NEW_POSY=$(( $POSY - 1 ))
			ShowDIRECTION "You are facing UP"
			;;
		d)	NEW_POSY=$(( $POSY + 1 ))
			ShowDIRECTION "You are facing DOWN"
			;;
		l)	NEW_POSX=$(( $POSX - 1 ))
			ShowDIRECTION "You are facing LEFT"
			;;
		r)	NEW_POSX=$(( $POSX + 1 ))
			ShowDIRECTION "You are facing RIGHT"
			;;
	esac

	TEST_RES=$(TestPosition $NEW_POSX $NEW_POSY)
	if [ "$TEST_RES" == 1 ]; then
		ShowMSG "That's the wall. Yup."
		NEW_POSX=$POSX
		NEW_POSY=$POSY
	elif [ "$TEST_RES" == 2 ]; then
		ShowMSG "That's an object. Press E to pick up!"
	elif [ "$TEST_RES" == 0 ]; then	
		ClearPlayer
		POSX=$NEW_POSX
		POSY=$NEW_POSY
		DrawPlayer
		ShowMSG "Keep on moving!"
	fi

	ShowSCORE
}

TestPosition()
{
	if [ "$1" -le "$FIRSTCOL" ] || [ "$1" -ge "$LASTCOL" ]; then
		echo 1
	elif [ "$2" -le "$FIRSTROW" ] || [ "$2" -ge "$LASTROW" ]; then
		echo 1
	elif [ "$1" == "$OBJX" ] && [ "$2" == "$OBJY" ]; then
		echo 2
	else
		echo 0	
	fi
}

PickUp()
{
	NEW_POSX=$POSX
	NEW_POSY=$POSY
	case "$DIRECTION" in
		u)	NEW_POSY=$(( $POSY - 1 ))
			ShowDIRECTION "You are facing UP"
			;;
		d)	NEW_POSY=$(( $POSY + 1 ))
			ShowDIRECTION "You are facing DOWN"
			;;
		l)	NEW_POSX=$(( $POSX - 1 ))
			ShowDIRECTION "You are facing LEFT"
			;;
		r)	NEW_POSX=$(( $POSX + 1 ))
			ShowDIRECTION "You are facing RIGHT"
			;;
	esac

	TEST_RES=$(TestPosition $NEW_POSX $NEW_POSY)

	if [ "$TEST_RES" == 2 ]; then
		ShowMSG "Picked up an object. Score!"
		ClearObject
		SCORE=$(( $SCORE + 1 ))
		ShowSCORE
	elif [ "$TEST_RES" == 1 ]; then
		ShowMSG "Picking up walls? Can't do!"
	elif [ "$TEST_RES" == 0 ]; then
		ShowMSG "Nothing to pick up."
	fi
}

Drop()
{
	if [ "$SCORE" == "0" ]; then
		ShowMSG "Nothing to drop."
	else
		NEW_POSX=$POSX
		NEW_POSY=$POSY
		case "$DIRECTION" in
			u)	NEW_POSY=$(( $POSY - 1 ))
				;;
			d)	NEW_POSY=$(( $POSY + 1 ))
				;;
			l)	NEW_POSX=$(( $POSX - 1 ))
				;;
			r)	NEW_POSX=$(( $POSX + 1 ))
				;;
		esac
		if [ "$(TestPosition $NEW_POSX $NEW_POSY)" == "0" ]; then
			OBJX=$NEW_POSX
			OBJY=$NEW_POSY
			SCORE=$(( $SCORE - 1 ))
			DrawObject
			ShowMSG "You dropped the object. Why would you do this?"
			ShowSCORE
		elif [ "$(TestPosition $NEW_POSX $NEW_POSY)" == "1" ]; then
			ShowMSG "This ain't no Castelvania - no objects in walls!"
		fi
	fi	
}

ShowMSG()
{
	tput setf 7
	tput cup $MSGROW 0
	stty echo
	for i in `seq 1 50`; do
		echo -ne " "
	done
	tput cup $MSGROW 0	
	printf %s "$1" | cut -c1-50
	stty -echo
}

ShowDIRECTION()
{

	tput setf 7
	tput cup $DIRECTIONROW 0
	stty echo
	for i in `seq 1 50`; do
		echo -ne " "
	done
	tput cup $DIRECTIONROW 0	
	echo -n "$1" | cut -c1-50
	stty -echo
}

reset
tput bold
tput setf 7
echo "
Keys:

	W - UP
	S - DOWN
	A - LEFT
	D - RIGHT
	E - PICK UP OBJECT
	R - DROP OBJECT
	X - QUIT

Press Return to continue
"

stty -echo
tput civis
read RTN
clear
DrawField
DrawPlayer
DrawObject
ShowDIRECTION
ShowSCORE


while :
do
	read -t 0 -s discard
	read -s -n 1 key
	case "$key" in
		w)	DIRECTION="u"
			Move
			;;
		s)	DIRECTION="d"
			Move
			;;
		a)	DIRECTION="l"
			Move
			;;
		d) 	DIRECTION="r"
			Move
			;;
		r) 	Drop
			;;
		e) 	PickUp
			;;
		x) 	ShowMSG "Quitting..."
		        stty echo
			tput cvvis
		        reset
		        exit 0
		        ;;
	esac
done
