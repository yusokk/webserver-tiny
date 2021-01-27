// tiny.c - A simple, iterative HTTP/1.0 Web server that uses the Get
//      method to serve static and dynamic content

#include "csapp.h"

void doit(int fd);
void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg);
void read_requesthdrs(rio_t *rp);
int parse_uri(char *uri, char *filename, char *cgiargs);
void serve_static(int fd, char *filename, int filesize);
void get_filetype(char *filename, char *filetype);
void serve_dynamic(int fd, char *filename, char *cgiargs);

int main(int argc, char **argv) {
    int listenfd, connfd;
    char hostname[MAXLINE], port[MAXLINE];
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;

    // check command-line args
    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1);
    }

    // 듣기 소켓 오픈
    listenfd = Open_listenfd(argv[1]);
    while (1) {
        clientlen = sizeof(clientaddr);
        // 무한 서버 루프를 실행하며, 반복적으로 연결 요청 접수
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        // 소켓주소구조체(SA)를 대응되는 호스트와 서비스 이름 스트링으로 변환하고, host와 service 버퍼로 복사
        Getnameinfo((SA *)&clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE, 0);
        printf("Accepted connection from (%s, %s)\n", hostname, port);
        // 트랜잭션 수행
        doit(connfd);
        // 자신 쪽 연결 끝을 닫음
        Close(connfd);
    }
}

void doit(int fd) {
    int is_static;
    struct stat sbuf;
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
    char filename[MAXLINE], cgiargs[MAXLINE];
    rio_t rio;

    // rio에 fd 연결
    Rio_readinitb(&rio, fd);
    // rio에서 buf로 읽기
    Rio_readlineb(&rio, buf, MAXLINE);
    printf("Request headers: \n");
    printf("%s", buf);
    // buf에 method, uri, version 입력
    sscanf(buf, "%s %s %s", method, uri, version);
    // "GET"이 아니면 501 에러 리턴
    if (strcasecmp(method, "GET")) {
        clienterror(fd, method, "501", "Not implemented", "Tiny does not implement this method");
        return;
    }
    // 읽어 들이고 다른 요청 헤더들 무시
    read_requesthdrs(&rio);

    // uri를 파일 이름과 비어 있을 수도 있는 cgi 인자 스트링으로 분석하고,
    // 요청이 정적 또는 동적 컨텐츠를 위한 것인지 나타내는 플래그 설정
    // 파일이 디스크 상에 있지 않으면 404에러 리턴
    is_static = parse_uri(uri, filename, cgiargs);
    printf("is static : %d\n", is_static);
    printf("filename : %s\n", filename);
    if (stat(filename, &sbuf) < 0) {
        clienterror(fd, filename, "404", "Not found", "Tiny couldn't find this file");
        return;
    }

    if (is_static) {
        // 보통(regular)파일인지, 읽기 권한을 가지고 있는지 검증
        if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode)) {
            clienterror(fd, filename, "403", "Forbidden", "Tiny couldn't run the CGI program");
            return;
        }
        // 정적 컨텐츠를 클라이언트에게 제공
        serve_static(fd, filename, sbuf.st_size);
    }
    else {
        // 보통 파일인지, 실행 가능한지 검증
        if (!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode)){
            clienterror(fd, filename, "403", "Forbidden", "Tiny couldn't run the CGI program");
            return;
        }
        // 동적 컨텐츠 제공
        serve_dynamic(fd, filename, cgiargs);
    }
}

void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg) {
    char buf[MAXLINE], body[MAXBUF];

    // build HTTP response body
    sprintf(body, "<html><title>Tiny Error</title>");
    sprintf(body, "%s<body bgcolor=""ffffff"">\r\n", body);
    sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
    sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
    sprintf(body, "%s<hr><em>The Tiny Web server</em>\r\n", body);

    // print HTTP response
    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-type: text/html\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body));
    Rio_writen(fd, buf, strlen(buf));
    Rio_writen(fd, body, strlen(body));
}

