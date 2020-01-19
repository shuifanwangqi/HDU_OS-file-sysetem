#include <stdio.h>
#include <stdlib.h>
//#include <conio.h>
#include <string.h>
#define BLKSIZE    512		// ���ݿ�Ĵ�С
#define BLKNUM     512		// ���ݿ�Ŀ���
#define INODESIZE  32		// i�ڵ�Ĵ�С
#define INODENUM   32		// i�ڵ����Ŀ
#define FILENUM    8		// ���ļ������Ŀ

// �û�(20B)
typedef struct
{
	char user_name[10];	    // �û���
	char password[10];	    // ����
} User;

// i�ڵ�(32B)
typedef struct
{
	short inum;          // �ļ�i�ڵ��
	char  file_name[10]; // �ļ���
	char  type;          // �ļ�����  ��-�� �ļ� ��d���ļ���
	char  user_name[10]; // �ļ�������
	short iparent;       // ��Ŀ¼��i�ڵ��
	short length;        // �ļ�����
	//short address[2];    // ����ļ��ĵ�ַ
	short start;//����ļ�����ʼ��ַ
	short blk_num;//���ݿ���
} Inode;

// ���ļ���(16B)
typedef struct
{
	short inum;	         // i�ڵ��
	char  file_name[10]; // �ļ���
	short mode;	         // ��дģʽ(1:read, 2:write,
	//         3:read and write)
	short offset;        // ƫ����
} File_table;

// ��������
void login(void);
void init(void);
int  analyse(char *);
void save_inode(int);
int	get_blknum(int);
void read_blk(int);
void write_blk(int,int,int);
void release_blk(int);
void pathset();
void delet(int innum);
int  check(int i);
// �û��������
void help(void);
void cd(void);
void dir(void);
void mkdir(void);
void creat(void);
void open(void);
void read(void);
void write(void);
void close(void);
void del(void);
void logout(void);
void command(void);
void rd();
void quit();
char getch();
char getche();
