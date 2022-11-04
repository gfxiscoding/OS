#include <string.h>
#include <stdio.h>


/*只能单条执行语句*/

/* 函数定义 */
/* get_input接受输入的字符并存入buf数组中 */
int get_input(char buf[]) {
    // buf数组初始化
    memset(buf, 0x00, BUFFSIZE);
    memset(backupBuf, 0x00, BUFFSIZE);        

    fgets(buf, BUFFSIZE, stdin);
    // 去除fgets带来的末尾\n字符
    buf[strlen(buf) - 1] = '\0';
    return strlen(buf);
}
