#!/bin/bash

function ShowHelp()
{
	echo "Usage: SysLab1 [OPTIONS]"
	echo "Basic system administration tools;"
	echo ""
	echo "  --mdIOpp		show most drive IO per process;"
	echo "  --mdIOpd		show most drive IO per drive;"
	echo "  --mCPUpp		show most CPU usage per process;"
	echo "  --mFOpp			show most files open per process;"
	echo "  --apFL			show what libraries active processes come from;"
	echo "  --iu			show inactive users and groups;"
	echo "  --pLf			show libraries used by process (requires -p PID) or by executable file (requires PATH);"
	echo "  --wSItsf		execute specified script and add system version annotation as a comment at it's end;"
	echo "  -h, --help		display this help and exit;"
}

function MostDriveIOPerProcess()
{
	sudo rm -f temp
	sudo iotop -o --delay=1 -P --iter=10 --batch -qqq >> temp
	echo "Most write:"
	echo | cat temp | tr -s " " | column -t -s ' ' | sort -nr -k 6 | tr -s " " | cut -d' ' -f1,4-7,12- | column -t -s ' ' | head -n 1
	echo "Most read:"
	echo | cat temp | tr -s " " | column -t -s ' ' | sort -nr -k 4 | tr -s " " | cut -d' ' -f1,4-7,12- | column -t -s ' ' | head -n 1
	sudo rm -f temp
}

function MostDriveIOPerDrive()
{
	echo "Device:  tps   kB_read/s  kB_wrtn/s  kB_read  kB_wrtn" > temp
	echo | iostat -d | tail -n +4 | column -t -s ' ' | sort -nr -k 2 | tr -s " " | cut -d' ' -f1-6 | head -n 1 >> temp
 	cat temp | column -t -s ' '
	sudo rm -f temp	
}

function MostCPUPerProcess()
{
	echo "PID %CPU COMMAND" > temp
	echo | ps -e -o pid,pcpu,args  | tail -n +1 | column -t -s ' ' | sort -nr -k 2 | tr -s " " | cut -d' ' -f1-3 | column -t -s ' ' | 		head -n 1 >> temp
	cat temp | column -t -s ' '
	sudo rm -f temp
}

function MostFilesOpenPerProcess()
{
	echo "FILES COMMAND" > temp
	lsof | column -t -s ' ' | tr -s " " | cut -d ' ' -f1 | sort | uniq -c | sort -nr -k 1 | column -t -s ' ' | tr -s " " | cut -d' ' 		-f1,2 | head -n 1 >> temp
	cat temp | column -t -s ' '
	sudo rm -f temp	
}

function ActiveProcessFromLib()
{
	sudo ps -e -o pid,args | tail -n +2 | tr -s " " | while read -r line ; do
		echo $line
		echo Libraries:
		filename=$(echo $line | cut -d' ' -f2);
		if [[ $filename != [* ]]; then
			dpkg -S $filename
		else
			echo CORE THREAD
		fi
		echo ""
	done
}

function InactiveUsers()
{
	sudo who -u | cut -d' ' -f1 | uniq | sort > temp1
	sudo cat /etc/shadow | cut -d':' -f1 | sort > temp2
	comm -13 temp1 temp2 > temp3
	rm -f temp1
	rm -f temp2
	echo "INACTIVE:"
	sudo cat /etc/gshadow | cut -d':' -f1,4 | while read -r line ; do
		groupname=$(echo $line | cut -d':' -f1)
		users=$(echo $line | cut -d':' -f2)
		if [[ "$users" != "" ]]; then
			rm -f temp4
			echo $users | tr "," "\n" | sort > temp4
			difference=$(comm -13 temp3 temp4)
			if [[ "$difference" == "" ]]; then
				echo "GROUP $groupname; USERS: $users"
				comm -12 temp3 temp4 >> temp1 
			else
				comm -12 temp3 temp4 >> temp2
			fi
			rm -f temp4
		fi
	done
	comm temp1 temp2 | uniq | sort > temp4
	comm -23 temp3 temp4 > temp5
	comm -12 temp1 temp2 > temp4
	comm temp4 temp5 | uniq | sort | while read -r line ; do
		echo "USER $line"
	done
	rm -f temp1
	rm -f temp2
	rm -f temp3
	rm -f temp4
	rm -f temp5
}

function ProcessLibsFromFile()
{
	echo "Libraries for file $1":
	ldd $1
}

function ProcessLibsFromPID()
{
	echo "Libraries for PID $1 :"
	rm -f temp
	pmap $1 | tr -s " " | cut -d' ' -f4 | while read -r line ; do
		if [[ "$line" != "[" ]]; then
			echo $line >> temp
		fi
	done
	cat temp | uniq
	rm -f temp
}

function WriteSysInfoToScriptFile()
{
	echo "Starting $1"
	bash $1
	res=$(echo $?)
	echo "Result is $res"
	if [[ "$res" == "0" ]]; then
		echo "# $(cat /proc/version)" >> $1
	fi	
}

function Input()
{
	if [[ "$1" == "" ]]; then
		echo "SysLab1: no options"
		echo "Try \`SysLab1 --help' for more information."
	        exit 1
	fi

	while [ "$1" != "" ]; do
	    case $1 in
	        --mdIOpp )  		MostDriveIOPerProcess
					exit 0
					;;
	        --mdIOpd )		MostDriveIOPerDrive
					exit 0
	                                ;;
	        --mCPUpp )  		MostCPUPerProcess
					exit 0
	                                ;;
	        --mFOpp ) 		MostFilesOpenPerProcess
					exit 0
	                                ;;
	        --apFL ) 		ActiveProcessFromLib
					exit 0
	                                ;;
	        --iu )		 	InactiveUsers
					exit 0
	                                ;;
	        --pLf )		 	if [ "$2" == "-p" ]; then
						if [ "$3" != "" ]; then
							ProcessLibsFromPID $3
							shift
							shift
							exit 0
						fi
						echo "SysLab1: option requires an argument -- '$1 $2'"
						echo "Try \`SysLab1 --help' for more information."
						exit 1						
					elif [ "$2" != "" ]; then
						ProcessLibsFromFile $2
						shift
						exit 0
					fi
					echo "SysLab1: option requires an argument -- '$1'"
					echo "Try \`SysLab1 --help' for more information."
					exit 1
	                                ;;
	        --wSItsf )	 	if [ "$2" != "" ]; then
						WriteSysInfoToScriptFile $2
						exit 0
					fi
					echo "SysLab1: option requires an argument -- '$1'"
					echo "Try \`SysLab1 --help' for more information."
					exit 1
	                                ;;
	        -h | --help )           ShowHelp
	                                exit 0
	                               	;;
	        * )                    	echo "SysLab1: unrecognized option '$1'"
					echo "Try \`SysLab1 --help' for more information."
		                        exit 1
	    esac
	    shift
	done
}

Input $@

exit 0
