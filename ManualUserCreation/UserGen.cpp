/*
	EXIT VALUES:
		0 	-	executed successfully (including -h key);
		1	-	incorrect input;
		2	-	error opening source file;
		3	-	source file contains UNFORGIVABLE mistakes (YOUR SOURCE IS BAD AND YOU SHOULD FEEL BAD);


CLEAN UP:
	sudo rm -f -r /home/Cucumber
	sudo rm -f -r /home/MADSKILZPROGRAMMER
	sudo rm -f -r /home/TESTERSON
	sudo userdel Cucumber
	sudo userdel MADSKILZPROGRAMMER
	sudo userdel TESTERSON
	
	OR

	sudo bash ./usgenCLEANUP.sh
*/


#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <pwd.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>

#include <crypt.h>
#include <time.h>

#define _XOPEN_SOURCE     
#include <unistd.h>
#include <fcntl.h>

#define COLORRED 	"\x1B[31m"
#define COLORGREEN 	"\x1B[32m"
#define COLORRESET 	"\033[0m"
#define COLORBLUE	"\x1B[34m"

using namespace std;

struct UsrStruct
{
	string login;
	string fullname;
	string uid;
	struct UsrStruct * next;
};

void ShowHelp(void);
int AddUser(string, string, string, string);
string PassGen(string);
int ReadFile(char*, int &, struct UsrStruct **);
string InputHandler(int, char**);

void ShowHelp()
{
	printf
	(	"Usage: usgen [OPTION]\n"
		"Generate list of users from source file with random 8-character string suitable for a password.\n"
		"Format for each line of source file: \"<LOGIN>;<UID>;<USER_DESCRIPTION>\".\n"
		"\n"
		"Mandatory arguments to long options are mandatory for short options too.\n"
		"  -f, -F, --file=filename	use file named \"filename\" as a source;\n"
		"  -h, --help			display this help and exit;\n"
	);
	return;
}

string InputHandler(int argc, char** argv)
{
	switch (argc)
	{
		case 2:
			if ((strcmp(argv[1],"-h")==0)||(strcmp(argv[1],"--help")==0))
			{
				ShowHelp();
				return("\n");
			}
			if ((strcmp(argv[1],"-f")==0)||(strcmp(argv[1],"--file")==0)||(strcmp(argv[1],"-F")==0))
			{
				printf("usgen: option requires an argument -- %s\n", argv[1]);
				printf("Try `usgen --help' for more information.\n");
				return("");
			}
			else
			{
				printf("usgen: unrecognized option %s\n", argv[1]);
				printf("Try `usgen --help' for more information.\n");
				return("");
			}
		break;
		case 3:
			if ((strcmp(argv[1],"-f")==0)||(strcmp(argv[1],"-F")==0)||(strcmp(argv[1],"--file")==0))
			{
				printf("Source file name: %s\n", argv[2]);
				string file(argv[2]);
				return (file);
			}
		break;
		default:
			printf("Try `usgen --help' for more information.\n");
			return("");
	}
}

