#include "ssu_make_fun.h"
#include "ssu_make_str.h"
extern bool isop_s;

//문장들을 읽어들여서 target문 command문 등등 문장 별로 분류하여(macro제외) 링크드 리스트를 만들어서 각각 parsig 하는 함수(리턴 : x, 인자 : 타겟 리스트의 head, 매크로 리스트의 head, Makefile명, 이 전 파일명(에러출력을 위한))
void makelist(nodet* thead, nodem*mhead, char* filename){ 
	char* line;
	int line_num=0;
	nodet* tcurrent = thead;
	nodem* mcurrent = mhead;
	int now = 8;
	char* temp;
	while (tcurrent->next != NULL) {
		tcurrent = tcurrent->next;
	}
	int fd = open(filename,O_RDONLY);
	if (fd < 0){
		fprintf(stderr,"make:%s:no such file or directory\n", filename);
		exit(1);
	}
	while ((line = readline(fd,filename,&(line_num))) != NULL) { //readline함수를 통하여 한줄씩 문장을 읽어 들임
		
		line_num++;
		if (isblankline(line)) { // 공백문장 일 경우 무시함
			continue;
		}
		else if (line[0] == ' ' ) { // 공백문장은 아니면서 시작을 공백으로 할 경우 에러처리
			printf("%s:%d:missing separator stop\n",filename, line_num);
			exit(1);
		}
		for (int i = 1 ; i < strlen(line) ; i++) { // 문장의 시작외에 주석문이 올 경우 에러처리
			if(line[i] == '#'){
				printf("%s:%d:missing separator stop\n",filename, line_num);
				exit(1);
			}
		}
		
		if (ismacroin(line) == 1) { //문장안에 매크로가 오는 경우 매크로값을 value 값으로 치환 해줌 (ismacro 의 return 값이 1일경우 일반 매크로 2일 경우 내부 매크로)
			line = mactoval(line, mhead);
		}
		if (istarget(line)) { // 문장이 타겟이 들어가 있는 문장일 경우
			if(line[0] == '\t'){ //타겟문인데 \t로 시작하는 경우
				printf("%s:%d:missing separator stop\n",filename, line_num);
				exit(1);
			}
			if(search_target(parsing_target(line)->target, thead) == NULL){ // 타겟이 중복되지 않은 경우
				tcurrent->next = parsing_target(line); // 파싱해서 리스트에 추가해줌
				tcurrent = tcurrent->next;
			}
			else { // 타겟이 중복되는경우 에러처리
				fprintf(stderr, "%s 이 중복됨\n", parsing_target(line)->target);
				exit(1);
			}
		}
		else if (iscommand(line)) { // 문장이 커맨드 문장인 경우
			if (ismacroin(line) == 2) { // 내부메크로가 오는경우
				line = mactoval_in(line, tcurrent->target); // 내부메크로를 타겟값으로 치환해줌
			}
			tcurrent->command[tcurrent->command_num] = substr(line, 2, strlen(line) - 1);
			tcurrent->command_num++;
		}
		else if (isinclude(line)) { //include 문장일경우
			if(line[0] == '\t'){ //include문인데 \t로 시작하는 경우
				printf("%s:%d:missing separator stop\n",filename, line_num);
				exit(1);
			}
			while(now <= strlen(line)){
				now = now + length_blank(line, 8);
				temp = substr(line, now, length_word(line, now)); //include 된 파일명들을 나누어서 temp 변수에 저장
				now = now + length_word(line, now); 
				makelist(thead, mhead, temp); // include된 파일에 대해서 makelist를 해줌
			}
			now = 8;
			while (tcurrent->next != NULL) {
				tcurrent = tcurrent->next;
			}

		}
		else if (ismacro(line)) { // 메크로 문장일경우 넘어감
			if(line[0] == '\t'){ //메크문인데 \t로 시작하는 경우
				printf("%s:%d:missing separator stop\n",filename, line_num);
				exit(1);
			}
			continue;
		}
		else if(line[0] == '#'){ // 주석문일 경우 넘어감
			if(line[0] == '\t'){ //주석문인데 \t로 시작하는 경우
				printf("%s:%d:missing separator stop\n",filename, line_num);
				exit(1);
			}
			continue;
		}
		else {
			printf("%s:%d:missing separator stop\n", filename, line_num); //그 외에 문장형식이 오는 경우 에러처리
			exit(1);	
		}
		
	}
}
//make_list에서 처리하지 못한 macro 정의 문장을 처리하는 함수(리턴 x 인자 : 메크로 리스트의 head , 파일명, 재귀 함수 호출시 호출이일어나는 linenum, 이전 파일명, include history)
void makelist_macro(nodem* mhead, char* filename,history* i_history){
	add_history(filename, i_history);
	nodem* mcurrent = mhead;
	char* line;
	char* temp;
	int now = 8;
	int temp_num = 0;
	while (mcurrent->next != NULL) {
		mcurrent = mcurrent->next;
	}
	int fd = open(filename, O_RDONLY);
	if (fd < 0) {
			printf("make:%s no such file or directory\n",filename);
			exit(1);
		}
	while ((line = readline(fd,filename,&(temp_num))) != NULL) {
		if (ismacro(line)) { // 문장이 메크로 정의 문장일경우
			if (ismacroin(line) == 1) { //메크로 정의 문장 안에 메크로가 오는 경우
				line = mactoval(line, mhead); // 메크로를 value로 치환함
			}
			nodem* temp = parsing_macro(line, mhead); // 메크로 정의를 파싱해서 리스트에 저장
			if (temp == NULL) {
				continue;
			}
			else {
				mcurrent->next = temp;
				mcurrent = mcurrent->next;
			}
		}
		else if (isinclude(line)) { //include 문장일경우(include 파일에 macro 정의 문장만 처리)
			if (ismacroin(line) != 0) {
				fprintf(stderr, "error include line has macro\n");
				exit(1);
				
			}
			while (now < strlen(line)) {
				now = now + length_blank(line, 8);
				temp = substr(line, now, length_word(line, now));
				now = now + length_word(line, now);
				if (search_history(temp, i_history) == NULL) {
					makelist_macro(mhead, temp, i_history); // include 된 파일들에 매크로 정의 문장들을 저장함
				}
				else {
					if (search_history(temp, i_history)->next != NULL) { // 같은 파일이 include가 여러번 되는 경우 loop가 생성되므로 에러처리 
						fprintf(stderr, "file loop exist\n");
						exit(1);
					}
					else if (strcmp(search_history(temp, i_history)->name, filename) == 0) { // 자신이 자신을 include 해도 loop가 생성되므로 에러처리
						fprintf(stderr, "자기가 자기 호출\n");
						exit(1);
					}
				} 
			}
			now = 8;
			while (mcurrent->next != NULL) {
				mcurrent = mcurrent->next;
			}
		}
		else {
			continue;
		}
	}
	lseek(fd, 0, SEEK_SET); //makelist 호출을 위해서 파일을 처음으로 옮겨놓음
}
//parsing이 완료된 리스트를 이용하여 주어진 target을 make하는 함수 (return x 인자 : target명 , target list 의 head, target_history, 디펜시브를 make하는 건지 target을 make하는지 판별하는 숫자(dmake =1 경우 dependency를 make하는 문장))
void make(char* target, nodet* thead,history * t_history,int is_dmake){ 
	history * d_history = (history*)malloc(sizeof(history)); 
	nodet* current = search_target(target, thead); 
	add_history(target, t_history); //진짜 make를 할때 history를 저장하는 리스트
	add_history(target, d_history);//make가 가능한지 검사를 할떄의 history를 저장하는 리스트
	bool* ispass;
	
	if (current == NULL) { //target리스트에 인자로 전달된 target이 없는 경우
		if (!access(target, 0)) { //파일인 경우 return
			delete_history(target, t_history);
			return;
		}
		else { //파일 조차 없는 경우 error처리
			printf("make: *** No rule to make target \'%s\'. Stop\n", target);
			exit(1);
		}
	}
	else { //target리스트에 target이 있는 경우
		if (current->dependency_num == 0) { //dependency의 개수가 0인경우 실행
			run_command(current);
			delete_history(target, t_history);
			return;
		}
		ispass = malloc(sizeof(bool) * current->dependency_num); // 각각의 dependency가 make가 가능한지 검사해서 결과를 저장하는 변수 ispass
		for (int i = 0 ; i < current->dependency_num ; i++) {
			ispass[i] = check_dependency(current->dependency[i], thead,d_history); //ispass에 각각의 dependency가 make가 가능한지 검사하여 저장
			if (ispass[i]) { //make가 가능한경우 history에 있으면 dependency를 make 안해주고 없는 경우 make를 해줌
				if (search_history(current->dependency[i], t_history) == NULL) {
					make(current->dependency[i], thead,t_history,1);
				}
				else {
					printf("make: Circular %s <- %s dependency dropped,\n", target,current->dependency[i]);
				}
				
			}
			else { //dependency가 make가 불가능한 경우
				printf("make: *** no rule to make target \'%s\' , needed by \'%s\'. stop\n", current->dependency[i], target);
				exit(1);
		}
	}

	if (isfile(target) && isallfile(current) && isuptodate(target,current)) { //하나의 타겟이 make 판별이 끝난 경우 타겟이 파일이고 dependency가 모두 파일인지 검사한 후 모두 파일일 경우 모든 dependency에 대해 uptodate를 검사함
		if (is_dmake) { //dependecy를 make하는 과정에서 uptodate가 발생하는 경우 그냥 return 함
			return;
		}
		else { //주어진 target을 make하는 과정에서 uptodate가 발생하는 경우 메세지를 띄우고 종료
			printf("make: \'%s\' is up to date\n",target);
			return;
		}
	}
	run_command(current); //모든 dependency에 대해 make가 끝난 경우 커맨드 실행
	delete_history(target, t_history);
	}	
}
//history list에서 주어진 타겟을 검사해서 그 target의 위치를 return 해주는 함수(return : 찾은 target의 위치 인자 : 찾을 target, target이 있는 list)
history* search_history(char *target,history* head_history){
	history* current = head_history->next;
	while(current != NULL){
		if (strcmp(current->name,target) == 0) {
			return current;
		}
		else {
			current = current -> next;
		}
	}
	return NULL;
}
// 주어진 list끝에 target이라는 새로운 노드를 추가해주는 함수(return x 인자: 추가할 target, 추가될 list)
void add_history(char* target, history* head_history){
	history* current = head_history;
	if(search_history(target,head_history)!=NULL){
		return;
	}
	while(current -> next != NULL){
		current = current -> next;
	}
	current ->next = (history*)malloc(sizeof(history));
	current ->next->name = target;
}
// 주어진 dependency가 make가 가능한지 검사하는 함수(return x 인자 : 검사할 dependency, 검사할 list, history를 저장할 list)
bool check_dependency(char* target,nodet* thead,history* d_history){
	add_history(target,d_history);
	nodet* current = search_target(target, thead);
	bool* ispass;
	if (current != NULL) { //dependency가 targetlist에 있는 경우
		if (current->dependency_num == 0) { //dependency 숫자가 0인경우 true를 return 함
			delete_history(target, d_history);
			return true;
		}
		else {
			ispass = malloc(sizeof(bool) * current->dependency_num); //주어진 dependency의 dependency를 체크하기 위한 변수
			for (int i = 0 ; i < current->dependency_num ; i++) {
				if (search_history(current->dependency[i], d_history) != NULL) { //history에 있는 경우 true로 취급
					ispass[i] = true;
				}
				else {
					ispass[i] = check_dependency(current->dependency[i], thead,d_history); //history에 없는 경우 dependency를 다시 검사
			}
			}
			for (int i = 0 ; i < current->dependency_num ; i++) {
				ispass[0] = ispass[0] && ispass[i]; //모든 검사가 통과한 경우 그 결과를 ispass[0]에 저장한후 return
			}
			delete_history(target, d_history);	
			return ispass[0];
		}
	}
	else { //dependency가 targetlist에 없는 경우
		if (!access(target, 0)) { //파일일 경우 return true
			delete_history(target, d_history);
			return true;
		}
		else { //파일이 없는 경우 false return
			delete_history(target, d_history);
			return false;
		}
	}
}

