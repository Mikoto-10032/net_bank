#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>
#include "bank.h"
#include "tools.h"

#define BUF_SIZE (4096)

//服务端socket对象
int svr_fd;
//银行账户后7位id
int id = 1234001;

//解析字符串到结构体中
void str_to_acc(Acc* acc,char* buf)
{
	sscanf(buf,"%d %s %s %s %f %d",&acc->type,acc->bank,acc->card,acc->pass,&acc->balance,&acc->islock);
}

//显示数据
void show_acc(Acc* acc)
{
	printf("%d %s %s %s %f %d\n",acc->type,acc->bank,acc->card,acc->pass,acc->balance,acc->islock);
}

//开户
void open_acc(Acc* acc,char* buf)
{
	printf("%s\n",__func__);
	// 获取银行卡号
	sprintf(acc->bank,"19992000%d",id++);
	// 设置初始密码
	sprintf(acc->pass,"123456");
	// 设置锁定状态
	acc->islock = 0;
	
	char path[256] = {};
	sprintf(path,"%s%s",ACC_PATH,acc->bank);

	int fd = open(path,O_WRONLY|O_CREAT|O_TRUNC|O_EXCL,0644);
	if(0 > fd)
	{
		sprintf(buf,"开户失败，服务器正在升级，请稍候!");
	}
	else
	{
		sprintf(buf,"开户成功，您的账号为:%s",acc->bank);
		show_acc(acc);
		write(fd,acc,sizeof(Acc));
		close(fd);
	}
}

//登陆
void login(Acc* acc,char* buf)
{
	printf("%s\n",__func__);
	
	char path[256] = {};
	sprintf(path,"%s%s",ACC_PATH,acc->bank);
	printf("%s\n",path);
	// 判断银行卡号是否正确
	if(0 != access(path,F_OK))
	{
		sprintf(buf,"N:卡号不存在，请检查!");
		return;
	}
	
	int fd = open(path,O_RDWR);
	if(0 > fd)
	{
		perror("open");
		sprintf(buf,"N:服务器正在升级，登陆失败!");
		puts("登陆失败");
		return;
	}
	
	Acc local_acc = {};
	read(fd,&local_acc,sizeof(Acc));
	show_acc(&local_acc);
	if(3 <= local_acc.islock)
	{
		sprintf(buf,"N:登陆失败，帐号已经锁住!");
		puts("登陆失败，帐号已经锁住!");
	}
	else if(0 != strcmp(local_acc.pass,acc->pass))
	{	
		local_acc.islock++;
		sprintf(buf,"N:登陆失败，帐号或者密码输错(您还有%d次机会)!",3-local_acc.islock);
		printf("%s\n",acc->pass);
		puts("登陆失败，密码输错!");
	}
	else
	{
		local_acc.islock = 0;
		sprintf(buf,"Y:登陆成功");
		puts("登陆成功");
	}
	fd = open(path,O_RDWR|O_TRUNC,0644);
	write(fd,&local_acc,sizeof(Acc));
	close(fd);
}

//销户
void destory(Acc* acc,char* buf)
{
	printf("%s\n",__func__);
	
	char path[256] = {};
	sprintf(path,"%s%s",ACC_PATH,acc->bank);
	// 判断银行卡号是否正确
	if(0 != access(path,F_OK))
	{
		sprintf(buf,"N:卡号不存在，请检查!");
		return;
	}
	
	int fd = open(path,O_RDWR);
	if(0 > fd)
	{
		perror("open");
		sprintf(buf,"N:服务器正在升级，销户失败");
		return;
	}
	
	Acc local_acc = {};
	read(fd,&local_acc,sizeof(Acc));
	show_acc(&local_acc);
	close(fd);
	
	if(strcmp(acc->card,local_acc.card))
	{
		sprintf(buf,"N:身份证号不正确，销户失败!");
		return;
	}
	
	if(strcmp(acc->pass,local_acc.pass))
	{
		sprintf(buf,"N:密码不正确，销户失败!");
		return;
	}
	
	if(remove(path))
	{
		perror("remove");
		sprintf(buf,"N:服务器正在升级，销户失败!");
		return;
	}
	
	sprintf(buf,"Y:销户成功，期待下次与你相遇!");
}

