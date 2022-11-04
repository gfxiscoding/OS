/* ͷ�ļ� */
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>
#include <dirent.h>             // readdir����

/* �궨�� */
#define IN 1
#define OUT 0
#define MAX_CMD 10
#define BUFFSIZE 100
#define MAX_CMD_LEN 100


/* ȫ�ֱ��� */
int argc;                                       // ��Ч��������
char* argv[MAX_CMD];                            // ��������
char command[MAX_CMD][MAX_CMD_LEN];             // ��������
char buf[BUFFSIZE];                             // ��������Ĳ�������
char backupBuf[BUFFSIZE];                       // ��������ı���
char curPath[BUFFSIZE];                         // ��ǰshell����·��
int i, j;                                       // ѭ������
int commandNum;                                 // �Ѿ�����ָ����Ŀ
char history[MAX_CMD][BUFFSIZE];                // �����ʷ����


int get_input(char buf[]);                      // ����ָ�����buf����
void parse(char* buf);                          // �����ַ���
void do_cmd(int argc, char* argv[]);
    int callCd(int argc);                       // ִ��cdָ��
    int printHistory(char command[MAX_CMD][MAX_CMD_LEN]);   // ��ӡ��ʷָ��
    // �ض���ָ��
    int commandWithOutputRedi(char buf[BUFFSIZE]);          // ִ������ض���
    int commandWithInputRedi(char buf[BUFFSIZE]);           // ִ�������ض�������
    int commandWithReOutputRedi(char buf[BUFFSIZE]);        // ִ������ض���׷��д
    int commandWithPipe(char buf[BUFFSIZE]);                // ִ�йܵ�����
    int commandInBackground(char buf[BUFFSIZE]);
    void myTop();                                           // ִ��mytopָ��


/* �������� */
/* get_input����������ַ�������buf������ */
int get_input(char buf[]) {
    // buf�����ʼ��
    memset(buf, 0x00, BUFFSIZE);
    memset(backupBuf, 0x00, BUFFSIZE);        

    fgets(buf, BUFFSIZE, stdin);
    // ȥ��fgets������ĩβ\n�ַ�����ֹ�ӵ�������
    buf[strlen(buf) - 1] = '\0';
    return strlen(buf);
}

void parse(char* buf) {
    // ��ʼ��argv�����argc
    for (i = 0; i < MAX_CMD; i++) {
        argv[i] = NULL;
        for (j = 0; j < MAX_CMD_LEN; j++)
            command[i][j] = '\0';
    }
    argc = 0;
    // ���в����ı���buf����, Ϊbuf������������
    strcpy(backupBuf, buf);
    /** ����command����
     *  ��������Ϊ"ls -a"
     *  strcmp(command[0], "ls") == 0 ������
     *  strcmp(command[1], "-a") == 0 ����
     */  
    //�Կո���Ϊ�����ķֽ��
    int len = strlen(buf);
    for (i = 0, j = 0; i < len; ++i) {
        if (buf[i] != ' ') {
            command[argc][j++] = buf[i];
        } else {
            if (j != 0) {//���ڲ������������ո�
                command[argc][j] = '\0';//��һ�������ַ��������'\0'�ֽ�
                ++argc;
                j = 0;
            }
        }
    }
    if (j != 0) {
        command[argc][j] = '\0';
    }

    /** ����argv����
     *  ��������bufΪ"ls -a"
     *  strcmp(argv[0], "ls") == 0 ������
     *  strcmp(argv[1], "-a") == 0 ����*/
    argc = 0;
    int flg = OUT;
    for (i = 0; buf[i] != '\0'; i++) {
        if (flg == OUT && !isspace(buf[i])) {
            flg = IN;
            argv[argc++] = buf + i;
        } else if (flg == IN && isspace(buf[i])) {
            flg = OUT;
            buf[i] = '\0';
        }
    }
    argv[argc] = NULL;
}

