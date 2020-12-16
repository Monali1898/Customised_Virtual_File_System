#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#include<string.h>

#define MAXFILES 100
#define FILESIZE 1024

#define READ 4
#define WRITE 2

#define REGULAR 1
#define SPECIAL 2

#define START 0
#define CURRENT 1
#define END 2

struct SuperBlock
{
	int TotalInodes;
	int FreeInode;
}Obj_Super;

struct inode
{
	char File_name[50];
	int Inode_number;
	int File_Size;
	int File_Type;
	int ActualFileSize;
	int Link_Count;
	int Reference_Count;
	int Permission;
	char * Data;
	struct inode *next;
};

typedef struct inode INODE;
typedef struct inode * PINODE;
typedef struct inode ** PPINODE;

struct FileTable
{
	int ReadOffset;
	int WriteOffset;
	int Count;
	PINODE iptr;
	int Mode;
};

typedef FileTable FILETABLE;
typedef FileTable * PFILETABLE;

struct UFDT
{
	PFILETABLE ufdt[MAXFILES];
}UFDTObj;

PINODE Head = NULL;  //global pointer of inode

// It is used to check whether the given file name is already present or not
bool ChkFile(char *name)
{
	PINODE temp = Head;
	
	while(temp != NULL)
	{
		if(temp->File_Type != 0)
		{
			if(strcmp(temp->File_name,name) == 0)
			{
				break;
			}
		}
		temp = temp -> next;
	}
	
	if(temp == NULL)
	{
		return false;
	}
	else
	{
		return true;
	}
}

int GetFDFromName(char * name)
{
	int i = 0;
	
	while(i < 50)
	{
		if(UFDTObj.ufdt[i] != NULL)
		{
			if(strcmp((UFDTObj.ufdt[i]->iptr->File_name),name) == 0)
			{
				break;
			}
			i++;
		}
	}
	
	if(i == 50)
		return -1;
	else 
		return i;
}

PINODE Get_Inode(char *name)
{
	PINODE temp = Head;
	int i = 0;
	
	if(name == NULL)
		return NULL;
	
	while(temp != NULL)
	{
		if(strcmp(name,temp->File_name) == 0)
			break;
		temp = temp->next;
	}
	
	return temp;
}

void CreateUFDT()
{
	int i = 0;
	for(i = 0; i < MAXFILES; i++)
	{
		UFDTObj.ufdt[i] = NULL;
	}
}

void CreateDILB()
{
	//create linkede list of inode
	
	int i = 1;
	
	PINODE newn = NULL;
	PINODE temp = Head;
	
	while(i <= MAXFILES)
	{
		newn = (PINODE)malloc(sizeof(INODE));
		
		newn->Inode_number = i;
		newn->File_Size = FILESIZE;
	    newn->File_Type = 0;
	    newn->ActualFileSize = 0;
		newn->Link_Count = 0;
		newn->Reference_Count = 0;
		newn->Data = NULL;
		newn->next = NULL;
		
		if(Head == NULL)
		{
			Head = newn;
			temp = Head;
		}
		else
		{
			temp->next = newn;
			temp = temp->next;
		}
		
		i++;
	}
	printf("DILB created succesfully!!\n");
}

void CreateSuperBlock()
{
	Obj_Super.TotalInodes = MAXFILES;
	Obj_Super.FreeInode = MAXFILES;
	
	printf("Super block created Succesfully!!\n");
}

void SetEnvoirnment()
{
	CreateDILB();
	CreateSuperBlock();
	CreateUFDT();
	printf("Envoirnment for the Virtual file system is set..\n");
}

