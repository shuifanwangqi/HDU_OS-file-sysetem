#include "head.h"
char	choice;
int		argc;		// 用户命令的参数个数
char	*argv[5];		// 用户命令的参数
int		inum_cur;		// 当前目录
char	temp[2 * BLKSIZE];	// 缓冲区
User	user;		// 当前的用户
char	bitmap[BLKNUM];	// 位图数组
Inode	inode_array[INODENUM];	// i节点数组
File_table file_array[FILENUM];	// 打开文件表数组
char	image_name[10] = "hd.dat";	// 文件系统名称
FILE	*fp;					// 打开文件指针

//创建映像hd，并将所有用户和文件清除
char getch()
{
char c;
system("stty -echo");  //不回显
system("stty -icanon");//设置一次性读完操作，如使用getchar()读操作，不需要按enter
c=getchar();
system("stty icanon");//取消上面的设置
system("stty echo");//回显
return c;
}

char getche()
{
char c;
system("stty -icanon");
c=getchar();
system("stty icanon");
return c;
}
void format(void)
{
	FILE  *fp;
	int   i;
	Inode inode;
	printf("Will be to format filesystem...\n");
	printf("WARNING:ALL DATA ON THIS FILESYSTEM WILL BE LOST!\n");
	printf("Proceed with Format(Y/N)?");
	scanf("%c", &choice);
	//gets(temp);
	//scanf("%[^\n]",temp);
	fgets(temp,20,stdin);
	if ((choice == 'y') || (choice == 'Y'))
	{
		if ((fp = fopen(image_name, "w+b")) == NULL)
		{
			printf("Can't create file %s\n", image_name);
			exit(-1);
		}
		for (i = 0; i < BLKSIZE; i++)
			fputc('0', fp);
		inode.inum = 0;
		strcpy(inode.file_name, "/");
		inode.type = 'd';
		strcpy(inode.user_name, "all");
		inode.iparent = 0;
		inode.length = 0;
		inode.start = -1;
		inode.blk_num = -1;
		fwrite(&inode, sizeof(Inode), 1, fp);
		inode.inum = -1;
		for (i = 0; i < 31; i++)
			fwrite(&inode, sizeof(Inode), 1, fp);
		for (i = 0; i < BLKNUM*BLKSIZE; i++)
			fputc('\0', fp);
		fclose(fp);
		// 打开文件user.txt
		if ((fp = fopen("user.txt", "w+")) == NULL)
		{
			printf("Can't create file %s\n", "user.txt");
			exit(-1);
		}
		fclose(fp);
		printf("Filesystem created successful.Please first login!\n");
	}
	return;
}
// 功能: 用户登陆，如果是新用户则创建用户
void login(void)
{
	char *p;
	int  flag;
	char user_name[10];
	char password[10];
	char file_name[10] = "user.txt";
	do
	{
		printf("login:");
		//gets(user_name);
		//scanf("%[^/n]",user_name);
		fgets(user_name,20,stdin);



       int len ;
        len = strlen(user_name);
        user_name[len-1]='\0';

		printf("password:");
		p = password;

		while (*p = getch())
		{
			if (*p == '\n') //当输入回车键时，0x0d为回车键的ASCII码
			{
				*p = '\0'; //将输入的回车键转换成空格
				break;
			}
			printf("*");   //将输入的密码以"*"号显示
			p++;
		}

		*p='\0';
		flag = 0;
		if ((fp = fopen(file_name, "r+")) == NULL)
		{
			printf("\nCan't open file %s.\n", file_name);
			printf("This filesystem not exist, it will be create!\n");
			format();
			login();
		}
		while (!feof(fp))
		{
			fread(&user, sizeof(User), 1, fp);
			// 已经存在的用户, 且密码正确
			if (!strcmp(user.user_name, user_name) &&
				!strcmp(user.password, password))
			{
				fclose(fp);
				printf("\n");
				return;
			}
			// 已经存在的用户, 但密码错误
			else if (!strcmp(user.user_name, user_name))
			{
				printf("\nThis user is exist, but password is incorrect.\n");
				flag = 1;
				fclose(fp);
				break;
			}
		}
		if (flag == 0) break;
	} while (flag);
	// 创建新用户
	if (flag == 0)
	{
		printf("\nDo you want to creat a new user?(y/n):");
		scanf("%c", &choice);
		//gets(temp);
		//scanf("%[^\n]",temp);
		fgets(temp,20,stdin);
		if ((choice == 'y') || (choice == 'Y'))
		{
			strcpy(user.user_name, user_name);
			strcpy(user.password, password);
			fwrite(&user, sizeof(User), 1, fp);
			fclose(fp);
			return;
		}
		if ((choice == 'n') || (choice == 'N'))
			login();
	}
}
// 功能: 将所有i节点读入内存
void init(void)
{
	int   i;
	if ((fp = fopen(image_name, "r+b")) == NULL)
	{
		printf("Can't open file %s.\n", image_name);
		exit(-1);
	}
	// 读入位图
	for (i = 0; i < BLKNUM; i++)
		bitmap[i] = fgetc(fp);
	// 显示位图
	// 读入i节点信息
	for (i = 0; i < INODENUM; i++)
		fread(&inode_array[i], sizeof(Inode), 1, fp);
	// 显示i节点
	// 当前目录为根目录
	inum_cur = 0;
	// 初始化打开文件表
	for (i = 0; i < FILENUM; i++)
		file_array[i].inum = -1;
}
// 功能: 分析用户命令, 将分析结果填充argc和argv
// 结果: 0-14为系统命令, 15为命令错误
int analyse(char *str)
{
	int  i;
	char temp[20];
	char *ptr_char;
	char  *syscmd[] = { "help", "cd", "ls", "mkdir", "creat", "open", "read", "write", "close", "delete", "logout", "clear", "format", "quit", "rd" };
	//printf("%s\n",syscmd[1]);
       // printf("%s",str);
	argc = 0;
	for (i = 0, ptr_char = str; *ptr_char != '\n'; ptr_char++)
	{
		if (*ptr_char != ' ')
		{
			while (*ptr_char != ' ' && (*ptr_char != '\n'))
				temp[i++] = *ptr_char++;
			argv[argc] = (char *)malloc(i + 1);
			strncpy(argv[argc], temp, i);
			argv[argc][i] = '\0';
			argc++;
			i = 0;
			if (*ptr_char == '\n') break;
		}
	}
//	printf("%d\n",argc);
//	printf("%s\n",argv[0]);
	if (argc != 0)
	{
		for (i = 0; (i < 15) && strcmp(argv[0], syscmd[i]); i++);
		return i;
	}
	else return 15;
}
// 功能: 将num号i节点保存到hd.dat
void save_inode(int num)
{
	if ((fp = fopen(image_name, "r+b")) == NULL)
	{
		printf("Can't open file %s\n", image_name);
		exit(-1);
	}
	fseek(fp, 512 + num*sizeof(Inode), SEEK_SET);
	fwrite(&inode_array[num], sizeof(Inode), 1, fp);
	fclose(fp);
}
// 功能: 申请一个数据块
int get_blknum(int num)
{
	int i,j;
	int flag;
	for (i = 0; i < BLKNUM; i++){
	   if (bitmap[i] == '0') {
          flag=i;
          for( j=0;bitmap[i]=='0'&&j<num;j++,i++);
          break;
	   }
	}
	if(j<num){
        printf("can't save so many area");
        exit(-1);
	}
	if ((fp = fopen(image_name, "r+b")) == NULL)
	{
		printf("Can't open file %s\n", image_name);
		exit(-1);
	}
    for( i=flag,j=0;j<num;j++)
        bitmap[i]='1';
        fseek(fp, i, SEEK_SET);
	    fputc('1', fp);
	fclose(fp);
	return flag;
}
//z追加磁盘块