//문장이 target일경우 target과 dependency를 분류해서 저장하는 함수 (return 타겟을 저장하는 구조체 포인터, 인자 : parsing할 문장)
nodet* parsing_target(char* string){
	nodet* next; //새로운 target 노드
	int now = 1;
	int target_length = length_word(string, 1); //target의 길이를 저장
	next = (nodet*)malloc(sizeof(nodet));
	next->command_num = 0;
	next->dependency_num = 0;
	char* temp = (char*)malloc(sizeof(char) * target_length);
	temp = substr(string, 1, target_length); //문장에서 target을 분리해내서 저장
	next->target = temp;
	if (ismacroin(string) == 2) { // 내부메크로 존재시
		string = mactoval_in(string, temp); //메크로를 value로 치환해줌
	}
	remove_blank(string);
	now = target_length + 2;
	while (now <= strlen(string)) { //각각의 dependency들을 저장함
		temp = (char*)malloc(sizeof(char) * length_word(string, now));
		temp = substr(string, now, length_word(string, now));
		next->dependency[next->dependency_num] = temp;
		next->dependency_num++;
		now = now + length_word(string, now) + length_blank(string,now + length_word(string, now));
	}
	return next;
}

//target 리스트에서 주어진 target이 있는지 검사해서 위치를 return 해주는 함수(인자 : 검색할 target 명, target list의 head 주소, return 찾은 위치)
nodet* search_target(char* target, nodet* thead){
	nodet* current = thead->next;
	while (current != NULL) {
		if (strcmp(current->target, target) == 0) {
			return current;
		}
		else {
			current = current->next;
		}
	}
	return NULL;
}