void DeleteFile(char * name)
{
	bool bret = false;
	
	if((name == NULL))
	{
		return ;
	}
	
	bret = ChkFile(name);
	
	if(bret == false)
	{
		printf("There is no such file\n");
		return ;
	}
	
	// Search UFDT entry
	int i = 0;
	for(i = 0; i < MAXFILES; i++)
	{
		 if(strcmp(UFDTObj.ufdt[i]->iptr->File_name,name)== 0)
		 {
			 break;
		 }
	}
	
	strcpy(UFDTObj.ufdt[i]->iptr->File_name,"");
	UFDTObj.ufdt[i]->iptr->File_Type = 0;
	UFDTObj.ufdt[i]->iptr->ActualFileSize = 0;
	UFDTObj.ufdt[i]->iptr->Link_Count = 0;
	UFDTObj.ufdt[i]->iptr->Reference_Count = 0;
	
	//Free the memory of file
	free(UFDTObj.ufdt[i]->iptr->Data);

	free(UFDTObj.ufdt[i]);
	
	UFDTObj.ufdt[i] = NULL;
	
	printf("File is Succesfully deleted\n");
	
	Obj_Super.FreeInode++;
}

int CreateFile(char * name, int Permission)
{
	bool bret = false;
	
	if((name == NULL) || (Permission > READ+WRITE) || (Permission < WRITE))
	{
		return -1;
	}
	
	bret = ChkFile(name);
	
	if(bret == true)
	{
		printf("File is already present\n");
		return -1;
	}
	
	if(Obj_Super.FreeInode == 0)
	{
		printf("There is no inode to create the file\n");
		return -1;
	}
	
	// Search for empty entry from UFDT
	int i = 0;
	for(i = 0; i <MAXFILES; i++)
	{
		 if(UFDTObj.ufdt[i] == NULL)
		 {
			 break;
		 }
	}
	
	if(i == MAXFILES)
	{
		printf("UNable to create UFDT\n");
		return -1;
	}
	
	// Allicate memory for file table
	UFDTObj.ufdt[i] = (PFILETABLE)malloc(sizeof(FILETABLE));
	
	// Iniotialiose the file table
	UFDTObj.ufdt[i]->ReadOffset = 0;
	UFDTObj.ufdt[i]->WriteOffset = 0;
	UFDTObj.ufdt[i]->Mode = Permission;
	UFDTObj.ufdt[i]->Count = 1;
	
	//search empty inode
	PINODE temp = Head;
	
	while(temp != NULL)
	{
		if(temp->File_Type == 0)
		{
			break;
		}
		temp = temp ->next;
	}
	
	UFDTObj.ufdt[i]->iptr = temp;
	strcpy(UFDTObj.ufdt[i]->iptr->File_name,name);
	UFDTObj.ufdt[i]->iptr->File_Type = REGULAR;
	UFDTObj.ufdt[i]->iptr->ActualFileSize = 0;
	UFDTObj.ufdt[i]->iptr->Link_Count = 1;
	UFDTObj.ufdt[i]->iptr->Reference_Count = 1;
	
	//Allocate memory for file data
	UFDTObj.ufdt[i]->iptr->Data = (char *)malloc(sizeof(FILESIZE));
	
	Obj_Super.FreeInode--;
	
	return i;
}

void LS()
{
	PINODE temp = Head;
	
	while(temp != NULL)
	{
		if(temp -> File_Type != 0)
		{
			printf("%s\n",temp->File_name);
		}
		temp = temp->next;
	}
}

int WriteFile(int fd, char * arr, int Size)
{
	if(UFDTObj.ufdt[fd] == NULL)
	{
		printf("Invalid file descriptor\n");
		return -1;
	}
	
	if(UFDTObj.ufdt[fd]->Mode == READ)
	{
		printf("There is no write Permission\n");
		return -1;
	}
	
	strncpy(((UFDTObj.ufdt[fd]->iptr->Data)+(UFDTObj.ufdt[fd]->WriteOffset)),arr,Size); // kuth kashatun kiti
	
	UFDTObj.ufdt[fd]->WriteOffset = UFDTObj.ufdt[fd]->WriteOffset + Size;
	
	return Size;
}