// 功能: 将i节点号为num的文件读入temp
void read_blk(int num)
{
	int  i, len;
	char ch;
	//int  add0, add1;
	int start,blk_num;
	len = inode_array[num].length;
	start=inode_array[num].start;
	blk_num=inode_array[num].blk_num;

	if ((fp = fopen(image_name, "r+b")) == NULL)
	{
		printf("Can't open file %s.\n", image_name);
		exit(-1);
	}
	fseek(fp,1536+start*BLKSIZE,SEEK_SET);
	ch=fgetc(fp);
	for(i=0;(i<len)&&(ch!='\0');i++){
        temp[i]=ch;
	//printf("%d %c\n",i,ch);

        ch=fgetc(fp);
       // printf("%d %c\n",i,ch);
	}
	temp[i] = '\0';
	fclose(fp);
}
// 功能: 将temp的内容输入hd的数据区
void write_blk(int num,int s_start,int length)//length 读入的数据长度
{
	int  i, len,j;
	char ch;
	//int  add0, add1;
	//add0 = inode_array[num].address[0];
	int start=inode_array[num].start;
	len = inode_array[num].length-length;
//	printf("append length %d",len);
	if ((fp = fopen(image_name, "r+b")) == NULL)
	{
		printf("Can't open file %s.\n", image_name);
		exit(-1);
	}

	int position=s_start;
	for( i=len-1,j=1;i>=position;i--,j++){
        fseek(fp,1536+i+start*BLKSIZE,SEEK_SET);
//	printf("%d  ch %c",i,ch);
         ch=fgetc(fp);
	if(ch!='\0'){
  //     printf("jj %d\n",inode_array[num].length-j);
        fseek(fp,1536+inode_array[num].length-j+start*BLKSIZE,SEEK_SET);
        fputc(ch,fp);
        	}
	}
//	fputc('\0',fp);
         fseek(fp,1536+inode_array[num].length+start*BLKSIZE,SEEK_SET);
        fputc('\0',fp);
	fseek(fp,position+start*BLKSIZE+1536,SEEK_SET);
	for( j=0;j<length&&(temp[j]!='\0');j++){
      //   fseek(fp,i+start*BLKSIZE+1536,SEEK_SET);
         fputc(temp[j],fp);
	}
//	fputc('\0', fp);
	fclose(fp);
}
// 功能: 释放文件块号为num的文件占用的空间
void release_blk(int num)
{
	FILE *fp;
	if ((fp = fopen(image_name, "r+b")) == NULL)
	{
		printf("Can't open file %s\n", image_name);
		exit(-1);
	}
	bitmap[num] = '0';
	fseek(fp, num, SEEK_SET);
	fputc('0', fp);
	fclose(fp);
}
// 功能: 显示帮助命令
void help(void)
{
	printf("16281294***命令: \n\
		   help   ---  显示帮助菜单 \n\
		   clear  ---  清空屏幕 \n\
		   cd     ---  更改目录  \n\
		   mkdir  ---  生成目录   \n\
		   creat  ---  创建新文件 \n\
		   open   ---  打开现有文件  \n\
		   read   ---  读文件 \n\
		   write  ---  写文件 \n\
		   close  ---  关闭文件 \n\
		   delete ---  删除现有文件 \n\
		   format ---  格式化现有文件系统  \n\
		   logout ---  退出用户 \n\
		   rd     ---  删除目录  \n\
           dir    ---  显示文件目录\n\
		   quit   ---  退出系统\n");
}
//设置文件路径
void pathset()
{
	char path[50];
	int m, n;
	if (inode_array[inum_cur].inum == 0)
		strcpy(path, user.user_name);
	else
	{
		strcpy(path, user.user_name);
		m = 0;
		n = inum_cur;
		while (m != inum_cur)
		{
			while (inode_array[n].iparent != m)
			{
				n = inode_array[n].iparent;
			}
			strcat(path, "/");
			strcat(path, inode_array[n].file_name);
			m = n;
			n = inum_cur;
		}
	}
	printf("[%s]@", path);
}
// 功能: 切换目录(cd .. 或者 cd dir1)
void cd(void)
{
	int i;
	if (argc != 2)
	{
		printf("Command cd must have two args. \n");
		return;
	}
	if (!strcmp(argv[1], ".."))
		inum_cur = inode_array[inum_cur].iparent;
	else
	{
		// 遍历i节点数组
		for (i = 0; i < INODENUM; i++)
		if ((inode_array[i].inum>0) &&
			(inode_array[i].type == 'd') &&
			(inode_array[i].iparent == inum_cur) &&
			!strcmp(inode_array[i].file_name, argv[1]) &&
			check(i))
			break; ;[[[l]
		if (i == INODENUM)
			printf("This directory isn't exsited.\n");
		else inum_cur = i;
	}
}
// 功能: 显示当前目录下的子目录和文件(dir)
void dir(void)
{
	int i;
	int dcount = 0, fcount = 0;
	short bcount = 0;
	if (argc != 1)
	{
		printf("Command dir must have one args. \n");
		return;
	}
	// 遍历i节点数组, 显示当前目录下的子目录和文件名
	for (i = 0; i < INODENUM; i++)
	if ((inode_array[i].inum> 0) &&
		(inode_array[i].iparent == inum_cur))
	{
		if (inode_array[i].type == 'd' && check(i))
		{
			dcount++;
			printf("%-20s<DIR>\n", inode_array[i].file_name);
		}
		if (inode_array[i].type == '-' && check(i))
		{
			fcount++;
			bcount += inode_array[i].length;
			printf("%-20s%12d bytes\n", inode_array[i].file_name, inode_array[i].length);
		}
	}
	//printf("\n                    %d file(s)%11d bytes\n", fcount, bcount);
	//printf("                    %d dir(s) %11d bytes FreeSpace\n", dcount, 1024 * 1024 - bcount);
}
// 功能: 删除目录树(rd dir1)
void rd()
{
	int i, j, t, flag = 0;
	if (argc != 2)
	{
		printf("Command delete must have one args. \n");
		return;
	}
	for (i = 0; i < INODENUM; i++)//查找待删除目录
	if ((inode_array[i].inum > 0) &&//是否为空
		(inode_array[i].iparent == inum_cur) &&
		(inode_array[i].type == 'd') &&
		(!strcmp(inode_array[i].file_name, argv[1])))
	{
		int chk;
		chk = check(i);//检查用户权限
		if (chk != 1)
		{
			printf("This directory is not your !\n");
			return;
		}
		else j = inode_array[i].inum;
		for (t = 0; t<INODENUM; t++)
		{
			if ((inode_array[t].inum>0) &&
				(inode_array[t].iparent == j) &&
				(inode_array[i].type == '-'))
				delet(t);//目录下有文件则删除
			else if ((inode_array[t].inum>0) &&
				(inode_array[t].iparent == j) &&
				(inode_array[i].type == 'd'))
				delet(t);//目录下有空目录则删除
		}
		if (t == INODENUM)
			delet(j);//下层目录为空删除之
	}
	if (i == INODENUM)
		delet(i);//待删除目录为空删除之
	return;
}
// 功能: 在当前目录下创建子目录(mkdir dir1)
void mkdir(void)
{
	int i;
	if (argc != 2)
	{
		printf("command mkdir must have two args. \n");
		return;
	}
	// 遍历i节点数组, 查找未用的i节点
	for (i = 0; i < INODENUM; i++)
	if (inode_array[i].inum < 0) break;
	if (i == INODENUM)
	{
		printf("Inode is full.\n");
		exit(-1);
	}
	inode_array[i].inum = i;
	strcpy(inode_array[i].file_name, argv[1]);
	inode_array[i].type = 'd';
	strcpy(inode_array[i].user_name, user.user_name);
	inode_array[i].iparent = inum_cur;
	inode_array[i].length = 0;
	save_inode(i);
}
// 功能: 在当前目录下创建文件(creat file1)
void creat(void)
{
	 int i;
    if (argc != 2)
    {
        printf("command creat must have one args. \n");
        return;
    }
    for (i = 0; i < INODENUM; i++)
      if( !strcmp(inode_array[i].file_name, argv[1])&&inode_array[i].iparent==inum_cur){
                 printf("This file is exsit.\n");
                  return  ;
               }



	for (i = 0; i < INODENUM; i++)
	if (inode_array[i].inum < 0) break;
	if (i == INODENUM)
	{
		printf("Inode is full.\n");
		exit(-1);
	}
	inode_array[i].inum = i;
	strcpy(inode_array[i].file_name, argv[1]);
	inode_array[i].type = '-';
	strcpy(inode_array[i].user_name, user.user_name);
	inode_array[i].iparent = inum_cur;
	inode_array[i].length = 0;
	save_inode(i);
}
// 功能: 打开当前目录下的文件(open file1)
void open()
{
	int i, inum, mode, filenum, chk;
	if (argc != 2)
	{
		printf("command open must have one args. \n");
		return;
	}
	for (i = 0; i < INODENUM; i++)
	if ((inode_array[i].inum > 0) &&
        (inode_array[i].type == '-') &&(inode_array[i].iparent ==inum_cur)&&
        !strcmp(inode_array[i].file_name, argv[1]))
		break;
	if (i == INODENUM)
	{
		printf("The file you want to open doesn't exsited.\n");
		return;
	}
	inum = i;
	chk = check(i);
	if (chk != 1)
	{
		printf("This file is not your !\n");
		return;
	}
	printf("Please input open mode:(1: read, 2: write, 3: read and write):");
	scanf("%d", &mode);
	//gets(temp);
	//scanf("%[^\n]",temp);
	fgets(temp,20,stdin);
	if ((mode < 1) || (mode > 3))
	{
		printf("Open mode is wrong.\n");
		return;
	}
	for (i = 0; i < FILENUM; i++)
	if (file_array[i].inum < 0) break;
	if (i == FILENUM)
	{
		printf("The file table is full, please close some file.\n");
		return;
	}
	filenum = i;
	file_array[filenum].inum = inum;
	strcpy(file_array[filenum].file_name, inode_array[inum].file_name);
	file_array[filenum].mode = mode;
	file_array[filenum].offset = 0;
	printf("Open file %s by ", file_array[filenum].file_name);
	if (mode == 1) printf("read only.\n");
	else if (mode == 2) printf("write only.\n");
	else printf("read and write.\n");
}
// 功能: 从文件中读出字符(read file1)
void read()
{
	int i, start, num, inum;
	if (argc != 2)
	{
		printf("command read must have one args. \n");
		return;
	}
	for (i = 0; i < FILENUM; i++)
    if(file_array[i].inum>0)
	if ((inode_array[file_array[i].inum].iparent==inum_cur)&&
        !strcmp(file_array[i].file_name, argv[1]))
		break;
	if (i == FILENUM)
	{
		printf("Open %s first.\n", argv[1]);
		return;
	}
	else if (file_array[i].mode == 2)
	{
		printf("Can't read %s.\n", argv[1]);
		return;
	}
	inum = file_array[i].inum;
	printf("The length of %s:%d.\n", argv[1], inode_array[inum].length);
	if (inode_array[inum].length > 0)
	{
		printf("The start position:");
		scanf("%d", &start);
		//gets(temp);
		//scanf("%[^\n]",temp);
		fgets(temp,20,stdin);
		if ((start<0) || (start >= inode_array[inum].length))
		{
			printf("Start position is wrong.\n");
			return;
		}
		printf("The bytes you want to read:");
		scanf("%d", &num);
		//gets(temp);
		//scanf("%[^\n]",temp);
		fgets(temp,1024,stdin);
		if (num <= 0)
		{
			printf("The num you want to read is wrong.\n");
			return;
		}
		read_blk(inum);
	//	printf("read %s\n",temp);
		for (i = 0; (i < num) && (temp[i] != '\0'); i++)
			printf("%c", temp[start + i]);
		printf("\n");
	}
}
// 功能: 向文件中写入字符(write file1)
void write()
{
	int i, inum, length,position;
	if (argc != 2)
	{
		printf("Command write must have one args. \n");
		return;
	}
	for (i = 0; i < FILENUM; i++)
        if((file_array[i].inum>0))
	if (   (inode_array[file_array[i].inum].iparent==inum_cur)&&
        !strcmp(file_array[i].file_name, argv[1])) break;
	if (i == FILENUM)
	{
		printf("Open %s first.\n", argv[1]);
		return;
	}
	else if (file_array[i].mode == 1)
	{
		printf("Can't write %s.\n", argv[1]);
		return;
	}
	inum = file_array[i].inum;
	printf("The length of %s:%d\n", inode_array[inum].file_name, inode_array[inum].length);
	    printf("The position you want to write(0-%d)",inode_array[inum].length);
	    scanf("%d",&position);
	    fgets(temp,20,stdin);
	    if(position<0||position>inode_array[inum].length) {
        printf("Input wrong\n");
	       return ;
	    }
		printf("The length you want to write(0-1024):");
		scanf("%d", &length);
		//gets(temp);
		//scanf("%[^\n]",temp);
		fgets(temp,20,stdin);
		if (length < 0||length>1024)
		{
			printf("Input wrong.\n");
			return;
		}
		//inode_array[inum].length = length+inode_array[inum].length;
        //inode_array[inum].blk_num=inode_array[inum].length/512+1;
		if(inode_array[inum].length==0){
        		inode_array[inum].length = length;
			inode_array[inum].blk_num=inode_array[inum].length/512+1;
        		inode_array[inum].start=get_blknum(inode_array[inum].blk_num);
			}
		else {
		     	inode_array[inum].length=length+inode_array[inum].length;
             		int remainder=inode_array[inum].length-inode_array[inum].blk_num*512;// judeg if add blocks
              		if(remainder<=0);

              		else{// add blocks
                	 int append_num=remainder/512+1;

                 	 if((fp=fopen(image_name,"r+b"))==NULL){
                    	printf("Can't open file %s\n",image_name);
                    	exit(0);
                 	 }
                 	 for( i=0;i<append_num;i++){
                    		if(bitmap[i+inode_array[inum].start+inode_array[inum].blk_num]=='0'){//exist blk_num blocks
                        		bitmap[i+inode_array[inum].start+inode_array[inum].blk_num]='1';
                       			fseek(fp,i+inode_array[inum].start+inode_array[inum].blk_num,SEEK_SET);
                       			fputc('1',fp);

                    		}else{printf("can not find free space");return;}
                  }
                   inode_array[inum].blk_num=append_num+inode_array[inum].blk_num;
                   fclose(fp);
              }
		}


                save_inode(inum);
		printf("Input the data(Enter to end):\n");
		//gets(temp);
		//scanf("%[^\n]",temp);
		fgets(temp,1024,stdin);
		write_blk(inum,position,length);
               // printf("write %s\n",temp);

}
// 功能: 关闭已经打开的文件(close file1)
void close(void)
{
	int i;
	if (argc != 2)
	{
		printf("Command close must have one args. \n");
		return;
	}
	for (i = 0; i < FILENUM; i++)
	if ((file_array[i].inum > 0) &&
		!strcmp(file_array[i].file_name, argv[1])&&inode_array[file_array[i].inum].iparent==inum_cur) break;
	if (i == FILENUM)
	{
		printf("This file doesn't be opened.\n");
		return;
	}
	else
	{
		file_array[i].inum = -1;
		printf("Close %s successful!\n", argv[1]);
	}
}
//删除目录树
void delet(int innum)
{

	inode_array[innum].inum = -1;
	inode_array[innum].blk_num=0;

	int i;
	for(i=inode_array[innum].start;i<inode_array[innum].blk_num;i++)
    {
        release_blk(i);
    }
	save_inode(innum);
}
// 功能: 删除文件(delete file1)
void del(void)
{
	int i, chk;
	if (argc != 2)
	{
		printf("Command delete must have one args. \n");
		return;
	}
	for (i = 0; i < INODENUM; i++)
	if ((inode_array[i].inum > 0) &&
		(inode_array[i].type == '-') &&
		!strcmp(inode_array[i].file_name, argv[1])&&inode_array[i].iparent==inum_cur) break;
	if (i == INODENUM)
	{
		printf("This file doesn't exist.\n");
		return;
	}
	chk = check(i);
	if (chk != 1)
	{
		printf("This file is not your !\n");
		return;
	}

	delet(i);
}
// 功能: 退出当前用户(logout)
void logout()
{
	char choice;
	printf("Do you want to exit this user(y/n)?");
	scanf("%c", &choice);
	//gets(temp);
	//scanf("%[^\n]",temp);
	fgets(temp,20,stdin);
	if ((choice == 'y') || (choice == 'Y'))
	{
		printf("\nCurrent user exited!\nPlease to login by other user!\n");
		login();
	}
	return;
}
//检查当前I节点的文件是否属于当前用户
int check(int i)
{
	int j;
	char *uuser, *fuser;
	uuser = user.user_name;
	fuser = inode_array[i].user_name;
	j = strcmp(fuser, uuser);
	if (j == 0)  return 1;
	else      return 0;
}
// 功能: 退出文件系统(quit)
void quit()
{
	char choice;
	printf("Do you want to exist(y/n):");
	scanf("%c", &choice);
	//gets(temp);
	//scanf("%[^\n]",temp);
	fgets(temp,20,stdin);
	if ((choice == 'y') || (choice == 'Y'))
		exit(0);
}
// 功能: 显示错误
void errcmd()
{
	printf("Command Error!!!\n");
}
//清空内存中存在的用户名
void free_user()
{
	int i;
	for (i = 0; i<10; i++)
		user.user_name[i] = '\0';
}
// 功能: 循环执行用户输入的命令, 直到logout
// "help", "cd", "dir", "mkdir", "creat", "open","read", "write", "close", "delete", "logout", "clear", "format","quit","rd"
void command(void)
{
	char cmd[100];
	system("clear");//调用系统命令cls完成清屏操作。
	do
	{
		pathset();
		//gets(cmd);
		//scanf("%[^\n]",cmd);
		fgets(cmd,20,stdin);
		switch (analyse(cmd))
		{
		case 0:
			help();// 功能: 显示帮助命令
			break;
		case 1:
			cd();// 功能: 切换目录(cd .. 或者 cd dir1)
			break;
		case 2:
			dir();// 功能: 显示当前目录下的子目录和文件(dir)
			break;
		case 3:
			mkdir();// 功能: 在当前目录下创建子目录(mkdir dir1)
			break;
		case 4:
			creat();// 功能: 在当前目录下创建文件(creat file1)
			break;
		case 5:
			open();// 功能: 打开当前目录下的文件(open file1)
			break;
		case 6:
			read();
			break;
		case 7:
			write();
			break;
		case 8:
			close();
			break;
		case 9:
			del();// 功能: 删除文件(delete file1)
			break;
		case 10:
			logout();
			break;
		case 11:
			system("clear");//清屏
			break;
		case 12:
			format();
			init();
			free_user();
			login();
			break;
		case 13:
			quit();
			break;
		case 14:
			rd();// 功能: 删除目录树(rd dir1)
			break;
		case 15:
			errcmd();
			break;
		default:
			break;
		}
	} while (1);
}
// 主函数
int main(void)
{

	login();
	init();
	command();

	return 0;
}
