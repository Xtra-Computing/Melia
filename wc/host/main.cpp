#include "lib/scheduler.h"
#include <iostream>
using namespace std;
int tony=-1;//-1 means CPU only, 0 means CPU+GPU, 1 means GPU only.
void DataGenerate_keySize4(char * filename,int NW,int DW){
	FILE *file;
	file = fopen(filename, "w");
	int count=65;
	int count_count=65;
	int count_count_count=65;
	for(int i=0;i<NW;i++){
			fprintf(file,"%c%c%c\n",(char)(count),(char)(count_count),(char)(count_count_count));
			count_count_count++;
			if((count==DW/26/26+65)&&(count_count==DW%(676)/26+65)&&(count_count_count==DW%(17576)+65)){
				count=65;count_count=65;count_count_count=65;
			}
			else{
				if(count_count_count>90){
					count_count++;
					count_count_count=65;
					if(count_count>90){
					 count++;
					 count_count=65;
					 if(count>90){
						count=65;
						}
					}
				}
			}
	}
	fclose(file);
}
void DataGenerate(char * filename,int NW,int DW){
	FILE *file;
	file = fopen(filename, "w");
	int count=65;
	int count_count=65;
	int count_count_count=65;
	for(int i=0;i<NW;i++){
		if(DW<=26){
		//	printf("%c\n",(char)(count));
			fprintf(file,"%c\n",(char)(count));
			count++;
			if(count==DW+65){
			 count =65;
			}						
		}else if(DW<=26*26){
			fprintf(file,"%c%c\n",(char)(count),(char)(count_count));
			count_count++;
			if((count==DW/26+65)&&(count_count==DW%(26)+65)){
				count_count=65;
				count=65;	
			}
			else{
				if(count_count>90){
					count++;
					 count_count=65;
					 if(count>90){
						count=65;
					}
				}
			}
		}
		else if(DW<=26*26*26){
			fprintf(file,"%c%c%c\n",(char)(count),(char)(count_count),(char)(count_count_count));
			count_count_count++;
			if((count==DW/26/26+65)&&(count_count==DW%(676)/26+65)&&(count_count_count==DW%(17576)+65)){
				count=65;count_count=65;count_count_count=65;
			}
			else{
				if(count_count_count>90){
					count_count++;
					count_count_count=65;
					if(count_count>90){
					 count++;
					 count_count=65;
					 if(count>90){
						count=65;
						}
					}
				}
			}
		}
	}
	fclose(file);
}

void DataGenerate_sort(char * filename,int NW,int DW){
	FILE *file;
	file = fopen(filename, "w");
	int count=65;
	int count_count=65;
	int count_count_count=65;
	//printf("%d %d\n", NW, DW);
	int size_dw   = NW/DW;

	for(int i=0;i<NW;i++){
		if(DW<=26){
		//	printf("%c\n",(char)(count));
			fprintf(file,"%c\n",(char)(count));
			count++;
			if(count==DW+65){
			 count =65;
			}						
		}
		else if(DW<=26*26)
		{  // printf("i/size_dw_3 = %d\n", i/size_dw_2);
			int key_index = i/size_dw;

			fprintf(file,"%c%c\n",(char)(65+(key_index/26)),(char)(65+(key_index%26)));

			/*
			fprintf(file,"%c%c\n",(char)(count),(char)(count_count));
			count_count++;
			if((count==DW/26+65)&&(count_count==DW%(26)+65)){
				count_count=65;
				count=65;	
			}
			else{
				if(count_count>90){
					count++;
					 count_count=65;
					 if(count>90){
						count=65;
					}
				}
			}
			*/
		}
		else if(DW<=26*26*26)
		{
			int key_index = i/size_dw;

			fprintf(file,"%c%c%c\n",(char)(65+key_index/676),(char)(65+(key_index/26)),(char)(65+(key_index%26)));

/*			fprintf(file,"%c%c%c\n",(char)(count),(char)(count_count),(char)(count_count_count));
			count_count_count++;
			if((count==DW/26/26+65)&&(count_count==DW%(676)/26+65)&&(count_count_count==DW%(17576)+65)){
				count=65;count_count=65;count_count_count=65;
			}
			else{
				if(count_count_count>90){
					count_count++;
					count_count_count=65;
					if(count_count>90){
					 count++;
					 count_count=65;
					 if(count>90){
						count=65;
						}
					}
				}
			}
*/
	   }
	}
	fclose(file);
}
int main(int argc, char** argv)
{
	if(argc==2){
		tony = atoi(argv[1]);
	}
	char filename[] = "data.txt";
	//char filename[] = "word_10MB.txt";
	//DataGenerate_keySize4(filename,1024*1024*1,400);
	DataGenerate(filename,25*1024*1024*1,500);//DataGenerate  DataGenerate_sort
	printf("data generated\n");
	FILE *file;
	file = fopen(filename, "r");
	fseek(file, 0, SEEK_END);
	int nLen = ftell(file);
	rewind(file);
	char *filebuf = new char[nLen + 1];
	nLen = fread(filebuf, sizeof(char), nLen, file);
	fclose(file);

	filebuf[nLen] = '\0';

	vector<unsigned int> offsets;
	unsigned int offset = 0;

	FILE *fp = fopen(filename, "r");

	bool in_word = false;
	char ch;
	do
	{
		ch = fgetc(fp);	
		if(ch==EOF)
			break;
		if(ch>='a'&&ch<='z'
			||ch>='0'&&ch<='9'
			||ch>='A'&&ch<='Z')
		{
			if(!in_word)
			{
				in_word = true;
				offsets.push_back(offset);
			}
		}

		else
		{
			if(in_word)
			{
				in_word = false;
				filebuf[offset] = '\0';
			}
		}
		offset++;
	}while(ch!=EOF);
	fclose(fp);
	cout<<"offset number: "<<offsets.size()<<endl;
	Scheduler scheduler((void *)filebuf, nLen + 1, &offsets[0], offsets.size(), sizeof(int));
	scheduler.do_mapreduce();
	struct output output = scheduler.get_output();

	int key_num = scheduler.get_key_num();
	int total_num = 0;
	for(int i = 0; i < key_num; i++)
	{
			char *key_address = output.output_keys + output.key_index[i];
            char *val_address = output.output_vals + output.val_index[i];
			int number = *(int *)val_address;
			cout<<key_address<<":"<<number<<endl;
			total_num+=number;
	}
	cout<<"total num of words: "<<total_num<<endl;

	delete[] filebuf;
	scheduler.freeResource();
	int a;
	std::cout<<"Enter any number to continue..."<<std::endl;
	std::cin>>a;
	return 0;
}