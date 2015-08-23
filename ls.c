#include<fcntl.h>
#include<sys/stat.h>
#include<dirent.h>
#include<sys/syscall.h>
#include<unistd.h>
#include<string.h>
#include<stdio.h>
#define _GNU_SOURCE

#define handle_error(msg) \
               do { perror(msg); return(1); } while (0)

#define print(n,p)  write(1," ",1); write(1,("%s",n),getLen(n)); qq=getLen(n); for(pp=0;pp<p-qq;pp++) write(1," ",1);
#define pc(n) write(1,("%s",colors[n]),getLen(colors[n]));
struct linux_dirent {
	long           d_ino;
	off_t          d_off;
	unsigned short d_reclen;
	char           d_name[];
};

#define BUF_SIZE 1024


//GLOBALS
const int ds = 24*60*60;
const int lys = 366*24*60*60;
const int ys = 365*24*60*60;
long long int tzone = 0;
char rStr[1024];
int pp;
int qq;
int fl=0;
int fa=0;
int fh=0;
int y=1970;
int day=1;
int mnth=1;
int hh;
int min;
int yy=0;
int mm=0;
int dd=0;
int color=0;
int months[13]={0,31,28,31,30,31,30,31,31,30,31,30,31};
char* mon[13]={"","Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
char* colors[4] = {"\x1b[0m","\x1b[32m","\x1b[34m","\x1b[36m"};


int leapYear(long long int year)
{
	if(year%400==0 || (year%100!=0 && year%4==0))
		return 1;
	return 0;
}

void localtime()
{
	int fd;
	fd=open("/etc/localtime",O_RDONLY);
	if(fd)
	{
		char c;
		int rd,f1=0,f2=0;
		while(c!='\n') read(fd,&c,sizeof(c));
		while(rd=read(fd,&c,sizeof(c)))
		{
			if(!f1 && c=='-')
				f1++;
			else if(f1)
			{
				int tmp=0;
				if(c=='-')
					f2=1;
				if(c>='0'&&c<='9')
					tmp = tmp*10 + (c-'0');
				while(rd=read(fd,&c,sizeof(c)))
				{
					if(c>='0'&&c<='9')
						tmp = tmp*10 + (c-'0');
					else if(c==':')
					{
						tmp*=3600;
						tzone+=tmp;
						break;
					}
				}
				tmp=0;
				while(rd=read(fd,&c,sizeof(c)))
					if(c>='0'&&c<='9')
						tmp = tmp*10 + (c-'0');
				tmp*=60;
				tzone+=tmp;
			}
		}
		if(f2)
			tzone=-tzone;
	}
}

void convert(long long int sec)
{
	int i;
	sec+=tzone;
	while(sec>=ds)
	{
		if(!yy)
		{
			if(leapYear(y))
				if(sec>=lys)
					sec-=lys, y++;
				else
					yy=1;
			else
				if(sec>=ys)
					sec-=ys, y++;
				else
					yy=1;
			if(leapYear(y))
				months[2]=29;
			else
				months[2]=28;
		}
		else if(!mm)
			for(i=1;i<12;i++)
				if(sec>=months[i]*ds)
					mnth++, sec-=months[i]*ds;
				else
				{
					mm=1; break;
				}
		else if(!dd)
		{
			day+=sec/ds;
			sec%=ds;
			dd=1;
		}
	}
	hh=sec/(3600);
	sec%=3600;
	min=sec/60;
	sec%=60;
}


int itos(long long int a, char *s)
{
	int i=0,len;
	if(a==0)
	{
		s[0]='0';
		s[1]='\0';
		return 1;
	}
	while(a)
	{
		s[i++]=(a%10)+'0';
		a/=10;
	}
	s[i]='\0';
	len = i;
	for(i=0;i<len/2;i++)
		s[i]=s[i] + s[len-i-1] - (s[len-i-1]=s[i]);
	return len;
}

int getLen(char *s)
{
	int c=0;
	while(s[c++]!='\0');
	return c-1;
}

void mycat(char* a, char *b)
{
	int c=0,k=0;
	while(a[c++]!='\0');
	c--;
	while(b[k]!='\0')
		a[c++] = b[k++];
	a[c++]=b[k++];

}

void getId(int id, int f)
{
	char c,xx[1000],idLen=0;
	int fd;
	idLen = itos(id,xx);
	if(f==0) //open for Username
	{
		if((fd = open("/etc/passwd",O_RDONLY)) == -1)
			perror("open");
	}
	else if(f==1) //open for Group name
	{
		if((fd = open("/etc/group",O_RDONLY)) == -1)
			perror("open");
	}
	while(read(fd,&c,1))
	{
		int len=0,i=0,flag=1;
		while(c!=':')
		{
			rStr[len++]=c;
			read(fd,&c,1);
		}
		rStr[len]='\0';
		while(read(fd,&c,1) && c!=':');
		while(read(fd,&c,1) && c!=':')
			if(i<idLen && c!=xx[i++])
			{
				flag = 0;
				break;
			}
		if(i==idLen && flag)
			return;
		while(1)
		{
			int ret = read(fd, &c, 1);
			if(ret==0 || c=='\n') break;
		}
	}
}

int main(int argc, char* argv[])
{
	char tmp[1000];
	int fd, nread,i;
	char buf[BUF_SIZE];
	struct linux_dirent *d;
	int bpos;
	char d_type;
	int argLen=0;
	char* args[1024];
	localtime();
	for(i=1;i<argc;i++)
		if(argv[i][0]=='-')
		{
			int k=1;
			while(argv[i][k])
			{
				if(argv[i][k]=='a')
					fa=1;
				else if(argv[i][k]=='l')
					fl=1;
				else if(argv[i][k]=='h')
					fh=1;
				k++;
			}
		}
		else
			args[argLen++]=argv[i];
	if(argLen==0)
		args[argLen++]=".";
	for(i=0;i<argLen;i++)
	{
		write(1,args[i],getLen(args[i]));
		write(1,": \n",3);
		fd = open(args[i], O_RDONLY | O_DIRECTORY);
		if (fd == -1)
			handle_error("open");

		for ( ; ; ) {
			nread = syscall(SYS_getdents, fd, buf, BUF_SIZE);
			if (nread == -1)
				handle_error("getdents");
			if (nread == 0)
				break;
			for (bpos = 0; bpos < nread;) {
				struct stat sb;
				d = (struct linux_dirent *) (buf + bpos);
				d_type = *(buf + bpos + d->d_reclen - 1);
				char jj[10000],slash[2];
				slash[0]='/';
				slash[1]='\0';
				jj[0]='\0';
				mycat(jj,args[i]); if(jj[getLen(jj)-1]!='/') mycat(jj,slash); mycat(jj,d->d_name);
				if(!fa && !fl)
				{
					if(d->d_name[0]!='.')
					{
						lstat(jj,&sb);
						color = d_type == DT_LNK ? 3 : d_type == DT_DIR ? 2 : sb.st_mode & S_IXUSR ? 1 : 0;
						pc(color);
						write(1,("%s",d->d_name),getLen(d->d_name));
						pc(0);
						write(1,"\n",1);
					}
				}
				else if(fa && !fl)
				{
					lstat(jj,&sb);
					color = d_type == DT_LNK ? 3 : d_type == DT_DIR ? 2 : sb.st_mode & S_IXUSR ? 1 : 0;
					pc(color);
					write(1,("%s",d->d_name),getLen(d->d_name));
					pc(0);
					write(1,"\n",1);
				}
				else if(fa || fl)
				{
					if(!fa && d->d_name[0]=='.')
					{
						bpos += d->d_reclen;
						continue;
					}
					lstat(jj,&sb);

					if(DT_LNK == d_type) write(1,"l",1);
					else if(DT_DIR == d_type) write(1,"d",1);
					else if(DT_BLK == d_type) write(1,"b",1);
					else if(DT_SOCK == d_type) write(1,"s",1);
					else if(DT_CHR == d_type) write(1,"?",1);
					else write(1,"-",1);
					write( 1,("%s",(sb.st_mode & S_IRUSR) ? "r" : "-") ,1);
					write( 1,("%s",(sb.st_mode & S_IWUSR) ? "w" : "-") ,1);
					write( 1,("%s",(sb.st_mode & S_IXUSR) ? "x" : "-") ,1);
					write( 1,("%s",(sb.st_mode & S_IRGRP) ? "r" : "-") ,1);
					write( 1,("%s",(sb.st_mode & S_IWGRP) ? "w" : "-") ,1);
					write( 1,("%s",(sb.st_mode & S_IXGRP) ? "x" : "-") ,1);
					write( 1,("%s",(sb.st_mode & S_IROTH) ? "r" : "-") ,1);
					write( 1,("%s",(sb.st_mode & S_IWOTH) ? "w" : "-") ,1);
					write( 1,("%s",(sb.st_mode & S_IXOTH) ? "x" : "-") ,1);
					itos(sb.st_nlink,tmp);
					print(tmp,0);
					getId(sb.st_uid, 0);
					print(rStr,10);
					getId(sb.st_gid, 1);
					print(rStr,10);
					if(fh) 
					{
						int cnt=0;
						char kk[1000];
						kk[0]='\0';
						long long int t = sb.st_size,p=0;
						while(t/1024)
						{
							p=t%1024;
							t/=1024;
							cnt++;
						}
						itos(t,tmp);
						mycat(kk,tmp);
						tmp[0]='.';
						tmp[1]='\0';
						mycat(kk,tmp);
						itos(p,tmp);
						tmp[2]='\0';
						mycat(kk,tmp);
						if(cnt==1)
						{
							tmp[0]='K';
							tmp[1]='\0';
							mycat(kk,tmp);
						}
						else if(cnt==2)
						{
							tmp[0]='M';
							tmp[1]='b';
							tmp[2]='\0';
							mycat(kk,tmp);
						}
						else if(cnt>=3)
						{
							tmp[0]='G';
							tmp[1]='b';
							tmp[2]='\0';
							mycat(kk,tmp);
						}
						print(kk,11);
					}
					else
					{
						itos(sb.st_size,tmp);
						print(tmp,8);
					}
					convert(sb.st_mtime);
					print(mon[mnth],0);
					itos(day,tmp);
					print(tmp,0);
					itos(hh,tmp);
					print(tmp,2);
					write(1,":",1);
					itos(min,tmp);
					print(tmp,3);
					color = d_type == DT_LNK ? 3 : d_type == DT_DIR ? 2 : sb.st_mode & S_IXUSR ? 1 : 0;
					pc(color);
					print(d->d_name,0);
					pc(0);
					if(d_type == DT_LNK)
					{
						char slink[1000];
						readlink(jj,slink,sb.st_size+1);
						write(1," ->",3);
						print(slink,0);
					}
					write(1,"\n",1);
					day=1; y=1970; mnth=1;yy=0;mm=0;dd=0;color=0;
				}
				bpos += d->d_reclen;
			}
		}
	}
	return 0;
}
