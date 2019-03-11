#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include "ssu_make_fun.h"


bool isop_s=false; //s옵션이 적용되었는지 판단하는 flag


int main(int argc, char *argv[])
{
	int option,index;
	int flag_h = 0,flag_m = 0, flag_t = 0;
	int target_count=0;
	int macro_count=0;
	char* targets[5]={"","","","",""}; // 프로그램 실행시 입력되는 target을 저장하는 배열
	char* macros[5]={"","","","",""}; // 프로그램 실행시 입력되는 macro를 저장하는 배열
	char * filename="Makefile";

	nodet *thead=(nodet*)malloc(sizeof(nodet));
	nodem *mhead=(nodem*)malloc(sizeof(nodem));
	history *t_history = (history*)malloc(sizeof(history));
	history *i_history = (history*)malloc(sizeof(history));
	for(index = 1; index < argc ; index++){ //옵션이 오기전에 target이나 macro가 오는 경우를 처리하기 위한 함수
		if (argv[index][0] == '-') {
			break;
		}
		else{
			if (ismacro(argv[index])) { //macro정의가 오는 경우
				if (macro_count > 4) { //기입된 메크로가 5개가 넘어가는 경우
						fprintf(stderr,"over macro\n");
						exit(1);
				}
				macros[macro_count++] = argv[index];
			}
			else { //target이 오는 경우
				if (target_count > 4) { //기입된 target이 5개가 넘어가는 경우
					fprintf(stderr,"over target\n");
					exit(1);
				}
				targets[target_count++] = argv[index];
			}	
		}
	}
	
	while ((option = getopt(argc,argv,"f:c:shtm")) != -1) { //option 처리를 위한 while문
		switch(option){
			case 'f': 
				filename = optarg;
				break;
			case 'c': 
				if (chdir(optarg) < 0) {
					printf("No such directory\n");
					exit(1);
				}
				break;
			case 's': 
				isop_s = true;
				break;
			case 'h':
				flag_h = 1;
				break;
			case 'm':
				flag_m = 1;
				break;
			case 't':
				flag_t = 1;
				break;
			case '?':
				exit(1);
				break;
		}
		if(optind >= argc){
			break;
		}
		//옵션 뒤에 macro나 타겟이 오는 경우
		for (index = optind ; index < argc ; index++) {
			if (argv[index][0] == '-') {
				break;
			}
			else {
				if (ismacro(argv[index])) {
					if (macro_count > 4) {
						fprintf(stderr,"over macro\n");
						exit(1);
					}
					macros[macro_count++] = argv[index];
				}
				else {
					if (target_count > 4) {
						fprintf(stderr,"over target\n");
						exit(1);
					}
					targets[target_count++] = argv[index];
				}	
			}
		}
	}
		
	makelist_macro(mhead, filename, i_history); //macro를 먼저 list로 만듬 (macro를 value로 치환하기 위하여)
	i_history->next = NULL;
	for (int i = 0 ; i < 5 ; i++) { //프로그램 실행시 입력된 macro들 추가
		if (strcmp(macros[i], "") == 0) {
				break;
		}
		else {
			add_macro(macros[i], mhead);
		}
	}
	
	if (flag_h) { 
		print_man();
		exit(1);
	}
	if(flag_m &&flag_t){
		printf("--------------------macro list------------------------\n");
		print_nodem(mhead);
	}
	else if(!flag_m&&!flag_t){
	}
	else{
		if(flag_m){
			printf("--------------------macro list------------------------\n");
			print_nodem(mhead);
			exit(1);
		}
	}
	
	
	makelist_macro(mhead, filename, i_history); // macro내의 macro를 처리하기 위하여 한번더 호출
	for (int i = 0 ; i < 5 ; i++) {
		if(strcmp(macros[i], "") == 0){
				break;
		}
		else {
			add_macro(macros[i], mhead);
		}
	}
	makelist(thead, mhead, filename); // macro를 제외한 기타 문장들 처리
	if(flag_t){
		printf("--------------------graph------------------------------\n");
		print_tree(thead);
		exit(0);
	}

	if (strcmp(targets[0], "") == 0){ //주어진 타겟이 없는 경우
		if(thead->next == NULL){
			printf("%s:%s:target not exist. stop\n","make", filename);
			exit(1);
		}
		make(thead->next->target, thead,t_history, 0);
	}
	else{ //타겟이 주어진 경우
		for (int i = 0 ; i < 5 ; i++) {
			if (strcmp(targets[i], "") == 0) {
				break;
			}
			else {
				if (search_target(targets[i],thead) == NULL) { //list에 주어진 target이 없는  경우
					printf("make: *** No rule to make target \'%s\'. Stop\n", targets[i]);
					exit(1);
				}
				else { //target이 있는 경우 make함수 호출
					make(targets[i], thead, t_history, 0);
					t_history->next = NULL;
				}
				
			}
		}
	}
	
}
