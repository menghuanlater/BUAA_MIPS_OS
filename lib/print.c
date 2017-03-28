/*
 * Copyright (C) 2001 MontaVista Software Inc.
 * Author: Jun Sun, jsun@mvista.com or jsun@junsun.net
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 * delecc
 */

#include	<print.h>

/* macros */
#define		IsDigit(x)	( ((x) >= '0') && ((x) <= '9') )
#define		Ctod(x)		( (x) - '0')

/* forward declaration */
extern int PrintChar(char *, char, int, int);
extern int PrintString(char *, char *, int, int);
extern int PrintNum(char *, unsigned long, int, int, int, int, char, int);

/* private variable */
static const char theFatalMsg[] = "fatal error in lp_Print!";

static int flagOfA;

/* -*-
 * A low level printf() function.
 */
void
lp_Print(void (*output)(void *, char *, int), 
	 void * arg,
	 char *fmt, 
	 va_list ap)
{

#define 	OUTPUT(arg, s, l)  \
  { if (((l) < 0) || ((l) > LP_MAX_BUF)) { \
       (*output)(arg, (char*)theFatalMsg, sizeof(theFatalMsg)-1); for(;;); \
    } else { \
      (*output)(arg, s, l); \
    } \
  }
    
    char buf[LP_MAX_BUF];//其他一定会输出的字符缓冲区？

    char c;//no
    char *s;//no
    long int num;//no

    int longFlag;//long int的标志
    int negFlag;//负数
	int posFlag;//正数加号是否输出
	int myFlag; //八进制以及十六进制的输出前缀标识符
    int width;//宽度
    int prec;//精度
    int ladjust;//左对齐(leftadjust)
    char padc;//与输出对齐有关，参见下方PrintNum函数的注释说明

    int length;//no

    for(;;) {
	{ 
	    /* scan for the next '%' */
		char *fmtTemp = fmt;
		while(*fmt!='%' && *fmt){
			fmt++;	
		}
	    /* flush the string found so far */
		OUTPUT(arg,fmtTemp,fmt-fmtTemp);
	    /* are we hitting the end? */
		if(!*fmt)
			break;
	}

	/* we found a '%' */
	fmt++; //将fmt定位到%后一个字符处
	/* check for long */
	if(*fmt=='l'){
		longFlag = 1;fmt++;
	}else{
		longFlag = 0;
	}
	/* check for other prefixes */
	/*
	** 需要考虑: '-'--->Left-justify within the given field width; Right justification is the default 
	** '+'--->Forces to preceed the result with a plus or minus sign (+ or -) even for positive numbers. By default, only		 negative numbers are preceded with a - sign.
	** (space)-->just output
	** '#'-->Used with o, x or X specifiers the value is preceeded with 0, 0x or 0X respectively for values different than zero.
	Used with a, A, e, E, f, F, g or G it forces the written output to contain a decimal point even if no more digits follow. By default, if no digits follow, no decimal point is written.
	**'0'-->Left-pads the number with zeroes (0) instead of spaces when padding is specified (see width sub-specifier).
	*/
	//首先，初始化各项变量数据
	width = 0;
	prec = -1;
	ladjust = 0;
	padc = ' ';
	posFlag = 0;
	myFlag = 0;
	if(*fmt=='-'){
		ladjust = 1;
		fmt++;
	}
	if(*fmt=='+'){
		posFlag = 1;
		fmt++;
	}
	if(*fmt==' '){
		OUTPUT(arg,fmt,1);
		fmt++;
	}
	if(*fmt=='#'){
		myFlag = 1;
		fmt++;
	}
	if(*fmt=='0'){
		padc = '0';//表示对齐用'0'填充
		fmt++;
	}
	if(*fmt>='1' && *fmt<='9'){
		width = *fmt - '0';
		fmt++;
		while(*fmt>='0' && *fmt<='9'){
			width = width*10 + *fmt-'0';
			fmt++;
		}
	}
	if(*fmt=='.'){
		fmt++;
		while(*fmt>='0' && *fmt<='9'){
			prec = prec*10 + *fmt-'0';
			fmt++;
		}
	}
	
	/* check format flag */
	negFlag = 0;
	switch (*fmt) {
	 case 'b':
	    if (longFlag) { 
		num = va_arg(ap, long int); 
	    } else { 
		num = va_arg(ap, int);
	    }
	    length = PrintNum(buf, num, 2, 0, width, ladjust, padc, 0);
	    OUTPUT(arg, buf, length);
	    break;

	 case 'd':
	 case 'D':
	    if (longFlag) { 
		num = va_arg(ap, long int);
	    } else { 
		num = va_arg(ap, int); 
	    }
	    if (num < 0) {
		num = - num;
		negFlag = 1;
	    }
		if(num > 0 && posFlag){
			OUTPUT(arg,"+",1);
		}
	    length = PrintNum(buf, num, 10, negFlag, width, ladjust, padc, 0);
	    OUTPUT(arg, buf, length);
	    break;

	 case 'o':
	 case 'O':
	 	if(myFlag){
			OUTPUT(arg,"0",1);
		}
	    if (longFlag) { 
		num = va_arg(ap, long int);
	    } else { 
		num = va_arg(ap, int); 
	    }
	    length = PrintNum(buf, num, 8, 0, width, ladjust, padc, 0);
	    OUTPUT(arg, buf, length);
	    break;

	 case 't':
	 	if(longFlag){
		num = va_arg(ap,long int);
		}else{
		num = va_arg(ap,int);
		}
		length = PrintNum(buf,num,11,0,width,ladjust,padc,0);
		flagOfA = 0;
		OUTPUT(arg,buf,length);
		break;
	 case 'T':
        if(longFlag){
		num = va_arg(ap,long int);
		}else{
		num = va_arg(ap,int);
		}
		flagOfA = 1;
		length = PrintNum(buf,num,11,0,width,ladjust,padc,0);
		OUTPUT(arg,buf,length);
		break;

	 case 'u':
	 case 'U':
	    if (longFlag) { 
		num = va_arg(ap, long int);
	    } else { 
		num = va_arg(ap, int); 
	    }
	    length = PrintNum(buf, num, 10, 0, width, ladjust, padc, 0);
	    OUTPUT(arg, buf, length);
	    break;
	    
	 case 'x':
	 	if(myFlag){
			OUTPUT(arg,"0x",2);
		}
	    if (longFlag) { 
		num = va_arg(ap, long int);
	    } else { 
		num = va_arg(ap, int); 
	    }
	    length = PrintNum(buf, num, 16, 0, width, ladjust, padc, 0);
	    OUTPUT(arg, buf, length);
	    break;

	 case 'X':
	 	if(myFlag){
			OUTPUT(arg,"0X",2);
		}
	    if (longFlag) { 
		num = va_arg(ap, long int);
	    } else { 
		num = va_arg(ap, int); 
	    }
	    length = PrintNum(buf, num, 16, 0, width, ladjust, padc, 1);
	    OUTPUT(arg, buf, length);
	    break;

	 case 'c':
	    c = (char)va_arg(ap, int);
	    length = PrintChar(buf, c, width, ladjust);
	    OUTPUT(arg, buf, length);
	    break;

	 case 's':
	    s = (char*)va_arg(ap, char *);
	    length = PrintString(buf, s, width, ladjust);
	    OUTPUT(arg, buf, length);
	    break;

	 case '\0':
	    fmt --;
	    break;

	 default:
	    /* output this char as it is */
	    OUTPUT(arg, fmt, 1);
	}	/* switch (*fmt) */

	fmt ++;
    }		/* for(;;) */

    /* special termination call */
    OUTPUT(arg, "\0", 1);
}