int ReadFile(int fd, char *arr, int Size)
{
	int read_size = 0;
	
	if(UFDTObj.ufdt[fd] == NULL)
		return -1;
	
	if(UFDTObj.ufdt[fd]->Mode != READ && UFDTObj.ufdt[fd]->Mode != READ+WRITE)
		return -2;
	
	if(UFDTObj.ufdt[fd]->iptr->Permission != READ && UFDTObj.ufdt[fd]->iptr->Permission != READ+WRITE)
		return -2;
	
	if(UFDTObj.ufdt[fd]->ReadOffset == UFDTObj.ufdt[fd]->iptr->ActualFileSize)
		return -3;
	
	if(UFDTObj.ufdt[fd]->iptr->File_Type != REGULAR)
		return -4;
	
	read_size = (UFDTObj.ufdt[fd]->iptr->ActualFileSize) - (UFDTObj.ufdt[fd]->ReadOffset);
	
	if(read_size < Size)
	{
		strncpy(arr,(UFDTObj.ufdt[fd]->iptr->Data) + (UFDTObj.ufdt[fd]->ReadOffset),read_size);
		
		UFDTObj.ufdt[fd]->ReadOffset = UFDTObj.ufdt[fd]->ReadOffset + read_size;
	}
	else
	{
		strncpy(arr,(UFDTObj.ufdt[fd]->iptr->Data) + (UFDTObj.ufdt[fd]->ReadOffset),Size);
		
		(UFDTObj.ufdt[fd]->ReadOffset) = (UFDTObj.ufdt[fd]->ReadOffset)+Size;
	}
	
	return Size;
}
/*
int WriteFile(int fd, char *arr, int size)
{
	if(((UFDTObj.ufdt[fd]->Mode) != WRITE) && ((UFDTObj.ufdt[fd]->Mode) != READ+WRITE))
		return -1;
	
	if(((UFDTObj.ufdt[fd]->iptr->Permission) != WRITE) && ((UFDTObj.ufdt[fd]->iptr->Permission) != READ+WRITE))
		return -1;
	
	if((UFDTObj.ufdt[fd]->WriteOffset) == MAXFILES)
		return -2;
	
	if((UFDTObj.ufdt[fd]->iptr->File_Type) != REGULAR)
		return -3;
	
	strcpy((UFDTObj.ufdt[fd]->iptr->Data) + (UFDTObj.ufdt[fd]->WriteOffset),arr,size);
	
	(UFDTObj.ufdt[fd]->WriteOffset) = (UFDTObj.ufdt[fd]->WriteOffset) + size;
	
	(UFDTObj.ufdt[fd]->iptr->ActualFileSize) = (UFDTObj.ufdt[fd]->iptr->ActualFileSize) + size;
	
	return size;
}
*/

int OpenFile(char *name, int Mode)
{
	int i = 0;
	PINODE temp = NULL;
	
	if(name == NULL || Mode <= 0)
		return -1;
	
	temp = Get_Inode(name);
	
	if(temp == NULL)
		return -2;
	
	if(temp->Permission < Mode)
		return -3;
	
	while(i < 50)
	{
		if(UFDTObj.ufdt[i] == NULL)
			break;
		i++;
	}
	
	UFDTObj.ufdt[i] = (PFILETABLE)malloc(sizeof(FILETABLE));
	
	if(UFDTObj.ufdt[i] == NULL)
		return -1;
	
	UFDTObj.ufdt[i]->Count = 1;
	UFDTObj.ufdt[i]->Mode = Mode;
	
	if(Mode == READ+WRITE)
	{
		UFDTObj.ufdt[i]->ReadOffset = 0;
		UFDTObj.ufdt[i]->WriteOffset = 0;
	}
	else if(Mode == READ)
	{
		UFDTObj.ufdt[i]->ReadOffset = 0;
	}
	else if(Mode == WRITE)
	{
		UFDTObj.ufdt[i]->WriteOffset = 0;
	}
	
	UFDTObj.ufdt[i]->iptr = temp;
	(UFDTObj.ufdt[i]->iptr->Reference_Count)--;
	
	return i;
}

