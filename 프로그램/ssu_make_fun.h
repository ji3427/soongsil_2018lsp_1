#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/types.h>

typedef struct _nodet{
	char * target;
	char * dependency[15];
	int dependency_num;
	char * command[15];
	int command_num;
	struct _nodet * next;
}nodet;

typedef struct _nodem{
	char * macro;
	char * value;
	struct _nodem * next;
}nodem;

typedef struct _history{
	char * name;
	struct _history *next;
}history;

void makelist(nodet*, nodem*, char*);// t_head, m_head, filename
void make(char*, nodet*,history*,int); // target, t_head, t_history, dmake
bool check_dependency(char*, nodet*,history*); //target, t_head,t_history
nodet* parsing_target(char*);
nodet* search_target(char*, nodet*);
bool istarget(char*);
nodem* parsing_macro(char*, nodem*);
nodem* search_macro(char*, nodem*);
bool ismacro(char*);
void run_command(nodet*);
bool iscommand(char*);
bool isinclude(char*);
bool isblankline(char*);
void print_nodem(nodem*);
void makelist_macro(nodem*, char*, history*);
int ismacroin(char*);
char* mactoval(char*, nodem*);
void print_man();
int getspotpos(char*);
char* mactoval_in(char* , char*);
void add_macro(char*, nodem*);
void add_history(char*, history*);
history* search_history(char*,history*);
bool isuptodate(char*, nodet*);
bool isfile(char*);
bool isallfile(nodet*);
void delete_history(char*, history*);
void print_tree(nodet*);
int get_childnum(char*,nodet*,history*th);
void save_tree(int, int,char* array[100][40],char*,nodet*,history*); //row,col,array,target,t_head,tree_history
char* nblank(int);