void do_cmd(int argc, char* argv[]) {
    pid_t pid;
    /* ʶ��program���� */
    // ʶ���ض����������
    for (j = 0; j < MAX_CMD; j++) {
        if (strcmp(command[j], ">") == 0) {
            strcpy(buf, backupBuf);
            int sample = commandWithOutputRedi(buf);
            return;
        }
    }
    // ʶ�������ض���
    for (j = 0; j < MAX_CMD; j++) {
        if (strcmp(command[i], "<") == 0) {
            strcpy(buf, backupBuf);
            int sample = commandWithInputRedi(buf);
            return;
        }
    }
    // ʶ��׷��д�ض���
    for (j = 0; j < MAX_CMD; j++) {
        if (strcmp(command[j], ">>") == 0) {
            strcpy(buf, backupBuf);
            int sample = commandWithReOutputRedi(buf);
            return;
        }
    }

    // ʶ��ܵ�����
    for (j = 0; j < MAX_CMD; j++) {
        if (strcmp(command[j], "|") == 0) {
            strcpy(buf, backupBuf);
            int sample = commandWithPipe(buf);
            return;
        }
    }

    // ʶ���̨��������
    for (j = 0; j < MAX_CMD; j++) {
        if (strcmp(command[j], "&") == 0) {
            strcpy(buf, backupBuf);
            int sample = commandInBackground(buf);
            return;
        }
    }





    /* ʶ��shell�������� */
    if (strcmp(command[0], "cd") == 0) {
        int res = callCd(argc);
        if (!res) printf("cdָ���������!");
    } else if (strcmp(command[0], "history") == 0) {
        printHistory(command);
    } else if (strcmp(command[0], "mytop") == 0) {
        myTop();
        return;
    } else if (strcmp(command[0], "exit") == 0) {
        exit(0);
    } else {//ִ�г���
        switch(pid = fork()) {
            // fork�ӽ���ʧ��
            case -1:
                printf("�����ӽ���δ�ɹ�");
                return;
            // �����ӽ���
            case 0:
                {   /* ����˵����execvp()���PATH ����������ָ��Ŀ¼�в��ҷ��ϲ���file ���ļ���, �ҵ����ִ�и��ļ�, 
                     * Ȼ�󽫵ڶ�������argv ��������ִ�е��ļ���
                     * ����ֵ�����ִ�гɹ��������᷵��, ִ��ʧ����ֱ�ӷ���-1, ʧ��ԭ�����errno ��. 
                     * */
                    execvp(argv[0], argv);
                    // ���뽡׳��: ����ӽ���δ���ɹ�ִ��, �򱨴�
                    printf("%s: �����������\n", argv[0]);
                    // exit������ֹ��ǰ����, �����ڲ���Ϊ1ʱ, �������ϵͳ����ý������쳣����ֹ
                    exit(1);
                }
            default: {
                    int status;
                    waitpid(pid, &status, 0);      // �ȴ��ӽ��̷���
                    int err = WEXITSTATUS(status); // ��ȡ�ӽ��̵ķ�����

                    if (err) { 
                        printf("Error: %s\n", strerror(err));
                    }                    
            }
        }
    }
}

int callCd(int argc) {
    // resultΪ1����ִ�гɹ�, Ϊ0����ִ��ʧ��
    int result = 1;
    if (argc != 2) {
        printf("ָ����Ŀ����!");
    } else {
        int ret = chdir(command[1]);
        if (ret) return 0;
    }

    if (result) {
        char* res = getcwd(curPath, BUFFSIZE);
        if (res == NULL) {
            printf("�ļ�·��������!");
        }
        return result;
    }
    return 0;
}


int printHistory(char command[MAX_CMD][MAX_CMD_LEN]) {
    int n = atoi(command[1]);

    for (i = n; i > 0 && commandNum - i >= 0; i--) {
        printf("%d\t%s\n", n - i + 1, history[commandNum - i]);
    }
    return 0;
}