int main(int argc, char** argv)
{
	string input = InputHandler(argc, argv);

	if (input=="")
	{
		return(1);
	}
	if (input=="\n")
	{
		return(0);
	}
	
	printf("USGEN STARTING TO READ FROM SOURCE: \"" COLORBLUE "%s" COLORRESET "\"...\n", input.c_str());
	struct UsrStruct * first;
	int N=0;	

	char * file = new char[input.size()+1];
	copy(input.begin(), input.end(), file);	
	
	int read_result = ReadFile(file, N, &first);
	delete[] file;

	if (read_result==1)	
	{
		printf
		(
			"USGEN FINISHED. "
			COLORRED "NO" COLORRESET
			" USERS WILL BE CREATED. WHAT A "
			COLORRED "SHAME" COLORRESET
			"...\n"
		);
		return (3);
	}
	if (read_result==2)	
	{
		printf
		(
			"USGEN FINISHED. "
			COLORRED "NO" COLORRESET
			" USERS WILL BE CREATED. WHAT A "
			COLORRED "SHAME" COLORRESET
			"...\n"
		);
		return (2);
	}
	struct UsrStruct * current = first;	
	printf("USGEN GOT "COLORBLUE "%d" COLORRESET" USERS:\n", N);
	
	int count_success = 0;
	for (int i=0; i<N; i++)
	{
		
		string pass;		
		pass = PassGen(current->login);
		
		if (pass!="")
		{
			printf
			(
				"   %d:\n"
				"      LOGIN    = \"%s\"\n"
				"      FULLNAME = \"%s\"\n"
				"      UID      = \"%s\"\n"
				"      PASSWORD = \"%s\"\n",
				i+1, current->login.c_str(),current->fullname.c_str(),current->uid.c_str(),pass.c_str()
			);
		
			int add_result = AddUser(current->login,current->fullname,current->uid, pass);

			if (add_result==0)	
			{
				printf
				(
					"      RESULT   : "
					COLORGREEN "[SUCCESS!!!]" COLORRESET
					"\n\n"
				);
				count_success++;
			}
			else
			{
				printf
				(
					"      RESULT   : "
					COLORRED "[FAILED!!!]" COLORRESET
					"\n"
				);
			}
		}	
		current = current->next;
	}
	printf
	(
		"USGEN TOTAL: " COLORGREEN "%d" COLORRESET "/"
		COLORBLUE "%d" COLORRESET
		" USERS SUCCESSFULLY CREATED (" COLORRED "%d" COLORRESET " FAILED)\n",
		count_success, N, N-count_success
	);
	printf("USGEN FINISHED. SEE YA!\n");
	return(0);
}



int ReadFile(char * filename, int &N, struct UsrStruct **first)
 {
 	FILE *fp;
 	
	fp = fopen(filename, "r");
	if (fp == NULL) 
	{
		fprintf
		(
			stderr, COLORRED "ERROR:" COLORRESET " USGEN COULD NOT OPEN THE FILE \""
			COLORBLUE "%s" COLORRESET
			"\"\n",
			filename
		);
		return (2);
	}

	
	struct UsrStruct * current = new (struct UsrStruct);
	struct UsrStruct * root = new (struct UsrStruct);
	

	char c;
	bool l_s;
	bool u_s;
	bool fn_s;
	int LINE = 0;
	do
	{
		LINE++;
		string login = "";
		string fullname = "";
		string uid = "";
		l_s = false;
		u_s = false;
		fn_s = false;
		c = getc(fp);
		while ((c != EOF)&&(c != '\n')&&(c != ';'))
		{
			login+=c;
			c = getc(fp);
		}
		if ((c == EOF)||(c == '\n')||(login==""))
		{
			break;
		}
		l_s = true;
		c = getc(fp);
		bool keep_reading_uid = true;
		while ((c != EOF)&&(c != '\n')&&(c != ';'))
		{
			if (!(isdigit(c)))
			{
				keep_reading_uid = false;
			}
			if (keep_reading_uid)
			{
				uid+=c;
			}
			
			c = getc(fp);
		}
		if ((c == EOF)||(c == '\n')||(uid==""))
		{
			break;
		}
		u_s = true;
		c = getc(fp);
		while ((c != EOF)&&(c != '\n')&&(c != ';'))
		{
			fullname+= c;
			c = getc(fp);
		}
		if (fullname=="")
		{
			break;
		}
		fn_s = true;
		struct UsrStruct * next = new (struct UsrStruct);

		if (LINE==1)
		{
			root->login = login;
			root->fullname = fullname;
			root->uid = uid;
			root->next = next;
			current = root->next;
		}
		else
		{
			current->login = login;
			current->fullname = fullname;
			current->uid = uid;
			current->next = next;
			current = current->next;
		}
	}
	while ((c != EOF));
	if (c!=EOF)
	{
		if (!l_s)
		{
			fprintf
			(
				stderr, COLORRED "ERROR:" COLORRESET 
				"USGEN FAILED TO READ LOGIN IN LINE #%d\n",
				LINE
			);
			return (1);
		}
		if (!u_s)
		{
			fprintf
			(
				stderr, COLORRED "ERROR:" COLORRESET
				" USGEN FAILED TO READ UID IN LINE #%d\n",
				LINE
			);
			return (1);
		}
		if (!fn_s)
		{
			fprintf
			(
				stderr, COLORRED "ERROR:" COLORRESET
				" USGEN FAILED TO READ USER DESCRIPTION IN LINE #%d\n",
				LINE
			);
			return (1);
		}
	}
	N = LINE-1;
	*first = root;
	return (0);
}