int fstat_file(int fd)
{
	PINODE temp = Head;
	int i = 0;
	
	if(fd < 0)
		return -1;
	
	if(UFDTObj.ufdt[i] == NULL)
		return -2;
	
	temp = UFDTObj.ufdt[i]->iptr;
		
	printf("\n------Statistical Information about file------\n");
	printf("File name : %s\n",temp->File_name);
	printf("Inode Number : %d\n",temp->Inode_number);
	printf("File size : %d\n",temp->File_Size);
	printf("Actual File size : %d\n",temp->ActualFileSize);
	printf("Link count : %d\n",temp->Link_Count);
	printf("Reference Count : %d\n",temp->Reference_Count);
	
	if(temp->Permission == 4)
		printf("File Permission : Read only\n");
	else if(temp->Permission == 2)
		printf("File Permission : Write only\n");
	else if(temp->Permission == 6)
		printf("File Permission : Read & Write\n");
	printf("------------------------------------------------\n");
	
	return 0;
}

int stat_file(char *name)
{
	PINODE temp = Head;
	int i = 0;
	
	if(name == NULL)
		return -1;
	
	while(temp != NULL)
	{
		if(strcmp(name, temp->File_name) == 0)
			break;
		temp = temp->next;
	}
	
	if(temp == NULL)
		return -2;
	
	printf("\n------Statistical Information about file------\n");
	printf("File name : %s\n",temp->File_name);
	printf("Inode Number : %d\n",temp->Inode_number);
	printf("File size : %d\n",temp->File_Size);
	printf("Actual File size : %d\n",temp->ActualFileSize);
	printf("Link count : %d\n",temp->Link_Count);
	printf("Reference Count : %d\n",temp->Reference_Count);
	
	if(temp->Permission == 4)
		printf("File Permission : Read only\n");
	else if(temp->Permission == 2)
		printf("File Permission : Write only\n");
	else if(temp->Permission == 6)
		printf("File Permission : Read & Write\n");
	printf("------------------------------------------------\n");
	
	return 0;
}

int CloseFileByName(char *name)
{
	int i = 0;
	i = GetFDFromName(name);
	
	if(i == -1)
		return -1;
	
	UFDTObj.ufdt[i]->ReadOffset = 0;
	UFDTObj.ufdt[i]->WriteOffset = 0;
	(UFDTObj.ufdt[i]->iptr->Reference_Count)--;
	
	return 0;
}

void CloseAllFile()
{
	int i = 0;
	while(i < 50)
	{
		if(UFDTObj.ufdt[i] != NULL)
		{
			UFDTObj.ufdt[i]->ReadOffset = 0;
			UFDTObj.ufdt[i]->WriteOffset = 0;
			(UFDTObj.ufdt[i]->iptr->Reference_Count)--;
			break;
		}
		i++;
	}
}

int truncate_File(char *name)
{
	int i = 0;
	i = GetFDFromName(name);
	
	if(i == -1)
	{
		return -1;
	}
	
	memset(UFDTObj.ufdt[i]->iptr->Data,0,1024);
	UFDTObj.ufdt[i]->ReadOffset = 0;
	UFDTObj.ufdt[i]->WriteOffset = 0;
	UFDTObj.ufdt[i]->iptr->ActualFileSize = 0;
	
	return 0;
}

