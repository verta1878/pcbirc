/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/* The source code in this module is proprietary software belonging to       */
/* Clark Development Company and is part of the PCBoard source code library. */
/* You are granted the right to use this source code for the building of any */
/* of the PCBoard products you have licensed.  Any other usage is forbidden  */
/* without prior written consent from Clark Development Company, Inc.        */
/*                                                                           */
/* Be sure to read the source code license agreement before utilizing any    */
/* of the source code found herein.                                          */
/*                                                                           */
/* Copyright (C) 1996  Clark Development Company, Inc.  All Rights Reserved. */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/


#ifdef FIDO

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "misc.h"
//#include "bug.h"
#include "pcb.h"
#include "pcboard.h"
//#include "pcboard.ext"
//#include "messages.h"
#include "screen.h"
//#include "users.h"

#include "defines.h"
#include "structs.h"
#include "prototyp.h"
#include "tossmisc.h"
//#include "times.h"
//#include "byte.h"

#include "seenby.hpp"

#ifdef DEBUG
#include <memcheck.h>
#endif


cSEENBY::cSEENBY(char *seenByList[], char *addr,char * sendee) : seenbylen(0),numElm(0)
{
	const int maxSize = 4000;
	list = NULL;
	dupe	= FALSE;
	list = new nodeInf[maxSize];
	if(!list)
	{
	   writeFidolog("Could not allocate memory for seen-by processing.",BLOCK);
	   return;
	}

	memset(list,0,sizeof(nodeInf)*maxSize);
	memset(&newAddr,0,sizeof(newAddr));
	numElm	= 0;
	parseList(seenByList);
	if(isDupe(sendee))
		 dupe = TRUE;
	else
	{
		dupe = FALSE;
		addList(addr);
		sort();
		createNewList(seenByList);
	}
}

cSEENBY::cSEENBY(void)
{
	memset(&list,0,sizeof(list));
	memset(&newAddr,0,sizeof(newAddr));
	dupe	= FALSE;
}

void cSEENBY::parseList(char *seenByList[])
{
	unsigned int   currentNet=0, currentNode=0;
	int   i=0;
	char *ptr1;
	char *ptr2;


	numElm = 0;
	// Process the array of pointers
	while(seenByList[i] != NULL)
	{
		// Set pointer to start of line
		ptr1 = seenByList[i];

		while(*ptr1 != '\x0')
		{
			// Skip to the first number
			while(!isdigit(*ptr1) && *ptr1 != '\x0')
			{
			  ptr1++;
			}
			if(*ptr1 == '\x0') continue;

			ptr2 = ptr1;

			// Skip to end of number
			while(isdigit(*ptr2))  ptr2++;

			// If the next char is a \ then get new net number and new zone number
			if(*ptr2 == '/')
			{
				*ptr2 = '\x0';
				currentNet = atoi(ptr1);
				*ptr2 = '/';
				ptr1 = ++ptr2;
				while(isdigit(*ptr2) && *ptr2 != '\x0') ptr2++;
			}
			currentNode = atoi(ptr1);
			ptr1 = ptr2;


			list[numElm].net  = currentNet;
			list[numElm++].node = currentNode;
		}
	  i++;
	}
}


void cSEENBY::createNewList(char *oldList[])
{
int i=0,j=0;
char tbuf[15];
unsigned int currNet = 0;
bool newLine = FALSE;

   while(1)
   {
		if(oldList[i] == NULL)
		  {
			if((oldList[i]=(char *)malloc(100))==NULL)
			  return;
		  }

		strcpy(oldList[i],"SEEN-BY:");

		while(strlen(oldList[i]) <= (80-12))
		{
			if(list[j].net > currNet || newLine)
			{
				newLine = FALSE;
				currNet = list[j].net;
				strcat(oldList[i]," ");
				strcat(oldList[i],itoa(list[j].net,tbuf,10));
				strcat(oldList[i],"/");
				strcat(oldList[i],itoa(list[j++].node,tbuf,10));
			}
			else if(list[j].net == currNet)
			{
				strcat(oldList[i]," ");
				strcat(oldList[i],itoa(list[j++].node,tbuf,10));
			}


			if(j >= numElm) return;
		}
		i++;
		newLine = TRUE;
   }
}

bool cSEENBY::isDupe(char *addr)
{
nodeInf   inf;
uint	  zone,point;

  fido_nodestr_to_int(addr,zone,inf.net,inf.node,point);
  return(find(inf));
}

bool cSEENBY::isDupe(void)
{
  return dupe;
}

bool cSEENBY::addAddr(char *addr)
{
char	*ptr1;
char	*ptr2;

  ptr1 = addr;
  ptr2 = strchr(ptr1,'/');

  if(ptr2)
	*ptr2 = '\x0';
  else
	return FALSE;

  newAddr.net = atoi(ptr1);
  *ptr2 = '/';
  ptr1 = ++ptr2;
  while(isdigit(*ptr2)) ptr2++;
  *ptr2 = '\x0';
  newAddr.node = atoi(ptr1);

  if(find(newAddr) == FALSE)
	{
	  list[numElm].net	= newAddr.net;
	  list[numElm++].node = newAddr.node;
	}
	//else
	//	dupe = TRUE;
  return(TRUE);
}

bool cSEENBY::addList(char *list)
{
char	*tptr;
char	*ptr;
bool	first;

  first=TRUE;
  ptr=list;

  while(1)
	{
			tptr=++ptr;
	  if((ptr=strchr(tptr,' '))!=NULL)
		{
		  first=FALSE;
		  *ptr=NULL;
		  addAddr(tptr);
					*ptr=' ';
				}
	  else if(first==TRUE)
		{
		  first=FALSE;
		  addAddr(tptr);
		}
	  else
		{
		  addAddr(tptr);
		  break;
		}
		}
  return(TRUE);
}

void cSEENBY::print(void)
{
   for(int i=0;i<24;i++)
	 printf("\nNet = %d Node = %d",list[i].net,list[i].node);
}

void cSEENBY::makeNode(const char *  str, nodeInf & node)
{
	char * tptr = NULL;

	tptr = strchr((char *)str,':');
	if(tptr) node.node = atoi(++tptr);
	else return;

	tptr = strchr((char *)str,'/');
	if(tptr) node.net = atoi(++tptr);
	else return;

}

bool cSEENBY::find(nodeInf &node)
{
	//if(bsearch(&node,list,size_t(numElm+1),sizeof(nodeInf),compare) != 0)
	if(bsearch(&node,list,size_t(numElm),sizeof(nodeInf),compare) != 0)
	  return TRUE;
	else
	  //dupe = FALSE;
	return (FALSE);
}

int cSEENBY::compare(const void *pkey,const void *pelm)
{
//	  long * ll = (long *) pkey;
//	  long * rl = (long *) pelm;
//	  return (*ll - *rl);

nodeInf * key, * elm;
   key = (nodeInf *)pkey;
   elm = (nodeInf *)pelm;

   if(key->net == elm->net && key->node == elm->node) return 0;
   if(key->net < elm->net) return -1;
   if(key->net > elm->net) return 1;
   if(key->net == elm->net && key->node < elm->node) return -1;
   if(key->net == elm->net && key->node > elm->node) return 1;
 return 0;
}

void cSEENBY::sort(void)
{
  qsort(list,size_t(numElm),sizeof(list[0]),compare);
}

#endif