//解锁
void unlock(Acc* acc,char* buf)
{
	printf("%s\n",__func__);
	
	char path[256] = {};
	sprintf(path,"%s%s",ACC_PATH,acc->bank);
	// 判断银行卡号是否正确
	if(0 != access(path,F_OK))
	{
		sprintf(buf,"N:卡号不存在，请检查!");
		return;
	}
	
	int fd = open(path,O_RDWR);
	if(0 > fd)
	{
		perror("open");
		sprintf(buf,"N:服务器正在升级，解锁失败");
		return;
	}
	
	Acc local_acc = {};
	read(fd,&local_acc,sizeof(Acc));
	show_acc(&local_acc);
	close(fd);
	
	if(strcmp(acc->card,local_acc.card))
	{
		sprintf(buf,"N:身份证号不正确，解锁失败!");
		return;
	}
	
	if(strcmp(acc->pass,local_acc.pass))
	{
		sprintf(buf,"N:密码不正确，解锁失败!");
		return;
	}
	
	local_acc.islock = 0;
	//lseek(fd,0,SEEK_SET);
	fd = open(path,O_RDWR|O_TRUNC,0644);
	write(fd,&local_acc,sizeof(Acc));
	close(fd);
	
	sprintf(buf,"Y:解锁成功!");
}

//存钱
void save(Acc* acc,char* buf)
{
	printf("%s\n",__func__);
	
	char path[256] = {};
	sprintf(path,"%s%s",ACC_PATH,acc->bank);
	
	int fd = open(path,O_RDWR);
	if(0 > fd)
	{
		perror("open");
		sprintf(buf,"N:服务器正在升级，存款失败!");
		return;
	}
	
	Acc local_acc = {};
	read(fd,&local_acc,sizeof(Acc));

	local_acc.balance += acc->balance;
	//lseek(fd,0,SEEK_SET);
	fd = open(path,O_RDWR|O_TRUNC,0644);
	write(fd,&local_acc,sizeof(Acc));
	close(fd);
	
	sprintf(buf,"Y:存款成功，当前余额为:%.2f!",local_acc.balance);
}

//取钱
void take(Acc* acc,char* buf)
{
	printf("%s\n",__func__);
	
	char path[256] = {};
	sprintf(path,"%s%s",ACC_PATH,acc->bank);
	
	int fd = open(path,O_RDWR);
	if(0 > fd)
	{
		perror("open");
		sprintf(buf,"N:服务器正在升级，取款失败!");
		return;
	}
	
	Acc local_acc = {};
	read(fd,&local_acc,sizeof(Acc));

	if(local_acc.balance < acc->balance)
	{
		sprintf(buf,"N:余额不足，取款失败。当前余额为:%.2f!",local_acc.balance);
		close(fd);
		return;
	}
	
	local_acc.balance -= acc->balance;
	//lseek(fd,0,SEEK_SET);
	fd = open(path,O_RDWR|O_TRUNC,0644);
	write(fd,&local_acc,sizeof(Acc));
	close(fd);
	
	sprintf(buf,"Y:取款成功，当前余额为:%.2f!",local_acc.balance);
}

//查询
void query(Acc* acc,char* buf)
{
	printf("%s\n",__func__);
	
	char path[256] = {};
	sprintf(path,"%s%s",ACC_PATH,acc->bank);
	
	int fd = open(path,O_RDWR);
	if(0 > fd)
	{
		perror("open");
		sprintf(buf,"N:服务器正在升级，查询失败!");
		return;
	}
	
	Acc local_acc = {};
	read(fd,&local_acc,sizeof(Acc));
	close(fd);
	
	sprintf(buf,"Y:查询成功，当前余额为:%.2f!",local_acc.balance);
}

