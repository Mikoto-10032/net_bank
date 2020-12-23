#ifndef BANK_H
#define BANK_H

// 操作类型
#define M_EXIT 			0 //退出
#define M_OPEN			1 //开户
#define M_LOGIN			2 //登陆
#define M_DESTORY		3 //销户
#define M_UNLOCK		4 //解锁
#define M_SAVE			5 //存款
#define M_TAKE			6 //取款
#define M_QUERY			7 //查询
#define M_TRANSFER		8 //转帐
#define M_REPASS		9 //改密

// 账户信息存储路径
#define ACC_PATH	"account/"

//账户信息结构体
typedef struct Account
{
	int type;		//操作类型
	char bank[20];	//银行卡号
	char card[20];	//身份证号
	char pass[20];	//密码
	float balance;	//余额
	int islock;		//锁定
}Acc;

#endif//BANK_H
