#include "inodehead.h"

#define SAME_DIR -5
#define REGULAR 1
#define DIRECTORY 2

using namespace std;


// Declerations
long long int f_s, b_s, n_b;
file_sys file;
vector<FD_t> FD;

int cur_dir;
int prev_dir;


/////////////////////////////////////////////////////////////////////////////////////////
void init(long long int a, long long int b, long long int c)
{
	::f_s = a;
	::b_s = b;
	::n_b = c;

	int i;
	//cout<<"Here......"<<endl;
	file.sp.file_sys_size = a;
	file.sp.b_s = b;
	file.sp.n_b = c;
	file.sp.n_inodes = 2 * (b/sizeof(inode));
	strcpy(file.sp.name, "INODE");
	file.sp.first_free_block = 3;
	
	file.b_inode.iNode = (inode *)malloc(file.sp.n_inodes * sizeof(inode));
	file.sp.free_inode=(bool*)malloc(file.sp.n_inodes * sizeof(bool));

	bool topush=true;
	
	for(i=0; i<file.sp.n_inodes; i++)
		file.sp.free_inode[i]=true;
	
	file.b = (data_block *)malloc((c-3)*sizeof(data_block));
	
	for(i=0; i<(c-3); i++)
	{
		file.b[i].next = i+4;
		file.b[i].data = (char *)malloc((b*1024)*sizeof(char));
		for(int j = 0; j<1024; j++)
			file.b[i].data[j] = '\0';
	}
	
	file.sp.free_inode[0] = false;

	int tp=-1;
	file.b_inode.iNode[0].dp.resize(5,tp);
	file.b_inode.iNode[0].dp[0] = SAME_DIR;
	file.b_inode.iNode[0].dp[1] = file.sp.first_free_block;
	file.sp.first_free_block = file.b[file.sp.first_free_block-3].next;
	file.b_inode.iNode[0].sip=file.b_inode.iNode[0].dip=-1;
	cur_dir = 0;
	prev_dir=-1;
}