int LseekFile(int fd, int size, int from)
{
	int i = 0;
	
	if((fd < 0) || (from > 2))
		return -1;
	
	if(UFDTObj.ufdt[i] == NULL)
		return -1;
	
	if((UFDTObj.ufdt[i]->Mode == READ) || (UFDTObj.ufdt[i]->Mode == READ+WRITE))
	{
		if(from == CURRENT)
		{
			if(((UFDTObj.ufdt[i]->ReadOffset) + size) > UFDTObj.ufdt[i]->iptr->ActualFileSize)
				return -1;
			
			if(((UFDTObj.ufdt[i]->ReadOffset) + size) < 0)
				return -1;
			
			(UFDTObj.ufdt[i]->ReadOffset) = (UFDTObj.ufdt[i]->ReadOffset) + size;
		}
		else if(from == START)
		{
			if(size > (UFDTObj.ufdt[i]->iptr->ActualFileSize))
				return -1;
			
			if(size < 0)
				return -1;
			
			(UFDTObj.ufdt[i]->ReadOffset) = size;
		}
		else if(from == END)
		{
			if((UFDTObj.ufdt[i]->iptr->ActualFileSize) + size > MAXFILES )
				return -1;
			
			if(((UFDTObj.ufdt[i]->ReadOffset) + size) < 0)
				return -1;
			
			(UFDTObj.ufdt[i]->ReadOffset) = (UFDTObj.ufdt[i]->iptr->ActualFileSize) + size;
		}
	}
	else if(UFDTObj.ufdt[i]->Mode == WRITE)
	{
		if(from == CURRENT)
		{
			if(((UFDTObj.ufdt[i]->WriteOffset) + size) > MAXFILES)
				return -1;
				
			if(((UFDTObj.ufdt[i]->WriteOffset) + size) < 0)
				return -1;
			
			if(((UFDTObj.ufdt[i]->WriteOffset) + size) > (UFDTObj.ufdt[i]->iptr->ActualFileSize))
				(UFDTObj.ufdt[i]->iptr->ActualFileSize) = (UFDTObj.ufdt[i]->WriteOffset) + size;
			
			(UFDTObj.ufdt[i]->WriteOffset) = (UFDTObj.ufdt[i]->WriteOffset) + size;
			
		}
		else if(from == START)
		{
			if(size > MAXFILES)
				return -1;
			
			if(size < 0)
				return -1;
			
			if(size > (UFDTObj.ufdt[i]->iptr->ActualFileSize))
				(UFDTObj.ufdt[i]->iptr->ActualFileSize) = size;
			
			(UFDTObj.ufdt[i]->WriteOffset) = size;
		}
		else if(from == END)
		{
			if((UFDTObj.ufdt[i]->iptr->ActualFileSize) + size > MAXFILES)
				return -1;
			
			if(((UFDTObj.ufdt[i]->WriteOffset) + size) < 0)
				return -1;
			
			(UFDTObj.ufdt[i]->WriteOffset) = (UFDTObj.ufdt[i]->iptr->ActualFileSize) + size;
		}
	}
}

void DisplayHelp()
{
	printf("----------------------------------------------\n");
	printf("open : It is used to open the existing file\n");
	printf("close : It is used to close the opened file\n");
	printf("read : It is used to read the contents of file\n");
	printf("write : It is used to write the data into file\n");
	printf("ls : It is used to List out all files\n");
	printf("rm : It is used to Delete the file\n");
	printf("lseek : It is used to change the offset of file\n");
	printf("stat : It is used to display the information of file\n");
	printf("fstat : It is used to display the information of opened file\n");
	printf("truncate : It is used to Remove all data from file\n");
	printf("cls : It is used to clear console\n");
	printf("exit : It is used to Terminate file system\n");
	printf("closeall : It is used to close all opened file\n");
	printf("-------------------------------------------------\n");
}

