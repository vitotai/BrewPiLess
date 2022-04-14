/**********************************************************************
 BrewManiac
 created by Vito Tai
 Copyright (C) 2015 Vito Tai

 This soft ware is provided as-is. Use at your own risks.
 You are free to modify and distribute this software without removing
 this statement.
 BrewManiac by Vito Tai is licensed under a Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
***********************************************************************/

#include "mystrlib.h"

int sprintIntDigit(char *buff, int number,int base)
{

    int indx=0;
    while (base > 1)
    {
       	int digit= number /base;
       	buff[indx++]=('0' + digit);
       	number = number - digit  * base;
    	base = base /10;

    }
	  buff[indx++]=('0' + number);
   	return indx;
}

int sprintInt(char *buff,int number)
{
	int sign=0;
	if(number <0)
	{
		*buff='-';
		buff = buff +1;
		number = 0- number;
		sign=1;
	}
	int base=1;

  	if(number >= 10)
  	{
      int cb=number;
      while(cb > 9)
      {
         cb = cb / 10;
    	  base = base *10;
      }
 	}

 	return sign+sprintIntDigit(buff,number,base);
}


int sprintFloat(char *buff,float value,int precision)
{
    int len=0;

    if(value <0)
    {
        buff[0]='-';
	buff = buff;
	value = 0.0- value;
	len=1;
    }

    int base=1;
    float r=0.5;

    for(int p=0; p < precision; p++)
    {
        base = base * 10;
        r= r * 0.1;
    }
    float number=value+ r;


   	if (number >= 1.0)
   	{
     	  int integer=(int)number;

     	  len +=sprintInt(buff+len,integer);
     	  number -= (float)integer;


   	}
   	else
   	{
       	  buff[len]='0';
       	  len += 1;
   	}


   	if(precision == 0) return len;

       buff[len++]='.';

   	number = number * base;
   	len+=sprintIntDigit(buff+len,(int)number,base/10);
	buff[len]='\0';
   	return len;
}
