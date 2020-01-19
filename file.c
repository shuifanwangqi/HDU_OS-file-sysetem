#include "head.h"
char	choice;
int		argc;		// �û�����Ĳ�������
char	*argv[5];		// �û�����Ĳ���
int		inum_cur;		// ��ǰĿ¼
char	temp[2 * BLKSIZE];	// ������
User	user;		// ��ǰ���û�
char	bitmap[BLKNUM];	// λͼ����
Inode	inode_array[INODENUM];	// i�ڵ�����
File_table file_array[FILENUM];	// ���ļ�������
char	image_name[10] = "hd.dat";	// �ļ�ϵͳ����
FILE	*fp;					// ���ļ�ָ��

//����ӳ��hd�����������û����ļ����
char getch()
{
char c;
system("stty -echo");  //������
system("stty -icanon");//����һ���Զ����������ʹ��getchar()������������Ҫ��enter
c=getchar();
system("stty icanon");//ȡ�����������
system("stty echo");//����
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
		// ���ļ�user.txt
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
// ����: �û���½����������û��򴴽��û�
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
			if (*p == '\n') //������س���ʱ��0x0dΪ�س�����ASCII��
			{
				*p = '\0'; //������Ļس���ת���ɿո�
				break;
			}
			printf("*");   //�������������"*"����ʾ
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
			// �Ѿ����ڵ��û�, ��������ȷ
			if (!strcmp(user.user_name, user_name) &&
				!strcmp(user.password, password))
			{
				fclose(fp);
				printf("\n");
				return;
			}
			// �Ѿ����ڵ��û�, ���������
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
	// �������û�
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
// ����: ������i�ڵ�����ڴ�
void init(void)
{
	int   i;
	if ((fp = fopen(image_name, "r+b")) == NULL)
	{
		printf("Can't open file %s.\n", image_name);
		exit(-1);
	}
	// ����λͼ
	for (i = 0; i < BLKNUM; i++)
		bitmap[i] = fgetc(fp);
	// ��ʾλͼ
	// ����i�ڵ���Ϣ
	for (i = 0; i < INODENUM; i++)
		fread(&inode_array[i], sizeof(Inode), 1, fp);
	// ��ʾi�ڵ�
	// ��ǰĿ¼Ϊ��Ŀ¼
	inum_cur = 0;
	// ��ʼ�����ļ���
	for (i = 0; i < FILENUM; i++)
		file_array[i].inum = -1;
}
// ����: �����û�����, ������������argc��argv
// ���: 0-14Ϊϵͳ����, 15Ϊ�������
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
// ����: ��num��i�ڵ㱣�浽hd.dat
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
// ����: ����һ�����ݿ�
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
//z׷�Ӵ��̿�

// ����: ��i�ڵ��Ϊnum���ļ�����temp
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
// ����: ��temp����������hd��������
void write_blk(int num,int s_start,int length)//length ��������ݳ���
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
// ����: �ͷ��ļ����Ϊnum���ļ�ռ�õĿռ�
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
// ����: ��ʾ��������
void help(void)
{
	printf("16281294***����: \n\
		   help   ---  ��ʾ�����˵� \n\
		   clear  ---  �����Ļ \n\
		   cd     ---  ����Ŀ¼  \n\
		   mkdir  ---  ����Ŀ¼   \n\
		   creat  ---  �������ļ� \n\
		   open   ---  �������ļ�  \n\
		   read   ---  ���ļ� \n\
		   write  ---  д�ļ� \n\
		   close  ---  �ر��ļ� \n\
		   delete ---  ɾ�������ļ� \n\
		   format ---  ��ʽ�������ļ�ϵͳ  \n\
		   logout ---  �˳��û� \n\
		   rd     ---  ɾ��Ŀ¼  \n\
           dir    ---  ��ʾ�ļ�Ŀ¼\n\
		   quit   ---  �˳�ϵͳ\n");
}
//�����ļ�·��
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
// ����: �л�Ŀ¼(cd .. ���� cd dir1)
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
		// ����i�ڵ�����
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
// ����: ��ʾ��ǰĿ¼�µ���Ŀ¼���ļ�(dir)
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
	// ����i�ڵ�����, ��ʾ��ǰĿ¼�µ���Ŀ¼���ļ���
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
// ����: ɾ��Ŀ¼��(rd dir1)
void rd()
{
	int i, j, t, flag = 0;
	if (argc != 2)
	{
		printf("Command delete must have one args. \n");
		return;
	}
	for (i = 0; i < INODENUM; i++)//���Ҵ�ɾ��Ŀ¼
	if ((inode_array[i].inum > 0) &&//�Ƿ�Ϊ��
		(inode_array[i].iparent == inum_cur) &&
		(inode_array[i].type == 'd') &&
		(!strcmp(inode_array[i].file_name, argv[1])))
	{
		int chk;
		chk = check(i);//����û�Ȩ��
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
				delet(t);//Ŀ¼�����ļ���ɾ��
			else if ((inode_array[t].inum>0) &&
				(inode_array[t].iparent == j) &&
				(inode_array[i].type == 'd'))
				delet(t);//Ŀ¼���п�Ŀ¼��ɾ��
		}
		if (t == INODENUM)
			delet(j);//�²�Ŀ¼Ϊ��ɾ��֮
	}
	if (i == INODENUM)
		delet(i);//��ɾ��Ŀ¼Ϊ��ɾ��֮
	return;
}
// ����: �ڵ�ǰĿ¼�´�����Ŀ¼(mkdir dir1)
void mkdir(void)
{
	int i;
	if (argc != 2)
	{
		printf("command mkdir must have two args. \n");
		return;
	}
	// ����i�ڵ�����, ����δ�õ�i�ڵ�
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
// ����: �ڵ�ǰĿ¼�´����ļ�(creat file1)
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
// ����: �򿪵�ǰĿ¼�µ��ļ�(open file1)
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
// ����: ���ļ��ж����ַ�(read file1)
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
// ����: ���ļ���д���ַ�(write file1)
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
// ����: �ر��Ѿ��򿪵��ļ�(close file1)
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
//ɾ��Ŀ¼��
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
// ����: ɾ���ļ�(delete file1)
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
// ����: �˳���ǰ�û�(logout)
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
//��鵱ǰI�ڵ���ļ��Ƿ����ڵ�ǰ�û�
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
// ����: �˳��ļ�ϵͳ(quit)
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
// ����: ��ʾ����
void errcmd()
{
	printf("Command Error!!!\n");
}
//����ڴ��д��ڵ��û���
void free_user()
{
	int i;
	for (i = 0; i<10; i++)
		user.user_name[i] = '\0';
}
// ����: ѭ��ִ���û����������, ֱ��logout
// "help", "cd", "dir", "mkdir", "creat", "open","read", "write", "close", "delete", "logout", "clear", "format","quit","rd"
void command(void)
{
	char cmd[100];
	system("clear");//����ϵͳ����cls�������������
	do
	{
		pathset();
		//gets(cmd);
		//scanf("%[^\n]",cmd);
		fgets(cmd,20,stdin);
		switch (analyse(cmd))
		{
		case 0:
			help();// ����: ��ʾ��������
			break;
		case 1:
			cd();// ����: �л�Ŀ¼(cd .. ���� cd dir1)
			break;
		case 2:
			dir();// ����: ��ʾ��ǰĿ¼�µ���Ŀ¼���ļ�(dir)
			break;
		case 3:
			mkdir();// ����: �ڵ�ǰĿ¼�´�����Ŀ¼(mkdir dir1)
			break;
		case 4:
			creat();// ����: �ڵ�ǰĿ¼�´����ļ�(creat file1)
			break;
		case 5:
			open();// ����: �򿪵�ǰĿ¼�µ��ļ�(open file1)
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
			del();// ����: ɾ���ļ�(delete file1)
			break;
		case 10:
			logout();
			break;
		case 11:
			system("clear");//����
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
			rd();// ����: ɾ��Ŀ¼��(rd dir1)
			break;
		case 15:
			errcmd();
			break;
		default:
			break;
		}
	} while (1);
}
// ������
int main(void)
{

	login();
	init();
	command();

	return 0;
}