int commandWithOutputRedi(char buf[BUFFSIZE]) {
    strcpy(buf, backupBuf);
    char outFile[BUFFSIZE];
    memset(outFile, 0x00, BUFFSIZE);
    int RediNum = 0;
    for ( i = 0; i + 1 < strlen(buf); i++) {
        if (buf[i] == '>' && buf[i + 1] == ' ') {
            RediNum++;
            break;
        }
    }
    if (RediNum != 1) {
        printf("����ض���ָ����������!");
        return 0;
    }

    for (i = 0; i < argc; i++) {
        if (strcmp(command[i], ">") == 0) {
            if (i + 1 < argc) {
                strcpy(outFile, command[i + 1]);
            } else {
                printf("ȱ������ļ�!");
                return 0;
            }
        }
    }

    // ָ��ָ�, outFileΪ����ļ�, bufΪ�ض������ǰ������
    for (j = 0; j < strlen(buf); j++) {
        if (buf[j] == '>') {
            break;
        }
    }
    buf[j - 1] = '\0';
    buf[j] = '\0';
    // ����ָ��
    parse(buf);
    pid_t pid;
    switch(pid = fork()) {
        case -1: {
            printf("�����ӽ���δ�ɹ�");
            return 0;
        }
        // �����ӽ���:
        case 0: {
            // �������ض���
            int fd;
            fd = open(outFile, O_WRONLY|O_CREAT|O_TRUNC, 7777);
            // �ļ���ʧ��
            if (fd < 0) {
                exit(1);
            }
            dup2(fd, STDOUT_FILENO);  
            execvp(argv[0], argv);
            if (fd != STDOUT_FILENO) {
                close(fd);
            }
            // ���뽡׳��: ����ӽ���δ���ɹ�ִ��, �򱨴�
            printf("%s: �����������\n", argv[0]);
            // exit������ֹ��ǰ����, �����ڲ���Ϊ1ʱ, �������ϵͳ����ý������쳣����ֹ
            exit(1);
        }
        default: {
            int status;
            waitpid(pid, &status, 0);       // �ȴ��ӽ��̷���
            int err = WEXITSTATUS(status);  // ��ȡ�ӽ��̵ķ�����
            if (err) { 
                printf("Error: %s\n", strerror(err));
            } 
        }                        
    }

}

int commandWithInputRedi(char buf[BUFFSIZE]) {
    strcpy(buf, backupBuf);
    char inFile[BUFFSIZE];
    memset(inFile, 0x00, BUFFSIZE);
    int RediNum = 0;
    for ( i = 0; i + 1< strlen(buf); i++) {
        if (buf[i] == '<' && buf[i + 1] == ' ') {
            RediNum++;
            break;
        }
    }
    if (RediNum != 1) {
        printf("�����ض���ָ����������!");
        return 0;
    }

    for (i = 0; i < argc; i++) {
        if (strcmp(command[i], "<") == 0) {
            if (i + 1 < argc) {
                strcpy(inFile, command[i + 1]);
            } else {
                printf("ȱ������ָ��!");
                return 0;
            }
        }
    }

    // ָ��ָ�, InFileΪ����ļ�, bufΪ�ض������ǰ������
    for (j = 0; j < strlen(buf); j++) {
        if (buf[j] == '<') {
            break;
        }
    }
    buf[j - 1] = '\0';
    buf[j] = '\0';
    parse(buf);
    pid_t pid;
    switch(pid = fork()) {
        case -1: {
            printf("�����ӽ���δ�ɹ�");
            return 0;
        }
        // �����ӽ���:
        case 0: {
            // ��������ض���
            int fd;
            fd = open(inFile, O_RDONLY, 7777);
            // �ļ���ʧ��
            if (fd < 0) {
                exit(1);
            }
            dup2(fd, STDIN_FILENO);  
            execvp(argv[0], argv);
            if (fd != STDIN_FILENO) {
                close(fd);
            }
            // ���뽡׳��: ����ӽ���δ���ɹ�ִ��, �򱨴�
            printf("%s: �����������\n", argv[0]);
            // exit������ֹ��ǰ����, �����ڲ���Ϊ1ʱ, �������ϵͳ����ý������쳣����ֹ
            exit(1);
        }
        default: {
            int status;
            waitpid(pid, &status, 0);       // �ȴ��ӽ��̷���
            int err = WEXITSTATUS(status);  // ��ȡ�ӽ��̵ķ�����
            if (err) { 
                printf("Error: %s\n", strerror(err));
            } 
        }                        
    }

}