void read_requesthdrs(rio_t *rp) {
    char buf[MAXLINE];

    Rio_readlineb(rp, buf, MAXLINE);
    while (strcmp(buf, "\r\n")) {
        Rio_readlineb(rp, buf, MAXLINE);
        printf("%s", buf);
    }
    return;
}

int parse_uri(char *uri, char *filename, char *cgiargs) {
    char *ptr;

    // uri에 cgi-bin이 없다면 -> 요청이 정적 컨텐츠라면
    if  (!strstr(uri, "cgi-bin")) {
        // cgiargs 지움
        strcpy(cgiargs, "");
        // uri를 상대 리눅스 경로로 바꿈
        strcpy(filename, ".");
        strcat(filename, uri);
        // 만일 uri가 /로 끝나면 기본 파일 이름 추가
        if (uri[strlen(uri)-1] == '/') {
            strcat(filename, "home.html");
        }
        return 1;
    }
    // 동적 컨텐츠라면
    else {
        ptr = index(uri, '?');
        // ptr이 있으면 cgiargs를 ? 이후로 바꿔주고, ? 위치를 null로 만들어줌
        if (ptr) {
            strcpy(cgiargs, ptr + 1);
            *ptr = '\0';
        }
        else
            strcpy(cgiargs, "");
        strcpy(filename, ".");
        strcpy(filename, uri);
        return 0;
    }
}

void serve_static(int fd, char *filename, int filesize) {
    int srcfd;
    char *srcp, filetype[MAXLINE], buf[MAXBUF];

    // 응답 헤더 보내기, 버퍼에 헤더 써주고 fd로 보냄
    get_filetype(filename, filetype);
    sprintf(buf, "HTTP/1.0 200 OK\r\n");
    sprintf(buf, "%sServer: Tiny Web Server\r\n", buf);
    sprintf(buf, "%sConnection: close\r\n", buf);
    sprintf(buf, "%sContent-length: %d\r\n", buf, filesize);
    sprintf(buf, "%sContent-type: %s\r\n\r\n", buf, filetype);
    Rio_writen(fd, buf, strlen(buf));
    printf("Response headers:\n");
    printf("%s", buf);

    // 응답 바디 보내기, srcfd로 파일을 읽어오고 매핑한 뒤, srcfd 닫고 fd에 보냄, 다시 언매핑
    srcfd = Open(filename, O_RDONLY, 0);
    srcp = Mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);
    Close(srcfd);
    Rio_writen(fd, srcp, filesize);
    Munmap(srcp, filesize);
}

void get_filetype(char *filename, char *filetype) {
    if (strstr(filename, ".html"))
        strcpy(filetype, "text/html");
    else if (strstr(filename, ".gif"))
        strcpy(filetype, "image/gif");
    else if (strstr(filename, ".png"))
        strcpy(filename, "image/png");
    else if (strstr(filename, ".jpg"))
        strcpy(filetype, "image/jpeg");
    else
        strcpy(filetype, "text/plain");
}

void serve_dynamic(int fd, char *filename, char *cgiargs) {
    char buf[MAXLINE], *emptylist[] = {NULL};

    sprintf(buf, "HTTP/1.0 200 OK\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Server: Tiny Web Server\r\n");
    Rio_writen(fd, buf, strlen(buf));

    // 자식을 포크
    if (Fork() == 0) {
        //환경변수에 cgiargs로 초기화
        setenv("QUERY_STRING", cgiargs, 1);
        // 자식의 표준출력을 연결 파일 식별자로 재지정
        Dup2(fd, STDOUT_FILENO);
        // CGI프로그램 실행, 자식 컨텍스트에서 실행되므로 execve를 호출하기 전에 존재하던 열린 파일들과
        // 환경변수들에도 동일하게 접근가능, 그래서 CGI프로그램이 표준 출력에 쓰는 모든 것은 직접 클라이언트
        // 프로세스로 부모 프로세스의 어떤 간섭도 없이 전달됨
        Execve(filename, emptylist, environ);
    }
    // 부모는 자식이 종료되어 정리되는 것 기다리기 위해 블록
    Wait(NULL);
}