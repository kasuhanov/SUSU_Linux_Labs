#!/bin/bash

function ShowHelp()
{
	echo "Usage: txttrnsf [OPTIONS] FILE"
	echo "View basic info on current processes from procfs system."
	echo ""
	echo "  -s, -S, --space		squeeze spaces to a single space;"
	echo "  -m, -M, --minus		squeeze multiple minuses to a single dash;"
	echo "  -d, -D, --unix2dos	change unix newlines to dos;"
	echo "  -u, -U, --dos2unix	change dos newlines to unix;"
	echo "  -h, --help		display this help and exit;"
}

function Input()
{
	if [[ "$1" == "" ]]; then
		echo "txttrnsf: no options"
		echo "Try \`txttrnsf --help' for more information."
	        exit 1
	fi

	flag_S=false
	flag_M=false
	flag_D=false
	flag_U=false

	while [ "$1" != "" ]; do
	    case $1 in
	        -s | -S | --space )     flag_S=true
					;;
	        -m | -M | --minus )     flag_M=true
	                                ;;
	        -d | -D | --unix2dos )  if [ "$flag_U" == "true" ]; then
						echo "txttrnsf: conflicting options for newline transformations"
						echo "Try \`txttrnsf --help' for more information."
						exit 1
					fi
					flag_D=true
	                                ;;
	        -u | -U | --dos2unix )  if [ "$flag_D" == "true" ]; then
						echo "txttrnsf: conflicting options for newline transformations"
						echo "Try \`txttrnsf --help' for more information."
						exit 1
					fi
					flag_U=true
	                                ;;
	        -h | --help )           ShowHelp
	                                exit 0
	                                ;;
	        * )                     if [ "$2" == "" ]; then
						Run $1
					else
						echo "txttrnsf: unrecognized option '$1'"
						echo "Try \`txttrnsf --help' for more information."
		                                exit 1
					fi
	    esac
	    shift
	done
	echo "txttrnsf: no source file"
	echo "Try \`txttrnsf --help' for more information."
        exit 1
}


SqueezeSpaces()
{
	cat $1 | tr -s ' '
}

SqueezeMinusToDash()
{
	cat $1 | sed -E 's/-{2,}/—/g'
}

UNIX2DOS()
{
	cat $1 | sed 's/$/\r/g'
}

DOS2UNIX()
{
	cat $1 | sed 's/\r$//g'
}

Run()
{
	cat $1 > temp
	if [ "$flag_S" == "true" ]; then
		echo "SQUEEZING SPACES..."
		SqueezeSpaces temp > temp1
		cat temp1 > temp
	fi
	if [ "$flag_M" == "true" ]; then
		echo "SQUEEZING MULTIPLE MINUSES TO DASHES..."
		SqueezeMinusToDash temp > temp1
		cat temp1 > temp
	fi
	if [ "$flag_D" == "true" ]; then
		echo "TURNING UNIX NEWLINES TO DOS (LF to CRLF)..."		
		DOS2UNIX temp > temp1
		cat temp1 > temp		
		UNIX2DOS temp > temp1
		cat temp1 > temp
	fi
	if [ "$flag_U" == "true" ]; then
		echo "TURNING DOS NEWLINES TO UNIX (CRLF to LF)..."
		DOS2UNIX temp > temp1
		cat temp1 > temp
	fi
	cat temp > ${1}_trnsf
	rm -f temp
	rm -f temp1
	echo $1:
	cat $1 | od -c
	echo ${1}_trnsf:
	cat ${1}_trnsf | od -c
	exit 0
}

Input $@

exit 0;