string PassGen(string login)
{
	string pass;
	FILE *in;
	char buff[512];
	string command = "PassGen";
	if(!(in = popen(command.c_str(), "r")))
	{
		fprintf
		(
			stderr, COLORRED "ERROR:" COLORRESET
			" USGEN FAILED TO CREATE A PASSWORD FOR USER \""
			COLORBLUE "%s" COLORRESET
			"\"\n",
			login.c_str()
		);
		return ("");
	}
	if (fgets(buff, sizeof(buff), in)!=NULL)
	{
		string temp(buff);
		pass = temp.c_str();
		pass.resize(8);
	}
	int result = pclose(in);
	return pass;
}

int AddUser(string login, string fullname, string uid, string pass)
{

	
	struct passwd *userlist;
	setpwent();

	while ((userlist = getpwent()) != NULL)
	{
		if (strcmp(userlist->pw_name,login.c_str())==0)
		{
			fprintf
			(
				stderr, COLORRED "ERROR:" COLORRESET
				" USGEN CAN'T CREATE USER \""
				COLORBLUE "%s" COLORRESET				
				"\" - LOGIN ALREADY IN USE\n",
				login.c_str()
			);
			endpwent ();
			return (2);
		}
		char uid_char[256];
		int n = sprintf(uid_char, "%d", (int) userlist->pw_uid);
		string uid_string(uid_char);
		if (strcmp(uid_char,uid.c_str())==0)
		{
			fprintf
			(
				stderr, COLORRED "ERROR:" COLORRESET
				" USGEN CAN'T CREATE USER \""
				COLORBLUE "%s" COLORRESET				
				"\" - UID ALREADY IN USE\n",
				login.c_str()
			);
			endpwent ();
			return (2);
		}
	}

	endpwent();	
	
	int filedes_p, filedes_g, filedes_gs, filedes_s, rc;
	filedes_p = open("/etc/passwd", O_RDONLY, 0666);
	rc = flock(filedes_p, LOCK_EX | LOCK_NB);
	if(rc!=0)
	{
		fprintf
		(
			stderr, COLORRED "ERROR:" COLORRESET
			" USGEN FAILED TO ADD USER TO /etc/passwd - \""
			COLORBLUE "%s" COLORRESET
			"\"\n"
			"YOU MIGHT NOT HAVE PERMISSION TO CREATE NEW USERS.\n",
			login.c_str()
		);
		return (1);
	}

	filedes_g = open("/etc/group", O_RDONLY, 0666);
	rc = flock(filedes_g, LOCK_EX | LOCK_NB);
	if(rc!=0)
	{
		fprintf
		(
			stderr, COLORRED "ERROR:" COLORRESET
			" USGEN FAILED TO ADD USER TO /etc/group - \""
			COLORBLUE "%s" COLORRESET
			"\"\n"
			"YOU MIGHT NOT HAVE PERMISSION TO CREATE NEW USERS.\n",
			login.c_str()
		);
		return (1);
	}

	filedes_gs = open("/etc/gshadow", O_RDONLY, 0666);
	rc = flock(filedes_gs, LOCK_EX | LOCK_NB);
	if(rc!=0)
	{
		fprintf
		(
			stderr, COLORRED "ERROR:" COLORRESET
			" USGEN FAILED TO ADD USER TO /etc/gshadow - \""
			COLORBLUE "%s" COLORRESET
			"\"\n"
			"YOU MIGHT NOT HAVE PERMISSION TO CREATE NEW USERS.\n",
			login.c_str()
		);
		return (1);
	}

	filedes_s = open("/etc/shadow", O_RDONLY, 0666);
	rc = flock(filedes_gs, LOCK_EX | LOCK_NB);
	if(rc!=0)
	{
		fprintf
		(
			stderr, COLORRED "ERROR:" COLORRESET
			" USGEN FAILED TO ADD USER TO /etc/shadow - \""
			COLORBLUE "%s" COLORRESET
			"\"\n"
			"YOU MIGHT NOT HAVE PERMISSION TO CREATE NEW USERS.\n",
			login.c_str()
		);
		return (1);
	}

	FILE *in_p;
	char buff[512];	
	in_p =fopen("/etc/passwd", "a");

	if(in_p==NULL)
	{
		fprintf
		(
			stderr, COLORRED "ERROR:" COLORRESET
			" USGEN FAILED TO ADD USER TO /etc/passwd - \""
			COLORBLUE "%s" COLORRESET
			"\"\n"
			"YOU MIGHT NOT HAVE PERMISSION TO CREATE NEW USERS.\n",
			login.c_str()
		);
		return (1);
	}

	FILE *in_g;
	in_g =fopen("/etc/group", "a");

	if(in_g==NULL)
	{
		fprintf
		(
			stderr, COLORRED "ERROR:" COLORRESET
			" USGEN FAILED TO ADD USER TO /etc/group - \""
			COLORBLUE "%s" COLORRESET
			"\"\n"
			"YOU MIGHT NOT HAVE PERMISSION TO CREATE NEW USERS.\n",
			login.c_str()
		);
		return (1);
	}

	FILE *in_gs;
	in_gs =fopen("/etc/gshadow", "a");

	if(in_gs==NULL)
	{
		fprintf
		(
			stderr, COLORRED "ERROR:" COLORRESET
			" USGEN FAILED TO ADD USER TO /etc/gshadow - \""
			COLORBLUE "%s" COLORRESET
			"\"\n"
			"YOU MIGHT NOT HAVE PERMISSION TO CREATE NEW USERS.\n",
			login.c_str()
		);
		return (1);
	}

	FILE *in_s;
	in_s =fopen("/etc/shadow", "a");

	if(in_s==NULL)
	{
		fprintf
		(
			stderr, COLORRED "ERROR:" COLORRESET
			" USGEN FAILED TO ADD USER TO /etc/shadow - \""
			COLORBLUE "%s" COLORRESET
			"\"\n"
			"YOU MIGHT NOT HAVE PERMISSION TO CREATE NEW USERS.\n",
			login.c_str()
		);
		return (1);
	}

	char * home=(char *)malloc(256*sizeof(char));;
	sprintf(home,"/home/%s",login.c_str());

	time_t now = time(NULL);
	unsigned int days = (unsigned int)(((now / 24) / 60 )/ 60);

	
	string salt = "HE";
	//salt += pass[0] + pass[pass.length()-1];

	string encr = crypt(pass.c_str(), salt.c_str());

 	fprintf(in_p, "%s:x:%s:%s:%s:%s:/bin/bash\n", login.c_str(),uid.c_str(),uid.c_str(),fullname.c_str(),home);
	
	fprintf(in_g, "%s:x:%s:\n", login.c_str(),uid.c_str());

	fprintf(in_gs, "%s:!::\n", login.c_str());

	fprintf(in_s, "%s:%s:%i:0:99999:14:::\n", login.c_str(),encr.c_str(),days);

 	fclose(in_p);
 	fclose(in_g);
	fclose(in_gs);
	fclose(in_s);
	rc = flock(filedes_p, LOCK_UN | LOCK_NB);
	close(filedes_p);
	rc = flock(filedes_g, LOCK_UN | LOCK_NB);
	close(filedes_g);	
	rc = flock(filedes_gs, LOCK_UN | LOCK_NB);
	close(filedes_gs);
	rc = flock(filedes_s, LOCK_UN | LOCK_NB);
	close(filedes_s);

	setpwent();

	bool add_result = false;

	while ((userlist = getpwent()) != NULL)
	{
		char uid_char[256];
		int n = sprintf(uid_char, "%d", (int) userlist->pw_uid);
		string uid_string(uid_char);
		if ((strcmp(userlist->pw_name,login.c_str())==0)&&(strcmp(uid_char,uid.c_str())==0))
		{
			add_result = true;
		}
	}

	endpwent();

	if (!add_result)
	{
		fprintf
		(
			stderr, COLORRED "ERROR:" COLORRESET
			" USGEN FAILED TO ADD USER TO /etc/passwd - \""
			COLORBLUE "%s" COLORRESET
			"\"\n"
			"YOU MIGHT NOT HAVE PERMISSION TO CREATE NEW USERS.\n",
			login.c_str()
		);
		return (1);
	}

	mkdir(home, 0755);
	chdir(home);
	chown(home, atoi(uid.c_str()), 1);
	chdir(".");
	return (0);
}
