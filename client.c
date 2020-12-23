#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "tools.h"
#include "bank.h"

#define BUF_SIZE (4096)

int cli_fd;
Acc acc = {1,"1","1","1",1,1};

void show_acc(Acc* acc)
{
	printf("%d %s %s %s %f %d\n",acc->type,acc->bank,acc->card,acc->pass,acc->balance,acc->islock);
}

void acc_to_str(Acc* acc,char* buf)
{
	sprintf(buf,"%d %s %s %s %f %d",acc->type,acc->bank,acc->card,acc->pass,acc->balance,acc->islock);
}

// 主菜单
void main_menu(void)
{
	puts("******欢迎使用指针银行******");
	puts("1、开户		2、登陆");
	puts("3、销户		4、解锁");
	puts("0、退出");
}

// 子菜单
void sub_menu(void)
{
	puts("******恭喜您登陆成功******");
	puts("1、存款		2、取款");
	puts("3、查询		4、转账");
	puts("5、改密		6、退出");
}

// 存款
void save(void)
{
	acc.type = M_SAVE;
	printf("请输入存款金额：");
	scanf("%f",&acc.balance);
}

// 取款
void take(void)
{
	acc.type = M_TAKE;
	printf("请输入取款金额：");
	scanf("%f",&acc.balance);
}

// 查询
void query(void)
{
	acc.type = M_QUERY;
}

// 转账
void transfer(void)
{
	acc.type = M_TRANSFER;
	printf("请输入目标银行卡号：");
	get_str(acc.card,20);
	printf("请输入转账金额：");
	scanf("%f",&acc.balance);
}

// 改密
void repass(void)
{
	char pass1[20] = {} , pass2[20] = {};
	printf("请输入新密码：");
	get_pass(pass1,20,true);
	printf("请再次输入新密码：");
	get_pass(pass2,20,true);
	if(strcmp(pass1,pass2))
	{
		puts("两次输入的密码不相同，修改失败!");
	}
	else
	{
		acc.type = M_REPASS;
		strcpy(acc.pass,pass1);
	}
}

// 开户
void open_acc(void)
{
	acc.type = M_OPEN;
	printf("请输入身份证号：");
	get_str(acc.card,20);
	printf("请输入开户金额：");
	scanf("%f",&acc.balance);
}

// 登陆
void login(void)
{
	acc.type = M_LOGIN;
	printf("请输入银行卡号：");
	get_str(acc.bank,20);
	printf("请输入密码：");
	//get_str(acc.pass,20);
	get_pass(acc.pass,20,true);
	
	// 向服务端发送消息
	char buf[BUF_SIZE];
	sprintf(buf,"%d %s %s %s %f %d",acc.type,acc.bank,acc.card,acc.pass,acc.balance,acc.islock);
	int send_size = write(cli_fd,buf,strlen(buf)+1);
		
	// 从服务端接收消息
	int recv_size = read(cli_fd,buf,sizeof(buf));

	// 显服务端执行的结果
	puts(buf);
	anykey_continue();
	if('Y' != buf[0])
	{
		return;
	}
	
	for(;;)
	{
		sub_menu();
		switch(get_cmd('1','6')-'0'+4)
		{
		case M_SAVE: 	save();		break;	// 存款
		case M_TAKE: 	take();		break;	// 取款
		case M_QUERY: 	query();	   break;	// 查询
		case M_TRANSFER:transfer();	break;	// 转账
		case M_REPASS:	repass();	   break;	// 改密
		default: return ;break;// 退出
		}
		
		// 向服务端发送消息
		char msg[BUF_SIZE];
		sprintf(msg,"%d %s %s %s %f %d",acc.type,acc.bank,acc.card,acc.pass,acc.balance,acc.islock);
		write(cli_fd,msg,strlen(msg)+1);
		
		// 从服务端接收消息
		read(cli_fd,msg,sizeof(msg));
		
		// 显服务端执行的结果
		puts(msg);
		anykey_continue();
	}

}

// 销户
void destory(void)
{
	acc.type = M_DESTORY;
	printf("请输入银行卡号：");
	get_str(acc.bank,20);
	printf("请输入身份证号：");
	get_str(acc.card,20);
	printf("请输入密码：");
	get_pass(acc.pass,20,true);
}

// 解锁
void unlock(void)
{
	acc.type = M_UNLOCK;
	printf("请输入银行卡号：");
	get_str(acc.bank,20);
	printf("请输入身份证号：");
	get_str(acc.card,20);
	printf("请输入密码：");
	get_pass(acc.pass,20,true);
}

void exit_bank(void)
{
	char buf[BUF_SIZE];
	sprintf(buf,"quit");
	write(cli_fd,buf,strlen(buf)+1);
	close(cli_fd);
	exit(0);
}

int main()
{
	//创建socket对象
	cli_fd = socket(AF_INET,SOCK_STREAM,0);
	if(0 > cli_fd)
	{
		perror("socket");
		return -1;
	}

	//准备通信地址
	struct sockaddr_in addr = {};
	addr.sin_family = AF_INET;
	addr.sin_port = htons(6789);
	addr.sin_addr.s_addr = inet_addr("10.0.2.15");
	socklen_t addrlen = sizeof(addr);

	//链接服务器
	if(connect(cli_fd,(struct sockaddr*)&addr,addrlen))
	{
		perror("connect");
		return -1;
	}

	for(;;)
	{
		main_menu();
		switch(get_cmd('0','4')-'0')
		{
		case M_EXIT:	exit_bank();	break;	// 退出
		case M_OPEN:	open_acc();	break;	// 开户
		case M_LOGIN: 	login();	continue;	// 登陆
		case M_DESTORY: destory();	break;	// 销户
		case M_UNLOCK:	unlock();	break; 	// 解锁
		}
		
		char buf[BUF_SIZE];
		//sprintf(buf,"%d %s %s %s %f %d",acc.type,acc.bank,acc.card,acc.pass,acc.balance,acc.islock);
		acc_to_str(&acc,buf);
		
		// 向服务端发送消息
		int send_size = write(cli_fd,buf,strlen(buf)+1);
		
		// 从服务端接收消息
		int recv_size = read(cli_fd,buf,sizeof(buf));
		
		// 显服务端执行的结果
		puts(buf);
		anykey_continue();
	}
}
