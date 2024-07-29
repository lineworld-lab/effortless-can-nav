1. ecsh 사용법

ㅁ ecsh_help : help 메시지 출력

ㅁ ecsh_vendor : 외부 라이브러리인 SOEM과 cJSON을 다운로드 및 빌드

ㅁ ecsh_build : 기존에 build가 있다면 삭제 후 새 build 폴더 만든 후 cmake 빌드

ㅁ ecsh_vendor_rt : 실시간 라이브러리인 EtherCAT 설치

ㅁ ecsh_build_rt : 실시간 빌드 수행

ㅁ ecsh_run : 실행

2. 2. 

ㅁ int RunInteractive() {  } : Keep 변수는 0으로 초기화, 이 조건이 참일 때까지 반복문이 실행.

ㅁ While (keep == 0) {  } : 명령어가 입력될 때까지 무한 루프 실행

ㅁ fgets(line, MAX_BUFFLEN, stdin); : fgets 함수를 사용해 입력을 받고 내용은 line 배열에 저장.

ㅁ int line_len = strlen(line) : 입력된 문자열의 길이를 확인

ㅁ if (line[i] == '\n') {
        line[i] = '\0'; : 개행 문자(\n)를 널 문자(\0)으로 바꾼다. 문자열의 끝을 명시하는 역활

ㅁ char* tok = strtok(line, delim) : strtok 함수를 사용해 입력된 문자열(“ ”)을 기준으로 토큰화한다.

ㅁ if (arg_len == 0) ~ else : 첫 번째 토큰은 malloc 사용하여 메모리 할당 | 그 이후 토큰은 realloc 사용하여 메모리 재할당

ㅁ if (strcmp(runtime_args[0], "connect") == 0) : ret_code가 0보다 작으면 실패 메시지 출력

ㅁ else if (strcmp(runtime_args[0], "send") == 0) : send 명령어를 입력 후 데이터 전송
 	res 배열은 전송 결과를 저장 후 전송 여부에 따라 메시지 출력

ㅁ else if (strcmp(runtime_args[0], "disconnect") == 0) : 연결을 종료

ㅁ else { } : 명령어와 일치하지 않은 경우, 유효하지 않은 인수 메시지를 출력

ㅁ int ConnectTo(char* res, int arg_len, char** runtime_args) : 

ㅁ if (arg_len < 3) : 인수의 개수가 3 미만인 경우 오류 메시지를 출력하고 –1 반환
		     필요한 인수는 address와 port 

ㅁ ret_code = InitECCmdGatewayWithTimeout(res, addr, port, 5000); : 주소와 포트로 연결 시도
