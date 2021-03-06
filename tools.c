#include <getch.h>
#include "tools.h"

// 清理输入缓冲区
void clear_stdin(void)
{
	stdin->_IO_read_ptr = stdin->_IO_read_end;
}

// 获取指令
int get_cmd(char start,char end)
{
	puts("--------------------");
	printf("请输入指令：");

	int cmd = 0;
	for(;;)
	{
		cmd = getch();
		if(cmd >= start && cmd <= end)
		{
			printf("%c\n",cmd);
			break;
		}
	}
	return cmd;
}

// 获取字符串
char* get_str(char* str,size_t len)
{
	clear_stdin();
	fgets(str,len,stdin);
	size_t size = strlen(str);

	if('\n' == str[size-1])
	{
		str[size-1] = '\0';
	}
	else
	{
		clear_stdin();
	}

	return str;
}

// 获取密码
char* get_pass(char* pass,size_t len,bool flag)
{
	clear_stdin();
	size_t index = 0;

	while(index < len)
	{
		char key = getch();
		if(127 == key)
		{
			if(index > 0)
			{
				index--;
				if(flag) printf("\b \b");
			}
			continue;
		}

		if('\n' == key) break;

		pass[index++] = key;
		if(flag) printf("*");
	}

	pass[index] = '\0';
	printf("\n");
	return pass;
}

// 初始化银行卡号
void init_bank(const char* path,int bank)
{
	if(0 == access(path,F_OK)) return;
	
	puts(path);
	int fd = open(path,O_WRONLY|O_CREAT|O_TRUNC,0644);
	if(0 > fd)
	{
		printf("%d:%s：open:%m\n",__LINE__,__func__);
		return;
	}
	
	write(fd,&bank,sizeof(bank));
	close(fd);
}

// 生成一个银行卡号
int get_bank(const char* path)
{
	int fd = open(path,O_RDWR);
	if(0 > fd)
	{
		perror("open");
		return -1;
	}
	
	int bank = 0;
	read(fd,&bank,sizeof(bank));
	bank++;
	lseek(fd,0,SEEK_SET);
	write(fd,&bank,sizeof(bank));
	return bank;
}

// 按任意键继续
void anykey_continue(void)
{
	clear_stdin();
	puts("按任意键继续......");
	getch();
}