/* --------------- local help functions --------------------- */
int
PrintChar(char * buf, char c, int length, int ladjust)
{
    int i;
    
    if (length < 1) length = 1;
    if (ladjust) {
	*buf = c;
	for (i=1; i< length; i++) buf[i] = ' ';
    } else {
	for (i=0; i< length-1; i++) buf[i] = ' ';
	buf[length - 1] = c;
    }
    return length;
}

int
PrintString(char * buf, char* s, int length, int ladjust)
{
    int i;
    int len=0;
    char* s1 = s;
    while (*s1++) len++;
    if (length < len) length = len;

    if (ladjust) {
	for (i=0; i< len; i++) buf[i] = s[i];
	for (i=len; i< length; i++) buf[i] = ' ';
    } else {
	for (i=0; i< length-len; i++) buf[i] = ' ';
	for (i=length-len; i < length; i++) buf[i] = s[i-length+len];
    }
    return length;
}

int
PrintNum(char * buf, unsigned long u, int base, int negFlag, 
	 int length, int ladjust, char padc, int upcase)
{
    /* algorithm :
     *  1. prints the number from left to right in reverse form.
     *  2. fill the remaining spaces with padc if length is longer than
     *     the actual length
     *     TRICKY : if left adjusted, no "0" padding.
     *		    if negtive, insert  "0" padding between "0" and number.
     *  3. if (!ladjust) we reverse the whole string including paddings
     *  4. otherwise we only reverse the actual string representing the num.
     */

    int actualLength =0;
    char *p = buf;
    int i;

    do {
	int tmp = u %base;
	if (tmp <= 9) {
	    *p++ = '0' + tmp;
	}else if(base==11 && tmp==10){
		*p++ = (flagOfA)? 'X':'x';
	} else if (upcase) {
	    *p++ = 'A' + tmp - 10;
	} else {
	    *p++ = 'a' + tmp - 10;
	}
	u /= base;
    } while (u != 0);

    if (negFlag) {
	*p++ = '-';
    }

    /* figure out actual length and adjust the maximum length */
    actualLength = p - buf;
    if (length < actualLength) length = actualLength;

    /* add padding */
    if (ladjust) {
	padc = ' ';
    }
    if (negFlag && !ladjust && (padc == '0')) {
	for (i = actualLength-1; i< length-1; i++) buf[i] = padc;
	buf[length -1] = '-';
    } else {
	for (i = actualLength; i< length; i++) buf[i] = padc;
    }
	    

    /* prepare to reverse the string */
    {
	int begin = 0;
	int end;
	if (ladjust) {
	    end = actualLength - 1;
	} else {
	    end = length -1;
	}

	while (end > begin) {
	    char tmp = buf[begin];
	    buf[begin] = buf[end];
	    buf[end] = tmp;
	    begin ++;
	    end --;
	}
    }

    /* adjust the string pointer */
    return length;
}