int commandWithReOutputRedi(char buf[BUFFSIZE]) {
    strcpy(buf, backupBuf);
    char reOutFile[BUFFSIZE];
    memset(reOutFile, 0x00, BUFFSIZE);
    int RediNum = 0;
    for ( i = 0; i + 2 < strlen(buf); i++) {
        if (buf[i] == '>' && buf[i + 1] == '>' && buf[i + 2] == ' ') {
            RediNum++;
            break;
        }
    }
    if (RediNum != 1) {
        printf("׷������ض���ָ����������!");
        return 0;
    }

    for (i = 0; i < argc; i++) {
        if (strcmp(command[i], ">>") == 0) {
            if (i + 1 < argc) {
                strcpy(reOutFile, command[i + 1]);
            } else {
                printf("ȱ������ļ�!");
                return 0;
            }
        }
    }

    // ָ��ָ�, outFileΪ����ļ�, bufΪ�ض������ǰ������
    for (j = 0; j + 2 < strlen(buf); j++) {
        if (buf[j] == '>' && buf[j + 1] == '>' 
            && buf[j + 2] == ' ') {
            break;
        }
    }
    buf[j - 1] = '\0';
    buf[j] = '\0';
    // ����ָ��
    parse(buf);
    pid_t pid;
    switch(pid = fork()) {
        case -1: {
            printf("�����ӽ���δ�ɹ�");
            return 0;
        }
        // �����ӽ���:
        case 0: {
            // �������ض���
            int fd;
            fd = open(reOutFile, O_WRONLY|O_APPEND|O_CREAT|O_APPEND, 7777);
            // �ļ���ʧ��
            if (fd < 0) {
                exit(1);
            }
            dup2(fd, STDOUT_FILENO);  
            execvp(argv[0], argv);
            if (fd != STDOUT_FILENO) {
                close(fd);
            }
            // ���뽡׳��: ����ӽ���δ���ɹ�ִ��, �򱨴�
            printf("%s: �����������\n", argv[0]);
            // exit������ֹ��ǰ����, �����ڲ���Ϊ1ʱ, �������ϵͳ����ý������쳣����ֹ
            exit(1);
        }
        default: {
            int status;
            waitpid(pid, &status, 0);       // �ȴ��ӽ��̷���
            int err = WEXITSTATUS(status);  // ��ȡ�ӽ��̵ķ�����
            if (err) { 
                printf("Error: %s\n", strerror(err));
            } 
        }                        
    }   
}


