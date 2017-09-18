#include <iostream> 
#include<fstream>
#include<string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include<vector>
#include<math.h>
using namespace std;
char* dispose(char*,int);
/**
server_and processes for and operation.
receiving the information from edge server, parse the data and send back the result.
**/
int main() 
{
	// record the information of the addresses that will be used
	struct in_addr addr;
	ulong l;
	l=inet_addr("127.0.0.1");
	memcpy(&addr,&l,4);
	struct sockaddr_in remote;
	struct sockaddr_in local;
	local.sin_family=AF_INET;
	local.sin_port=htons(22749);
	local.sin_addr=addr;
	remote.sin_family=AF_INET;
	remote.sin_port=htons(24749);
	remote.sin_addr=addr;
	memset(&(local.sin_zero), 0, 8);
	int count=0;
	
	
	//create new UDP socket;
	int sock=socket(PF_INET,SOCK_DGRAM,0);
	if(bind(sock,(struct sockaddr*)&local,sizeof(struct sockaddr))==-1)
	{
		cout<<"bad binding"<<endl;
		return 0;
	}
	cout<<"The Server AND is up and running using UDP on port 22749."<<endl;
	cout<<"The Server AND start receiving lines from the edge server for AND computation. The computation results are:"<<endl;
	
	//loop for data transfer between backend server and edge server
	while(1)
	{
		socklen_t clientSize=sizeof(struct sockaddr_in);
	    vector<char*> result_list;
	    char buffer[50];
	    char message[50];
	    char success[10]="@";
	    int len;
		
		//receving from the edge server one by one.
	    while(1)
		{
			int len=recvfrom(sock,buffer,50,0,(struct sockaddr *)&remote,&clientSize);
			if((len)==-1)  
			{	  
				cout<<"recv error"<<endl;   
				return 0;  
			}
			if(len>0)
			{
				
				//if get the end message, stop receiving and run to the process process
				if(buffer[0]=='@')
				{
					cout<<"The Server AND has successfully received ";
					cout<<count;
					cout<<" lines from the edge server and finished all AND computations"<<endl;
					count=0;
					break;
				}
		    char* res=dispose(buffer,len);	//do and operation for the row
		    result_list.push_back(res);	//push the result to list
		    count++;
			}
		
		}
		
		//send the result together to edge server.
	    for(int i=0;i<result_list.size();i++)
		{
			char* res=result_list[i];
			int length=0;
			while(res[length]!='$')
			{
				length++;
			}
			if(len=sendto(sock,res,length+1,0,(struct sockaddr*)&remote,sizeof(struct sockaddr))==-1)
			{
				cout<<"sending error"<<endl;
				return 0;
			}
		
		}
		
		//send the end message
		if(len=sendto(sock,success,10,0,(struct sockaddr*)&remote,sizeof(struct sockaddr))==-1)
		{
			cout<<"sending error"<<endl;
			return 0;
		}
		cout<<"The Server AND has successfully finished sending all computation results to the edge server."<<endl;
	    
		//release cache
		for(int i=0;i<result_list.size();i++)  
		{
			delete result_list[i];
		}
	    

	
	}
	close(sock);
}

/**
function for doing the operation
**/
char* dispose(char* source,int length)
{
	char * test=new char[50];
	
	//get information from buffer
	for(int i=0;i<length;i++)
    {
		test[i]=source[i];
    }
	int lena=0,lenb=0,len=0,start=0;
	char num[2];
	bool digiFlag=false;
	
	//get the position of two operands a and b
	for(int i=0;test[i]!='#';i++)
    {
		len=i;
		if(test[i]==',')
		{
			if(start!=0)
			{
				lena=i-start-1;
			}
			start=i;
		}
    }
	num[0]=test[len+2];
	if(test[len+3]!='$'&&test[len+4]=='$')
	{
		digiFlag=true;
		num[1]=test[len+3];
	}
	lenb=len-start;
	len=max(lena,lenb);
	
	//start to get two operands.
	char a[len],b[len];
	start=0;
	int count=0;
	for(int i=0;i<length;i++)
    {
		if(test[i]==',')
		{
			start++;
			if(start==1)
			{
				for(int j=0;j<len-lena;j++)
				{
					a[j]='0';
				}
				count=len-lena;
			}
			if(start==2)
			{
				for(int j=0;j<len-lenb;j++)
				{
					b[j]='0';
				}
				count=len-lenb;
			}
		}
		else
		{
			if(start==1)
			{
				a[count++]=test[i];
			}
			if(start==2)
			{
				b[count++]=test[i];
			}
		}
    }

	delete test;
	
	
	//calculate and output the result.
	char* res=new char[50];
	bool flag=false;
	count=0;
	for(int i=len-lena;i<len;i++)
    {
		res[count++]=a[i];
    }
	res[count++]=' ';
	res[count++]='a';
	res[count++]='n';
	res[count++]='d';
	res[count++]=' ';
	for(int i=len-lenb;i<len;i++)
    {
		res[count++]=b[i];
    }
	res[count++]=' ';
	res[count++]='=';
	res[count++]=' ';
	for(int i=0;i<len;i++)
    {
		int tmp=(a[i]-'0')&(b[i]-'0');
		if(flag||tmp==1)
		{
			flag=true;
			res[count++]=(char)(tmp+'0');
		}
    }
	if(!flag)
	{
		res[count++]='0';
    }
	res[count++]='#';
	res[count++]=num[0];
	if(digiFlag)
	{
		res[count++]=num[1];
    }
	res[count]='$';
	for(int i=0;res[i]!='#';i++)
    {
		cout<<res[i];
    }
	cout<<endl;

	return res;
}