void ManPage(char *str)
{
	if(strcmp(str,"open")== 0)
	{
		printf("Description : It is used to open an existing file\n");
		printf("Usage : Open File_name Mode\n");
	}
	else if(strcmp(str,"close")== 0)
	{
		printf("Desricption : It is used to close the existing file\n");
        printf("Usage : close File_name\n");
	}
	else if(strcmp(str,"ls") == 0)
	{
		printf("Desricption : It is used to list out all names of the files\n");
        printf("Usage : ls\n");
	}
	else if(strcmp(str,"create") == 0)
	{
		printf("Description : It is used to create new regular file\n");
		printf("Usage :create File_name Permission\n");
	}
	else if(strcmp(str,"rm") == 0)
	{
		printf("Description : It is used to delete regular file\n");
		printf("Usage : rm File_name\n");
	}
	else if(strcmp(str,"write")== 0)
	{
		printf("Description : It is used to write data into file\n");
		printf("Usage : write File_Desriptor\n");
		printf("After the command please enter the data\n");
	}
	else if(strcmp(str,"read") == 0)
	{
		printf("Description : Used to read data from regular file\n");
		printf("Usage : read File_name No_Of_Bytes_To_Read\n");
	}
	else if(strcmp(str,"fstat") == 0)
	{
		printf("Description : It is used to display information of file\n");
		printf("Usage : fstat File_Desriptor\n");
	}
	else if(strcmp(str,"stat") == 0)
	{
		printf("Description : It is used to Display information of entered file\n");
		printf("Usage : stat File_name\n");
	}
	else if(strcmp(str,"truncate") == 0)
	{
		printf("Description : Used to remove data from file\n");
		printf("Usage : truncate File_name\n");
	}
	else if(strcmp(str,"closeall") == 0)
	{
		printf("Description : It is used to close all open file\n");
		printf("Usage : closeall\n");
	}
	else if(strcmp(str,"lseek") == 0)
	{
		printf("Description : Used to change file offset\n");
		printf("Usage : lseek File_name ChangeInOffset StartPoint\n");
	}
	else
	{
		printf("Man page not found\n");
	}
}

