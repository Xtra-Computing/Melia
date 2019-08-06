#ifndef UTILHOST
#define UTILHOST

#include <fstream>
#include "stdio.h"

class Kernelfile
{
public:
	Kernelfile()
	{
		this->source_ = NULL;
		this->size_ = 0;
	}

	~Kernelfile()
	{
		free(source_);
	}

	bool open(const char *filename)
	{
		FILE *file;
		if(fopen_s(&file, filename, "r")!=0)
		{
			printf("file cannot be loaded...\n");
			return false;
		}
		fseek(file, 0, SEEK_END);
		size_ = ftell(file);
		source_ = (char *)malloc(size_ + 1);
		fseek(file, 0, SEEK_SET);
		fread(source_, 1, size_, file);
		//fread(source_, 1, size_, file);
		source_[size_] = '\0';
		fclose(file);
			return true;
	}
	
	char * source()
	{
		return source_;
	}

	int size()
	{
		return size_;
	}

private:
	char * source_;
	int size_;
};
#endif