int commandWithPipe(char buf[BUFFSIZE]) {
    // ��ȡ�ܵ����ŵ�λ������
    for(j = 0; buf[j] != '\0'; j++) {
        if (buf[j] == '|')
            break;
    }

    // ����ָ��, ���ܵ�����ǰ���ָ����������������
    // outputBuf��Źܵ�ǰ������, inputBuf��Źܵ��������
    char outputBuf[j];
    memset(outputBuf, 0x00, j);
    char inputBuf[strlen(buf) - j];
    memset(inputBuf, 0x00, strlen(buf) - j);
    for (i = 0; i < j - 1; i++) {
        outputBuf[i] = buf[i];
    }
    for (i = 0; i < strlen(buf) - j - 1; i++) {
        inputBuf[i] = buf[j + 2 + i];
    }


    int pd[2];
    pid_t pid;
    if (pipe(pd) < 0) {
        perror("pipe()");
        exit(1);
    }

    pid = fork();
    if (pid < 0) {
        perror("fork()");
        exit(1);
    }


    if (pid == 0) {                     // �ӽ���д�ܵ�
        close(pd[0]);                   // �ر��ӽ��̵Ķ���
        dup2(pd[1], STDOUT_FILENO);     // ���ӽ��̵�д����Ϊ��׼���
        parse(outputBuf);
        execvp(argv[0], argv);
        if (pd[1] != STDOUT_FILENO) {
            close(pd[1]);
        }
    }else {                              // �����̶��ܵ�
        /** �ؼ�����
         *  �ӽ���д�ܵ���Ϻ���ִ�и����̶��ܵ�, ������Ҫ��wait�����ȴ��ӽ��̷��غ��ٲ���
         */
        int status;
        waitpid(pid, &status, 0);       // �ȴ��ӽ��̷���
        int err = WEXITSTATUS(status);  // ��ȡ�ӽ��̵ķ�����
        if (err) { 
            printf("Error: %s\n", strerror(err));
        }

        close(pd[1]);                    // �رո����̹ܵ���д��
        dup2(pd[0], STDIN_FILENO);       // �ܵ����˶������ض���Ϊ��׼����
        parse(inputBuf);
        execvp(argv[0], argv);
        if (pd[0] != STDIN_FILENO) {
            close(pd[0]);
        }       
    }

    return 1;
}


int commandInBackground(char buf[BUFFSIZE]) {
    char backgroundBuf[strlen(buf)];
    memset(backgroundBuf, 0x00, strlen(buf));
    for (i = 0; i < strlen(buf); i++) {
        backgroundBuf[i] = buf[i];
        if (buf[i] == '&') {
            backgroundBuf[i] = '\0';
            backgroundBuf[i - 1] = '\0';
            break;
        }
    }

    pid_t pid;
    pid = fork();
    if (pid < 0) {
        perror("fork()");
        exit(1);
    }

    if (pid == 0) {
        // ��stdin��stdout��stderr�ض���/dev/null
        freopen( "/dev/null", "w", stdout );
        freopen( "/dev/null", "r", stdin ); 
        signal(SIGCHLD,SIG_IGN);
        parse(backgroundBuf);
        execvp(argv[0], argv);
        printf("%s: �����������\n", argv[0]);
        // exit������ֹ��ǰ����, �����ڲ���Ϊ1ʱ, �������ϵͳ����ý������쳣����ֹ
        exit(1);
    }else {
        exit(0);
    }
}