int main()
{
	char *ptr = NULL;
	char arr[1024];
	char str[80];
	char command[4][80];
	int count = 0, ret = 0, fd = 0;
	printf("Customised virtual file system\n");
	
	SetEnvoirnment();
	
	while(1)
	{
		printf("Marvellous VFS :>");
		fgets(str,80,stdin);
		
		//scanf("%[^'\n']s",str);
		fflush(stdin);
		
		count = sscanf(str,"%s %s %s %s",command[0],command[1],command[2],command[3]);
		
		if(count == 1)
		{
			if(strcmp(command[0],"help")== 0)
			{
				DisplayHelp();
			}
			else if(strcmp(command[0],"exit")== 0)
			{
				printf("Thank you for using Marvellous Virtual file system");
				break;
			}
			else if(strcmp(command[0],"cls")== 0)
			{
				system("cls");
			}
			else if(strcmp(command[0],"ls")==0)
			{
				LS();
			}
			else if(strcmp(command[0],"closeall") == 0)
			{
				CloseAllFile();
				printf("All file closed Succesfully\n");
			}
			else
			{
				printf("Command not found!!\n");
			}
		}
		else if(count == 2)
		{
			if(strcmp(command[0],"man")== 0)
			{
				ManPage(command[1]);
			}
			else if(strcmp(command[0],"rm") == 0)
			{
				DeleteFile(command[1]);
			}
			else if(strcmp(command[0],"write") == 0)
			{
				char arr[1024];
				
				printf("Please enter data to write\n");
				fgets(arr,1024,stdin);
				
				fflush(stdin);
				
				ret = WriteFile(atoi(command[1]),arr,strlen(arr)-1);
				
				if(ret != -1)
				{
					printf("%d bytes gets written Succesfully in the file\n",ret);
				}
			}
			else if(strcmp(command[0],"fstat") == 0)
			{
				ret = fstat_file(atoi(command[1]));
				if(ret == -4)
					printf("ERROR : Incorrect parameter\n");
				if(ret == -2)
					printf("ERROR : There is no such file\n");
			}
			else if(strcmp(command[0],"stat") == 0)
			{
				ret = stat_file(command[1]);
				if(ret == -4)
					printf("ERROR : Incorrect parameter\n");
				if(ret == -2)
					printf("ERROR : There is no such file\n");
			}
			/*else if(strcmp(command[0],"write") == 0)
			{
				fd = GetFDFromName(command[1]);
				if(fd == -1)
				{
					printf("ERROR : Incorrect parameter\n");
				}
				
				printf("Enter the data :\n");
				scanf("%[^\n]",arr);
				
				ret = strlen(arr);
				
				if(ret == 0)
				{
					printf("ERROR : Incorrect parameter\n");
				}
				
				ret = WriteFile(fd,arr,ret);
				if(ret == -1)
				{
					printf("ERROR : Permission denied\n");
				}
				if(ret == -2)
				{
					printf("ERROR : The is no sufficient memeory to write\n");
				}
				if(ret == -3)
				{
					printf("ERROR : It is not regular file\n");
				}
			}*/
			else if(strcmp(command[0],"close") == 0)
			{
				ret = CloseFileByName(command[1]);
				
				if(ret == -1)
				{
					printf("ERROR : There is no such file\n");
				}
				printf("File is close Succesfully\n");
			}
			else if(strcmp(command[0],"truncate") == 0)
			{
				ret = truncate_File(command[1]);
				
				if(ret == -1)
				{
					printf("ERROR : Incorrect parameter\n");
				}
				printf("File is truncate Succesfully\n");
			}
			else
			{
				printf("Command not Found!!\n");
			}
		}
		else if(count == 3)
		{
			if(strcmp(command[0],"create") == 0)
			{
				fd = CreateFile(command[1],atoi(command[2]));     // Ascii to integer convert karne ke liye atoi
				
				if(fd == -1)
				{
					printf("Unable to create file\n");
				}
				else
				{
					printf("File is Succesfully created with FD %d\n",fd);
				}
			}
			else if(strcmp(command[0],"read") == 0)
			{
				fd = GetFDFromName(command[1]);
				
				if(fd == -1)
				{
					printf("ERROR : Incorrect parameter\n");
				}
				
				ptr = (char *)malloc(sizeof(atoi(command[2])) + 1);
				
				if(ptr == NULL)
				{
					printf("ERROR : Memory allocation failed\n");
				}
				
				ret = ReadFile(fd,ptr,atoi(command[2]));

				if(ret == -2)
				{
					printf("ERROR : File not exiting\n");
				}
				if(ret == -4)
				{
					printf("ERROR : Permission denied\n");
				}
				if(ret == -6)
				{
					printf("ERROR : Reached at end of file\n");
				}
				if(ret == -4)
				{
					printf("ERROR : It is not regular file\n");
				}
				if(ret > 0)
				{
					write(2,ptr,ret);
				}
				if(ret != -1)
				{
					printf("%d bytes gets read Succesfully in the file\n",ret);
				}
			}
			else if(strcmp(command[0],"open") == 0)
			{
				ret = OpenFile(command[1],atoi(command[2]));
				if(ret >= 0)
				{
					printf("File is Succesfully opened with file descriptor : %d\n",ret);
				}
				if(ret == -1)
				{
					printf("ERROR : Incorrect parameter\n");
				}
				if(ret == -2)
				{
					printf("ERROR : File not present\n");
				}
				if(ret == -3)
				{
					printf("ERROR : Permission denied\n");
				}
			}	
			else
			{
				printf("\nERROR : Command not found!!!\n");
			}
		}
		else if(count == 4)
		{
			if(strcmp(command[0],"lseek") == 0)
			{
				fd = GetFDFromName(command[1]);
				
				if(fd == -1)
				{
					printf("ERROR : Incorrect parameter\n");
				}
				ret = LseekFile(fd,atoi(command[2]),atoi(command[3]));
				
				if(ret == -1)
				{
					printf("Unable to perform lseek\n");
				}
			}
			else
			{
				printf("\nERROR : Command not found!!!\n");
			}
			printf("File offset is changed Succesfully\n");
		}
		else
		{
			//printf("Entered command is : %d",str);
			printf("Bad command of File name\n");
		}
	}
	return 0;
}