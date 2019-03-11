#include "ssu_make_str.h"

//시작 위치 부터 개행, 문장끝, 공백, 특수문자를 만날때까지 읽어서 글자를 출력해주는 함수
int length_word(char* string, int st_point){
	int temp = st_point - 1;
	while (string[temp] != ' ' && string[temp] != '\n' && string[temp] != '\0' && string[temp] != ':' && string[temp] != '=' && !(string[temp] == '?'&&string[temp+1] == '=') && string[temp] != '\t') {
		temp++;
	}
	return temp-st_point + 1;
}
//한줄을 읽는 함수
char* readline(int fd,char* filename, int*line_num){
	int i = 0;
	int length = 0; 
	int result;
	char temp[1];
	while (result = read(fd, temp, 1)) { //문장의 길이를 파악하기위한 반복문
		if (temp[0] == '\\') { // 긴문장일 경우
			read(fd, temp, 1);
			(*line_num)++;
			length++;
			if (temp[0] != '\n') { // 역슬래쉬 다음에 개행 외에 문자가 오는 경우 에러 처리
				printf("error : \'\\\' 's back letter can not be other letter,only \\n\n");
				exit(1);
			}
			else {
				length++;
				continue;
			}
		}
		else if (temp[0] == '\n') { //개행이 오는 경우 종료
			length++;
			break;
		}
		length++;
	}
	lseek(fd, length * (-1), SEEK_CUR); // 길이를 파악한 후 처음으로 돌아감
	char* line = malloc((sizeof(char) * length)+10);
	while(1) { // 파일을 읽어 들여서 한문장씩 나눠서 return 해주는 반복문
		result = read(fd, &line[i], 1);
		if (result > 0) {
			if (line[i] == '\\') { // 긴문장일 경우 \를 무시하고 한문장으로 처리함
				i = i - 1;
				lseek(fd, 1, SEEK_CUR);
			}
			else if (line[i] == '\n') { // 개행을 만나는 경우 문장 끝으로파악
				line[i] = '\0';
				break;
			}
			i++;
		}
		else if (result == 0) {
			return NULL;
		}
	}
	for (int i = 0 ; i < strlen(line) ; i++) { // 주석 문장 처리
		if (line[i] == '#') {
			for(int j = 0 ; j < length_word(line, i+1) ; j++){ //#을 만난 경우 #을 제외한 뒤에 문장들은 읽어 들이지 않음( 주석문 외에 다른 문장뒤에 #이 오는 경우 에러처리를 위해 #은 남겨둠)
				line[i+j+1]='\0';
			}
			break;
		}
	}
	return line;
}
// 문장을 나누는 함수(시작지점부터 주어진 offset 만큼 나누어서 리턴 함)
char* substr(char*string, int st_point, int offset){
	int i;
	int j = 0;
	char* temp = malloc(sizeof(char) * strlen(string));
	for (i = st_point - 1 ; i < (st_point + offset - 1) ; i++){
		temp[j] = string[i];
		j++;
	}
	return temp;
}
// 문장에서 = 이나 : 을 기준으로 양 옆에 공백을 제거해주는 함수
void remove_blank(char* string){
	int i = 0;
	while (string[i] != '=' && string[i] != ':') { // =이나 :을 만날떄 까지 전진
		if(i > strlen(string)-1){
			return;
		}
		if(string[i] == ' ' || string[i] == '\t'){ // =이나 :을 만나기 전까지 만나는 
			remove_letter(string, i+1);
			continue;
		}
		i++;
	}
	i++;
	while (string[i] == ' ' || string[i] == '\t'){
		remove_letter(string, i+1);
	}
}
// 문자에서 주어진 위치의 글자를 제거 하는 함수
void remove_letter(char* string, int pos){
	pos = pos - 1;
	int length=strlen(string);
	for(int i = pos ; i < length-1 ; i++){
		string[i]=string[i+1];
	}
	string[length-1] ='\0';
	
	
}
// 주어진 위치에서부터 공백의 길이를 return 해주는 함수
int length_blank(char* string, int pos){
	int length = 0;
	pos = pos - 1;
	if (string[pos] != ' ' && string[pos] != '\t') {
		return 1;
	}
	else {
		while(string[pos] == ' ' || string[pos] == '\t') {
			length++;
			pos++;
		}
		return length;
	}
}

// 주어진 문자열에서 target이 되는 문자열을 내가 원하는 문자열로 치환 해주는 함수
char *replaceValue(char *strInput, char *strTarget, char *strChange)
{
    char* strResult;
    char* strTemp;
    int i = 0;
    int nCount = 0;
    int nTargetLength = strlen(strTarget);
 
    if (nTargetLength < 1)
        return strInput;
 
    int nChangeLength = strlen(strChange);
 
    if (nChangeLength != nTargetLength) //두 문자열의 길이가 다른 경우
    {
        for (i = 0; strInput[i] != '\0';)
        {
            if (memcmp(&strInput[i], strTarget, nTargetLength) == 0)
            {
                nCount++;
                i += nTargetLength;
            }
            else i++;
        }
    }
    else
    {
        i = strlen(strInput);
    }
 
    strResult = (char *) malloc(i + 1 + nCount * (nChangeLength - nTargetLength));
    if (strResult == NULL) return NULL;
 
 
    strTemp = strResult;
    while (*strInput)
    {
        if (memcmp(strInput, strTarget, nTargetLength) == 0)
        {
            memcpy(strTemp, strChange, nChangeLength);
            strTemp += nChangeLength;
            strInput  += nTargetLength;
        }
        else
        {
            *strTemp++ = *strInput++;
        }
    }
 
    *strTemp = '\0';
 
    return strResult;
}