//转帐
void transfer(Acc* acc,char* buf)
{
	printf("%s\n",__func__);
	
	char src_path[256] = {} , dest_path[256];
	
	sprintf(src_path,"%s%s",ACC_PATH,acc->bank);
	sprintf(dest_path,"%s%s",ACC_PATH,acc->card);
	
	// 判断银行卡号是否正确
	if(0 != access(dest_path,F_OK))
	{
		sprintf(buf,"N:目标卡号不存在，请检查!");
		return;
	}
	
	int src_fd = open(src_path,O_RDWR);
	int dest_fd = open(dest_path,O_RDWR);
	if(0 > src_fd || 0 > dest_fd)
	{
		error("open");
		sprintf(buf,"N:服务器正在升级，转账失败");
		return;
	}
	
	Acc src_acc = {} , dest_acc = {};
	read(src_fd,&src_acc,sizeof(Acc));
	read(dest_fd,&dest_acc,sizeof(Acc));

	if(src_acc.balance <  acc->balance)
	{
		sprintf(buf,"N:余额不足，转账失败。当前余额为:%.2f!",src_acc.balance);
		return;
	}

	src_acc.balance -= acc->balance;
	dest_acc.balance += acc->balance;
	
	
	lseek(src_fd,0,SEEK_SET);
	lseek(dest_fd,0,SEEK_SET);
	write(src_fd,&src_acc,sizeof(Acc));
	write(dest_fd,&dest_acc,sizeof(Acc));
	close(src_fd);
	close(dest_fd);
	
	sprintf(buf,"Y:转账成功，当前余额为:%.2f!",src_acc.balance);
}

//修改密码
void repass(Acc* acc,char* buf)
{
	printf("%s\n",__func__);
	
	char path[256] = {};
	sprintf(path,"%s%s",ACC_PATH,acc->bank);
	
	int fd = open(path,O_WRONLY);
	if(0 > fd)
	{
		error("open");
		sprintf(buf,"N:服务器正在升级，查询失败!");
		return;
	}
	
	Acc local_acc = {};
	read(fd,&local_acc,sizeof(Acc));
	strcpy(local_acc.pass,acc->pass);
	write(fd,&local_acc,sizeof(Acc));
	
	sprintf(buf,"Y:修改密码成功!");
}

//客户端线程
void* server(void* arg)
{
	int cli_fd = *(int*)arg;
	Acc* acc = malloc(sizeof(Acc));
	for(;;)
	{
		char buf[BUF_SIZE];
		int recv_size = read(cli_fd,buf,sizeof(buf));
		if(0 >= recv_size || 0 == strcmp(buf,"quit"))
		{
			printf("客户端%d退出!\n",cli_fd);
			free(acc);
			close(cli_fd);
			pthread_exit(NULL);
		}
		//解析字符串
		str_to_acc(acc,buf);
		//show_acc(acc);
		
		//根据字符串type操作
		switch(acc->type)
		{
			case M_OPEN: open_acc(acc,buf); break;		//开户
			case M_LOGIN: login(acc,buf); break;		//登陆
			case M_DESTORY: destory(acc,buf); break;	//销户
			case M_UNLOCK: unlock(acc,buf); break;		//解锁
			case M_SAVE: save(acc,buf); break;			//存钱
			case M_TAKE: take(acc,buf); break;			//取钱
			case M_QUERY: query(acc,buf); break;		//查询
			case M_TRANSFER: transfer(acc,buf); break;//转帐
			case M_REPASS: repass(acc,buf); break;		//改密码
		}
		write(cli_fd,buf,strlen(buf)+1);
	}
}

//服务器退出
void sigint(int signum)
{
	close(svr_fd);
	printf("服务器关闭\n");
	exit(0);
}

int main()
{
	signal(SIGINT,sigint);
	//创建socket对象
	svr_fd = socket(AF_INET,SOCK_STREAM,0);
	if(0 > svr_fd)
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

	//绑定socket对象与地址
	if(bind(svr_fd,(struct sockaddr*)&addr,addrlen))
	{
		perror("bind");
		return -1;
	}

	//设置监听
	if(listen(svr_fd,10))
	{
		perror("listen");
		return -1;
	}

	//等待链接
	for(;;)
	{
		int cli_fd = accept(svr_fd,(struct sockaddr*)&addr,&addrlen);

		if(0 > cli_fd)
		{
			printf("accept");
			return -1;
		}

		pthread_t tid;
		pthread_create(&tid,NULL,server,&cli_fd);
	}
}
