#include <string.h>
#include <stdio.h>


/*ֻ�ܵ���ִ�����*/

/* �������� */
/* get_input����������ַ�������buf������ */
int get_input(char buf[]) {
    // buf�����ʼ��
    memset(buf, 0x00, BUFFSIZE);
    memset(backupBuf, 0x00, BUFFSIZE);        

    fgets(buf, BUFFSIZE, stdin);
    // ȥ��fgets������ĩβ\n�ַ�
    buf[strlen(buf) - 1] = '\0';
    return strlen(buf);
}