void myTop() {
    FILE *fp = NULL;                    // �ļ�ָ��
    char buff[255];

    /* ��ȡ����һ: 
       �����ڴ��С��
       �����ڴ��С��
       �����С�� */
    fp = fopen("/proc/meminfo", "r");   // ��ֻ����ʽ��meminfo�ļ�
    fgets(buff, 255, (FILE*)fp);        // ��ȡmeminfo�ļ����ݽ�buff
    fclose(fp);

    // ��ȡ pagesize
    int i = 0, pagesize = 0;
    while (buff[i] != ' ') {
        pagesize = 10 * pagesize + buff[i] - 48;
        i++;
    }

    // ��ȡ ҳ���� total
    i++;
    int total = 0;
    while (buff[i] != ' ') {
        total = 10 * total + buff[i] - 48;
        i++;
    }

    // ��ȡ����ҳ�� free
    i++;
    int free = 0;
    while (buff[i] != ' ') {
        free = 10 * free + buff[i] - 48;
        i++;
    }

    // ��ȡ���ҳ����largest
    i++;
    int largest = 0;
    while (buff[i] != ' ') {
        largest = 10 * largest + buff[i] - 48;
        i++;
    }

    // ��ȡ����ҳ���� cached
    i++;
    int cached = 0;
    while (buff[i] >= '0' && buff[i] <= '9') {
        cached = 10 * cached + buff[i] - 48;
        i++;
    }

    // �����ڴ��С = (pagesize * total) / 1024 ��λ KB
    int totalMemory  = pagesize / 1024 * total;
    // �����ڴ��С = pagesize * free) / 1024 ��λ KB
    int freeMemory   = pagesize / 1024 * free;
    // �����С    = (pagesize * cached) / 1024 ��λ KB
    int cachedMemory = pagesize / 1024 * cached;

    printf("totalMemory  is %d KB\n", totalMemory);
    printf("freeMemory   is %d KB\n", freeMemory);
    printf("cachedMemory is %d KB\n", cachedMemory);

    /* 2. ��ȡ����2
        ���̺���������
     */
    fp = fopen("/proc/kinfo", "r");     // ��ֻ����ʽ��meminfo�ļ�
    memset(buff, 0x00, 255);            // ��ʽ��buff�ַ���
    fgets(buff, 255, (FILE*)fp);        // ��ȡmeminfo�ļ����ݽ�buff
    fclose(fp);

    // ��ȡ��������
    int processNumber = 0;
    i = 0;
    while (buff[i] != ' ') {
        processNumber = 10 * processNumber + buff[i] - 48;
        i++;
    }
    printf("processNumber = %d\n", processNumber);

    // ��ȡ��������
    i++;
    int tasksNumber = 0;
    while (buff[i] >= '0' && buff[i] <= '9') {
        tasksNumber = 10 * tasksNumber + buff[i] - 48;
        i++;
    }
    printf("tasksNumber = %d\n", tasksNumber);


    // /* 3. ��ȡpsinfo�е����� */
    DIR *d;
    struct dirent *dir;
    d = opendir("/proc");
    int totalTicks = 0, freeTicks = 0;
    if (d) {
        while ((dir = readdir(d)) != NULL) {                   // ����proc�ļ���
            if (strcmp(dir->d_name, ".") != 0 && 
                strcmp(dir->d_name, "..") != 0) {
                    char path[255];
                    memset(path, 0x00, 255);
                    strcpy(path, "/proc/");
                    strcat(path, dir->d_name);          // ���ӳ�Ϊ���·����
                    struct stat s;
                    if (stat (path, &s) == 0) {
                        if (S_ISDIR(s.st_mode)) {       // �ж�ΪĿ¼
                            strcat(path, "/psinfo");

                            FILE* fp = fopen(path, "r");
                            char buf[255];
                            memset(buf, 0x00, 255);
                            fgets(buf, 255, (FILE*)fp);
                            fclose(fp);

                            // ��ȡticks�ͽ���״̬
                            int j = 0;
                            for (i = 0; i < 4;) {
                                for (j = 0; j < 255; j++) {
                                    if (i >= 4) break;
                                    if (buf[j] == ' ') i++;
                                }
                            }
                            // ѭ������, buf[j]Ϊ���̵�״̬, ����S, W, R����״̬.
                            int k = j + 1;
                            for (i = 0; i < 3;) {               // ѭ��������kָ��ticksλ��
                                for (k = j + 1; k < 255; k++) {
                                    if (i >= 3) break;
                                    if (buf[k] == ' ') i++;
                                }
                            }
                            int processTick = 0;
                            while (buf[k] != ' ') {
                                processTick = 10 * processTick + buff[k] - 48;
                                k++;
                            }
                            totalTicks += processTick;
                            if (buf[j] != 'R') {
                                freeTicks += processTick;
                            }
                        }else continue;
                    }else continue;
                }
        }
    }
    printf("CPU states: %.2lf%% used,\t%.2lf%% idle",
           (double)((totalTicks - freeTicks) * 100) / (double)totalTicks,
           (double)(freeTicks * 100) / (double)totalTicks);
    return;
}


/* main���� */
int main() {
    // whileѭ��
    while(1) {
        printf("[myshell]$ ");
        // �����ַ�����buf����, ��������ַ���Ϊ0, �������˴�ѭ��
        if (get_input(buf) == 0)
            continue;
        strcpy(history[commandNum++], buf);
        strcpy(backupBuf, buf);
        parse(buf);
        do_cmd(argc, argv);
        argc = 0;
    }
}