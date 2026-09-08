//
// Created by bysunlits on 2026/9/6.
//
#include <stdio.h>

enum INFOMESSAGE
{
	START_MSG, IN_PROGRESS_MSG, END_MSG
};

int main()
{
	{
		size_t buffer[256*1024];
		if (fread(buffer,1,sizeof(buffer),stdin)) {

		}
	}

}
