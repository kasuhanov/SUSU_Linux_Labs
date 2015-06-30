#!/bin/bash

function GetPass()
{
	while [ "${n:=1}" -le "$1" ]
	do
       		PASS=$PASS`head --lines 1 /dev/urandom | base64 | tr -dc _A-Z-a-z-0-9 | head -c 1`
        	let n+=1
	done

	echo "$PASS"
}

function ShowHelp()
{
	echo "Usage: PassGen [OPTION]"
	echo "Generate random 8-character string suitable for a password, and print it to the standard output."
	echo ""
	echo "Mandatory arguments to long options are mandatory for short options too."
	echo "  -c, -C, --count=K	output string of K characters, instead of default 8;"
	echo "  -h, --help		display this help and exit;"

}

PASS=""
LENGTH="8";

while [ "$1" != "" ]; do
    case $1 in
        -c | -C |--count)       if [ "$2" = "" ]; then
					echo "PassGen: option requires an argument -- '$1'"
					echo "Try \`PassGen --help' for more information."
					exit 1
				else
					LENGTH="$2"
					shift
				fi
				
                                ;;
        -h | --help )           ShowHelp
                                exit 0
                                ;;
        * )                     echo "PassGen: unrecognized option '$1'"
				echo "Try \`PassGen --help' for more information."
                                exit 1
    esac
    shift
done

if ! [[ $LENGTH =~ ^[0-9]+$ ]]; then
	echo "PassGen: $LENGTH: invalid length of a password"
	exit 1
fi

GetPass "$LENGTH"

exit 0