bool istarget(char* string){ //문장이 target인지 아닌지 검사해주는 함수(:의 유무를 통해서 검사)
	for(int i = 0 ; i < strlen(string) ; i++){
		if (string[i] == ':') {
			return true;
		}
	}
	return false;
	
}
//문장에서 macro와 value를 분류해서 저장하는 함수 (인자 : 주어진 문장 , macro list의 head 주소 return : 새로운 노드)
nodem* parsing_macro(char* string, nodem* mhead){
	nodem* next;
	int i = 0;
	next = (nodem*)malloc(sizeof(nodem));
	int mlen = length_word(string, 1); // macro의 길이를측정
	char* temp = (char*)malloc(sizeof(char*) * mlen);
	temp = substr(string, 1, mlen); //macro를 분류해서 저장 (=을 기준으로 분류함)
	next->macro = temp;
	remove_blank(string);
	while (string[i] != '=') { 
		i++;
	}
	if (string[i-1] != '?') { //=일경우
		if (search_macro(next->macro, mhead) == NULL) { //macro가 존재하지 않는 경우 새로 추가함
			temp = substr(string, mlen + 2, strlen(string) - mlen - 1);
			next->value = temp;
		}
		else{ //macro가 존재하는 경우 기존에 있던 value값을 새로운 value값으로 변경
			temp = substr(string, mlen + 2, strlen(string) - mlen - 1);
			search_macro(next->macro, mhead) -> value = temp;
			return NULL;
		}
		
	}
	else { // ?=일경우
		if (search_macro(next->macro, mhead) == NULL) { //기존의 macro가 존재하지 않는 경우 새로 추가
			temp = substr(string, mlen + 3, strlen(string) - mlen - 2);
			next->value = temp;
		}
		else { //존재하는 경우 NULL return
			return NULL;
		}
	}
	return next;

}
//macro가 존재하는지 검사하는 함수
nodem* search_macro(char* macro, nodem* mhead){
	nodem* current = mhead->next;
	while (current != NULL) {
		if (strcmp(current->macro, macro) == 0) {
			return current;
		}
		else {
			current = current->next;
		}
	}
	return NULL;
	
}
//macro 인지 아닌지 검사하는 함수(=의 유무를 기준으로 검사)
bool ismacro(char* string){
	int i = 0;
	while (string[i] != '\n' && string[i] != '\0') {
		if (string[i] == '=') {
			return true;
		}
		i++;
	}
	return false;
}
//command를 system함수를 통해서 실행하는 함수 
void run_command(nodet* current){
	for (int i = 0 ;i < current->command_num ; i++) {
		if (isop_s){
			system(current->command[i]);
		}
		else{
			printf("%s\n", current->command[i]);
			system(current->command[i]);
		}
		
	}
}
//command인지 검사하는 함수 (시작이 \t일경우로 구분)
bool iscommand(char* string){
	if (string[0] == '\t') {
		return true;
	}
	return false;
}
//include인지 검사하는 함수(include로 시작하는지로 구분)
bool isinclude(char*string){
	char * temp = substr(string, 1, 7);
	if (strcmp("include", temp) == 0) {
		return true;
	}
	return false;
}
//공백라인인지 검사하는 함수
bool isblankline(char*string){
	for (int i = 0 ; i < strlen(string) ; i++) {
		if (string[i] != ' ' && string[i] != '\t') {
			return false;
		}
	}
	return true;
}
//macro 출력 함수
void print_nodem(nodem* mhead) {
	mhead = mhead->next;
	while (mhead != NULL) {
			printf("%s--> %s\n", mhead->macro, mhead->value);
			mhead = mhead->next;
	}
		
}
//문장에 변환할 macro가 있는지 검사하는 함수 내부메크로 return 2 일반메크로 return 1 메크로가 존재하지 않을시 return 0
int ismacroin(char* string){ 
	if (strlen(string) < 2) {
		return 0;
	}
	for (int i = 0 ; i < strlen(string)-1 ; i++) {
		if (string[i] == '$') {
			if (string[i+1] == '(') {
				return 1;
			}	
			else if (string[i+1] == '*' || string[i+1] == '@') {
				return 2;
			}
				
			else{
				return 0;
			}
				
		}
	}
	return 0;
		
}
//문장에 있는 macro를 value로 변환 해줌 (내부메크로 제외)
char* mactoval(char* string, nodem* mhead){
	int i = 0;
	int sp;
	int ep;
	char* result;
	while (string[i] != '$') {
		i++;
	}
	sp = i;
	i = i + 2;;
	while (string[i] != ')') {
		i++;
	}
	ep = i;
	char* macro_full = substr(string,sp + 1, ep - sp + 1); //문장에서 macro=value의 형태를 잘라냄
	char* macro = substr(string, sp + 3, ep - sp - 2); //문장에서 macro부분만 추출
	nodem* temp = search_macro(macro, mhead);
	if (temp == NULL) { //macro가 존재하지않는 경우
		if (ismacro(string)) { //macro정의 문장일경우 그대로 나둠
			return string;
		}
		else { //macro정의 문장이 아닌경우 macro를 ""으로 치환
			result = replaceValue(string, macro_full, "");
			return result;
		}
	}
	result = replaceValue(string, macro_full, temp->value); //macro가 존재하는 경우 value로 치환
	return result;
}
//문장에 있는 macro를 value로 변환 해줌(내부메크로)
char* mactoval_in(char* string, char*target){
	int i = 0;
	int pos;
	char* result;
	char* temp;
	for(; i < strlen(string) ; i++){
		if (string[i] == '$') {
			if (string[i+1] == '*') { //$*인 경우
				if (istarget(string)) { //내부메크로가 target문장에 오는 경우
					fprintf(stderr, "inner macro can not be located on the target\n");
					exit(1);
				}
				if ((pos=getspotpos(target)) == -1) { //.이  없는경우
					result = replaceValue(string, "$*", target);
				}
				else { //.이 있는경우
					temp = substr(target,1,pos);
					result = replaceValue(string, "$*", temp);
				}
			}
			else if (string[i+1] == '@') { //$@인 경우
				if (istarget(string)) {
					fprintf(stderr, "inner macro can not be located on the target\n");
					exit(1);
				}
				result = replaceValue(string, "$@", target);
			}
		}
	}	
	return result;
}
//문장에서 .에 위치를 파악해주는 함수
int getspotpos(char* string){
	for (int i = 0 ; i < strlen(string) ; i++) {
		if (string[i] == '.') {
			return i;
		}
	}
	return -1;
}
//문장에서 macro와 value를 분리해서 list끝에 추가해주는 함수
void add_macro(char* string, nodem* mhead){
	int i = 0;
	nodem*mcurrent = mhead;
	while (mcurrent->next != NULL) {
		mcurrent = mcurrent->next;
	}
	char* mac;
	char* val;
	for (; i < strlen(string) ; i++) {
		if (string[i] == '=') {
			break;
		}
	}
	mac = substr(string, 1, i);
	val = substr(string, i + 2, strlen(string));
	if (search_macro(mac,mhead) == NULL) {
		nodem* next = (nodem*)malloc(sizeof(nodem));
		next->macro = mac;
		next->value = val;
		mcurrent->next=next;
	}
	else {
		search_macro(mac,mhead)->value = val;
	}
	
}
// 주어진 target의 dependency들의 최종 수정시간을 비교하여 uptodate를 확인해주는 함수
bool isuptodate(char* target,nodet* current){
	time_t ttime;
	time_t dtime;
	struct stat target_stat;
	struct stat dependency_stat;
	if (stat(target,&target_stat) < 0) {
		fprintf(stderr, "stat error for %s\n", target);
		return false;
	}
	for (int i = 0 ; i < current->dependency_num ; i++) {
		if (stat(current->dependency[i], &dependency_stat) < 0) {
			fprintf(stderr, "stat error for %s\n", current->dependency[i]);
			return false;
		}
		ttime = target_stat.st_mtime;
		dtime = dependency_stat.st_mtime;
		if(ttime < dtime){ //1개라도 dependency에 최종수정시간이 target의 최종 수정시간보다 최신인 경우
			return false;
		}
	}
	return true;
	
}
bool isfile(char* filename){ //파일이 존재하는지 검사하는 함수
	if (!access(filename,F_OK)) {
		return true;
	}
	else {
		return false;
	}
}
bool isallfile(nodet* current){ //target의 dependency가 모두 파일이 존재하는지 검사하는 함수
	for (int i = 0 ; i < current->dependency_num ; i++) {
		if (!isfile(current->dependency[i])) {
			return false;
		}
	}
	return true;
	
}
//옵션 출력 함수
void print_man(){
	printf("Usage : ssu_make [Option][Target][Macro]\n");
	printf("Option:\n");
	printf("-f %-12s Use <file> as a makefile\n","<file>");
	printf("-c %-12s Change to directiry <directory> before reading the makefiles\n","<directory>");
	printf("-s %12s Do not print the commands as they are excutedt\n","");
	printf("-h %12s print usage\n","");
	printf("-m %12s print macro list\n","");
	printf("-t %12s print tree\n","");
}
//주어진 target을 history에서 삭제하는 함수
void delete_history(char* target, history*head_history){
	history* current = head_history->next;
	if(search_history(target, head_history)==NULL){
		return;
	}
	while (current != NULL) { // 주어진 target이 저장된 위치를 검사
		if (strcmp(current->name, target) == 0) {
			break;
		}
		current = current->next;
	}
	if (current == head_history->next) { //위치가 제일 앞에 있는 경우
		head_history->next = head_history->next->next;
	}
	else if (current->next == NULL) { //위치가 끝에 있는 경우
		current = head_history;
		while (current->next->next != NULL) {
			current = current->next;
		}
		current->next = NULL;
	}
	else { //그 외 경우
		history* temp = head_history->next;
		while (temp->next != current) {
			temp = temp->next;
		}
		temp->next = temp->next->next;
	}
}
//최하위 dependency의 개수를 계산해주는 함수
int get_childnum(char* target, nodet* t_head,history* temp){
	add_history(target,temp);
	nodet* current = search_target(target, t_head); 
	if (current == NULL || current->dependency_num == 0) { //파일이거나 최하위 dependency일경우
		return 1;
	}
	int result = 0;
	for (int i = 0 ; i < current->dependency_num ; i++) {
		if (search_history(current->dependency[i],temp) != NULL) { //history에 있는경우(순환이 생기는 경우)
			result++;
			continue;
		}
		if (search_target(current->dependency[i],t_head) == NULL || search_target(current->dependency[i], t_head)->dependency_num == 0) { //dependency가 파일이거나 최하위 dependency인 경우
			result++;
		}
		else{
			result = result + get_childnum(current->dependency[i], t_head, temp); //재귀 호출
		}
	}
	return result;
}
//배열에 의존성 그래프 인자를 저장하는 함수
void save_tree(int row,int col,char* array[100][40],char*target,nodet* t_head,history* th){
	nodet* current = search_target(target, t_head);
	history* temp = (history*)malloc(sizeof(history));
	add_history(target, th);
	if(current == NULL){ //파일인 경우
		array[row][col] = target;
		return;
	}
	int next_row=row;
	array[row][col]=target;
	temp->next=NULL;
	for(int i = row + 1 ; i < row + get_childnum(target,t_head,temp) ; i++){
		array[i][col] = nblank(strlen(target)); //타겟 아래를 다음 타겟 전까지 타겟의 글자수의 공백으로 채워줌
		temp->next = NULL; //getchildnum 호출후 비워줘야 됨
	}
	col++;
	//dependecy들을 배열에 저장하는 함수
	for (int i = 0 ; i < current->dependency_num ; i++) {
		if (search_history(current->dependency[i],th) != NULL) { //순환이 생기는 경우
			array[next_row][col] = current->dependency[i];
			next_row++;
			continue;
		}
		save_tree(next_row, col, array, current->dependency[i], t_head, th);
		next_row = next_row + get_childnum(current->dependency[i], t_head, temp);
		temp->next = NULL;
	}
	delete_history(target,th);
}
//n개 만큼의 공백 문장을 생성히는 함수
char* nblank(int num){
	char * temp = (char*)malloc(sizeof(char) * num);
	for(int i = 0 ; i < num ; i++){
		temp[i] = ' ';
	}
	return temp;
}
//그래프 출력
void print_tree(nodet* t_head){
	nodet* current = t_head->next;
	history * th = (history*)malloc(sizeof(history));
	history * temp = (history*)malloc(sizeof(history));
	char* array[100][40];
	int row=0;
	//배열 초기화
	for (int i = 0 ; i < 100 ; i++) {
		for (int j = 0 ; j <40 ; j++) {
			char *temp = (char*)malloc(sizeof(char));
			temp = "";
			array[i][j] = temp;
		}
	}
	array[0][0] = "root";
	//루트 아래로 전부 공백칸
	for (int i = 1 ; i <100 ; i++) {
		array[i][0] = nblank(4);
	}
	//각 dependency의 dependency들을 저장함
	while (current!=NULL) {
		save_tree(row, 1, array, current->target, t_head, th);
		row = row + get_childnum(current->target, t_head, temp);
		current = current->next;
		th->next = NULL;
		temp->next = NULL;
	}
	for(int i = 0 ; i < row ; i++){
		for(int j = 0 ; j <40 ; j++){
			printf("%s ", array[i][j]);
		}
		printf("\n");
	}
		
}
