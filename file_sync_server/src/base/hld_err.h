//
// Created by wnz on 2026/9/30.
//
#pragma once
#ifndef SERVER_ERR_H
#define SERVER_ERR_H
#include "hld_err.h"

#endif //SERVER_ERR_H
typedef struct ERR_INFO
{
	int ERR_CODE;
	char *MESSAGE;
}ERR_INFO;

int hld_err(ERR_INFO **ERR_PTR,const char *message,const int *err_code);
