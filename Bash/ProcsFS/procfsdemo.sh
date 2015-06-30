#!/bin/bash

function ShowHelp()
{
	echo "Usage: ProcfsDemo [OPTION]"
	echo "View basic info on current processes from procfs system."
	echo ""
	echo "  -m, -M, --memory	display process memory usage;"
	echo "  -n, -N, --name		display process name usage;"
	echo "  -s, -S, --state		display process state;"
	echo "  -h, --help		display this help and exit;"
}

function Input()
{
	flag_S=false
	flag_N=false
	flag_M=false
	if [[ "$1" == "" ]]; then
		flag_S=true
		flag_N=true
		flag_M=true	
	fi	


	while [ "$1" != "" ]; do
	    case $1 in
	        -n | -N | --name )      flag_N=true
	                                ;;
	        -s | -S | --state )     flag_S=true
	                                ;;
	        -m | -M | --memory )    flag_M=true
	                                ;;
	        -h | --help )           ShowHelp
	                                exit 0
	                                ;;
	        * )                     echo "ProcfsDemo: unrecognized option '$1'"
					echo "Try \`ProcfsDemo --help' for more information."
	                                exit 1
	    esac
	    shift
	done
}

function Run()
{	
	folder="/proc"
	pDATA="PID"
	if [[ $flag_N = true ]]; then
		pDATA=$pDATA$'\t'"|"$'\t'"Name"
	fi
	if [[ $flag_S = true ]]; then
		pDATA=$pDATA$'\t'"|"$'\t'"S (state)"
	fi				
	if [[ $flag_M = true ]]; then
		pDATA=$pDATA$'\t'"|"$'\t'"Memory"
	fi
	echo $pDATA
	ls -1 $folder | sort -n | while read -r pid ; do
		if [[ $pid =~ ^[0-9]+$ ]]; then
			if [ -f $folder"/"$pid"/status" ]; then
				pNAME="N/A";
				pPID="N/A";
				pSTATE="N/A";
				pMEM="N/A";
				while read -r line ; do
					if [[ $line =~ ^Name:.*$ ]]; then
						pNAME=$(echo $line | cut -d' ' -f2-)
						continue
					fi
					if [[ $line =~ ^Pid:.*$ ]]; then
						pPID=$(echo $line | cut -d' ' -f2-)
						continue
					fi
					if [[ $line =~ ^State:.*$ ]]; then
						pSTATE=$(echo $line | cut -d' ' -f2-)
						continue
					fi
					if [[ $line =~ ^VmSize:.*$ ]]; then
						pMEM=$(echo $line | cut -d' ' -f2-)
						continue
					fi
					#echo $line
				done < $folder"/"$pid"/status"
				pDATA=$pPID
				if [[ $flag_N = true ]]; then
					pDATA=$pDATA$'\t'"|"$'\t'$pNAME
				fi
				if [[ $flag_S = true ]]; then
					pDATA=$pDATA$'\t'"|"$'\t'$pSTATE
				fi				
				if [[ $flag_M = true ]]; then
					pDATA=$pDATA$'\t'"|"$'\t'$pMEM
				fi
				echo $pDATA
			fi
		fi
	done
}

Input $@
echo Building list of processes now...
Run | tr -s " " | cut -d$'\t' -f1- | column -tx -s ' '

exit 0
