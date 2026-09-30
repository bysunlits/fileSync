//
// Created by wnz on 2026/9/30.
//

#include "hld_err.h"
#include <stdlib.h>

int hld_err(ERR_INFO **ERR_PTR,const char *message,const int *err_code)
{
	if (*ERR_PTR==NULL)
	{
		return 0;
	}
	else if (message==NULL||err_code==NULL)
	{
		return -1;
	}
	else
	{
		(**ERR_PTR).ERR_CODE=*err_code;
		(**ERR_PTR).MESSAGE=message;
		return 0;
	}

}