/////////////////////////////////////////////////////////////////////////////////////////
int gpt(int inodeno, int x2, int* x3, int* x1, int* gpx3, int* x5, int* x6, int* dipblockoffset1, int* x7 )
{

	if(*x1+x2<b_s){
		*x1=*x1+x2;
		if(file.b[*(x3)-3].data[*x1]==NULL) return (-3);
		return (*gpx3);
	}
	else if(*x3==file.b_inode.iNode[inodeno].last_data_block)
	{
		return -1;
	}
	else if (*gpx3==0 && *x5<4)
	{
		*x5++;
		*x3=file.b_inode.iNode[inodeno].dp[*x5];
		*x1=0;
		return 0;
	}
	else if (*gpx3==0 && *x5==4)
	{
		*gpx3=1;
		*x6=0;
		int* ptr=(int*)file.b[file.b_inode.iNode[inodeno].sip-3].data;
		*x3=*ptr;
		*x1=0;
		return (*gpx3);
	}
	else if(*gpx3==1 && (*x6+sizeof(int))<b_s)
	{
		*x6+=sizeof(int);
		int* ptr=(int*)((char*)file.b[file.b_inode.iNode[inodeno].sip-3].data+sizeof(int));
		*x3=*ptr;
		*x1=0;
		return (*gpx3);
	}
	else if(*gpx3==1 && (*x6+sizeof(int))>=b_s)
	{
		*gpx3=2;
		*dipblockoffset1=0;
		*x7=0;
		int dipblockno=file.b_inode.iNode[inodeno].dip;
		int* tempptr=(int*)((char*)file.b[dipblockno-3].data+*dipblockoffset1);
		int actualsearchinblock=*tempptr;
		int* tempptr2=(int*)((char*)file.b[actualsearchinblock-3].data+*x7);
		*x3=*tempptr2;
		*x1=0;
		return (*gpx3);
	}
	else if(*gpx3==2 && (*x7+sizeof(int))<b_s)
	{
		*x7+=sizeof(int);
		int dipblockno=file.b_inode.iNode[inodeno].dip;
		int* tempptr=(int*)((char*)file.b[dipblockno-3].data+*dipblockoffset1);
		int actualsearchinblock=*tempptr;
		int* tempptr2=(int*)((char*)file.b[actualsearchinblock-3].data+*x7);
		*x3=*tempptr2;
		*x1=0;
		return (*gpx3);
	}
	else if(*gpx3==2 && (*x7+sizeof(int))>=b_s && (*dipblockoffset1+sizeof(int))<b_s)
	{
		*dipblockoffset1+=sizeof(int);
		*x7=0;
		int dipblockno=file.b_inode.iNode[inodeno].dip;
		int* tempptr=(int*)((char*)file.b[dipblockno-3].data+*dipblockoffset1);
		int actualsearchinblock=*tempptr;
		int* tempptr2=(int*)((char*)file.b[actualsearchinblock-3].data+*x7);
		*x3=*tempptr2;
		*x1=0;
		return (*gpx3);
	}
	else
	{
		return -2;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
int my_mkdir(char *str)
{
	if(!strcmp(str, ".") || !strcmp(str, ".."))
		return -1;

	int inodeno = cur_dir; //jobhi inode we are working on
	int x3;
	if(inodeno==0)
		x3 = file.b_inode.iNode[cur_dir].dp[1];  //initialize to dp[0] wala block no
	else
		x3 = file.b_inode.iNode[cur_dir].dp[2];

	int x1 = 0; //initially 0
	int gpx3 = 0; //initially 0
	int x5 = 0; //initially 0
	int x6 = 0; //initially 0
	int dipblockoffset1 = 0; //initially 0
	int x7 = 0; //initially 0
	// cout<<"Here......"<<endl;

	//search if same named directory/file exists, if no return -1
	int found=0;
	int rv;
	int x4;

	do
	{
		char filename[15];
		for(int i=0;i<14;i++){
			filename[i]=file.b[x3-3].data[x1+i];
		}
		filename[14]='\0';
		if(!strcmp(filename,str))
		{
			found=1;
			int x4=*(short int*)(file.b[x3-3].data+(int)(x1)+14);
			break;
		}
		rv=gpt(cur_dir, 16, &x3, &x1, &gpx3, &x5, &x6, &dipblockoffset1, &x7);
	}while(rv!=-1 && rv!=-3);
	//if that file is a dir, return -2
	//else add a new entry in this directory
	if (found==0)
	{
		if(x1+16<b_s)
		{
			for(int i=0;i<14;i++)
			{
				file.b[x3-3].data[x1+i]=str[i];
			}

		}
		short int freeinode = -1;
		for(int i=0; i<file.sp.n_inodes; i++)
		{
			if(file.sp.free_inode[i] == true)
			{
				freeinode = (short int)i;
				break;
			}
		}
		if(freeinode == -1)
			return -1;
		file.b_inode.iNode[freeinode].dp.resize(5,-1);
		file.b_inode.iNode[freeinode].dp[0]=SAME_DIR;
		file.b_inode.iNode[freeinode].dp[1]=cur_dir;
		file.b_inode.iNode[freeinode].dp[2]=file.sp.first_free_block;
		file.b_inode.iNode[freeinode].last_data_block=file.sp.first_free_block;
		file.sp.first_free_block=file.b[file.sp.first_free_block-3].next;
		file.sp.free_inode[freeinode]=false;
		char* ptr=file.b[x3-3].data+x1+14;

		sprintf(ptr,"%02hd",(short int)freeinode);
		for(int i=0; i<2; i++)
		{
			file.b[x3].data[x1+14+i]=ptr[i];
		}
	}
	else
	{
		cout<<"Same name file/directory already exists in the current directory"<<endl;
		return -1;
	}

}

/////////////////////////////////////////////////////////////////////////////////////////
int my_rmdir(char *str)
{
	if(!strcmp(str, ".") || !strcmp(str, ".."))
		return -1;

	int inodeno = cur_dir; //jobhi inode we are working on
	int x3;
	if(inodeno==0)
		x3 = file.b_inode.iNode[cur_dir].dp[1];  //initialize to dp[0] wala block no
	else
		x3 = file.b_inode.iNode[cur_dir].dp[2];
	// cout<<x3<<endl;
	int x1 = 0; //initially 0
	int gpx3 = 0; //initially 0
	int x5 = 0; //initially 0
	int x6 = 0; //initially 0
	int dipblockoffset1 = 0; //initially 0
	int x7 = 0; //initially 0
	// cout<<"Here......"<<endl;

	//search if same named directory/file exists, if no return -1
	int found=0;
	int rv;
	int x4;
	do
	{
		char filename[15];
		for(int i=0;i<14;i++){
			filename[i]=file.b[x3-3].data[x1+i];
		}
		filename[14]='\0';
		if(!strcmp(filename,str))
		{
			found=1;
			int x4=*(short int*)(file.b[x3-3].data+(int)(x1)+14);
			break;
		}
		rv=gpt(cur_dir, 16, &x3, &x1, &gpx3, &x5, &x6, &dipblockoffset1, &x7);
	}while(rv!=-1 && rv!=-3);
	//if that file is a dir, return -2
	int dirtodelete;
	//else add a new entry in this directory
	if (found==1)
	{
		char ptr[3];
		for(int i=0; i<2; i++)
		{
			ptr[i] = file.b[x3].data[x1+14+i];
		}

		ptr[2] = '\0';

		dirtodelete = atoi(ptr);
		for(int i=0;i<16;i++)
			file.b[x3-3].data[x1+i]=-2;
		int inodeno1 = dirtodelete; //jobhi inode we are working on
		int _x3;
		if (inodeno1==0)
			_x3 = file.b_inode.iNode[inodeno1].dp[1];  //initialize to dp[0] wala block no
		else
			_x3 = file.b_inode.iNode[inodeno1].dp[2];
		int _x1 = 0; //initially 0
		int _gpx3 = 0; //initially 0
		int _x5 = 0; //initially 0
		int _x6 = 0; //initially 0
		int _dipblockoffset1 = 0; //initially 0
		int _x7 = 0; //initially 0

		//search if same named file exists, if no return -1
		int rv;
		int x4;
		do
		{
			for(int i=0;i<16;i++)
				file.b[_x3-3].data[_x1+i]=-2;
			rv=gpt(inodeno1, 16, &_x3, &_x1, &_gpx3, &_x5, &_x6, &_dipblockoffset1, &_x7);
		}while(rv!=-1 && rv!=-3);

		return cur_dir;
	}
	else
	{
		cout<<"Same name file/directory does not exist in the current directory"<<endl;
		return -1;
	}

}

////////////////////////////////////////////////////////////////////////////////////////
int my_chdir(char *str)
{
	if(!strcmp(str, "."))
		return cur_dir;

	if(!strcmp(str, "..")){
		int a = cur_dir;
		cur_dir = (prev_dir == -1)?-1:prev_dir;
		if(cur_dir == -1){
			cur_dir = a;
			cout<<"This is home directory"<<endl;
			return cur_dir;
		}
		prev_dir = file.b_inode.iNode[cur_dir].dp[1];

	}
	int inodeno = cur_dir; //jobhi inode we are working on
	int x3;
	if(inodeno==0)
		x3 = file.b_inode.iNode[cur_dir].dp[1];  //initialize to dp[0] wala block no
	else
		x3 = file.b_inode.iNode[cur_dir].dp[2];
	// cout<<x3<<endl;
	int x1 = 0; //initially 0
	int gpx3 = 0; //initially 0
	int x5 = 0; //initially 0
	int x6 = 0; //initially 0
	int dipblockoffset1 = 0; //initially 0
	int x7 = 0; //initially 0

	//search if same named directory/file exists, if no return -1
	int found=0;
	int rv;
	int x4;

	do
	{
		char filename[15];
		for(int i=0;i<14;i++){
			filename[i]=file.b[x3-3].data[x1+i];
		}
		filename[14]='\0';
		if(!strcmp(filename,str))
		{
			found=1;
			int x4=*(short int*)(file.b[x3-3].data+(int)(x1)+14);
			break;
		}
		rv=gpt(cur_dir, 16, &x3, &x1, &gpx3, &x5, &x6, &dipblockoffset1, &x7);
	}while(rv!=-1 && rv!=-3);
	//if that file is a dir, return -2

	//else add a new entry in this directory
	if (found==1)
	{
		prev_dir=cur_dir;
		char ptr[3];
		for(int i=0; i<2; i++){
			ptr[i] = file.b[x3].data[x1+14+i];
		}
		ptr[2] = '\0';

		cur_dir = atoi(ptr);

		return cur_dir;
	}
	else
	{
		cout<<"Same name file/directory does not exist in the current directory"<<endl;
		return -1;
	}

}
