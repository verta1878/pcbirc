#include <stddef.h>
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


/*
bool gen_seenby(char * arr[], char * str)
{

    netnode  * hr     = NULL;
    int    i          = 0;
    int    bytesAdded = 0;
    int    currNet    = 0;
    char   buf[8];
    char   *ptr;
    int firstOneAdded = 0;
    int        PCBnet = 0;
    int       PCBnode = 0;
    char       buffer[50],tmp[50];
    size_t      size;

    memset(buffer,0,sizeof(buffer));
    memset(tmp   ,0,sizeof(tmp));
    assert(stackavail() > 2000);

    maxstrcpy(buffer,UsersData.Name,sizeof(buffer)-1);
    size=(sizeof(buffer)-6);
    memcpy(tmp,buffer+6,size);
    fido_nodestr_to_int(tmp,NULL,&PCBnet,&PCBnode,NULL);

    while(arr[i] != NULL)
    {
        if(addto_seenby(&hr,arr[i],TRUE,PCBnet,PCBnode)==TRUE)
        {
          free_list(hr);
          hr=NULL;
          return TRUE;
        }
        i++;
    }

    if(addto_seenby(&hr,str,FALSE,PCBnet,PCBnode) == TRUE)
    {
      free_list(hr);
      return TRUE;
    }
    i = 0;

    if(arr[i]!=NULL)
      addlineheader(arr[i]);
    else
      {
        free_list(hr);
        return(TRUE);
      }

    bytesAdded = HDR_SIZE;

    do
    {
        currNet = get_text(hr,buf);
        if((bytesAdded + strlen(buf)) < LINE_LEN-3)
        {
          if(strchr(buf,'/') && firstOneAdded)
            continue;
          strcat(arr[i],buf);
          bytesAdded += strlen(buf);
          firstOneAdded = 0;
        }
        else
        {
          if(*(arr[i]+bytesAdded-1)=='/')
            {
              ptr=arr[i]+bytesAdded;
              while(*ptr!=0x20)
                ptr--;
              *ptr=NULL;
            }
          strcat(arr[i],"\x0D\0");
          i++;
          bytesAdded = 0;
          if(arr[i] == NULL)
             arr[i] = (char *) malloc(LINE_LEN);

          if(arr[i]!=NULL)
            addlineheader(arr[i]);
          else
            {
              free_list(hr);
              return(FALSE);
            }

          bytesAdded = HDR_SIZE;
          itoa(currNet,buf,10);
          strcat(arr[i],buf);
          bytesAdded += strlen(buf);
          *(arr[i]+bytesAdded) = '/';
          *(arr[i]+(++bytesAdded)) = '\x0';
          firstOneAdded = 1;
        }
    }while(currNet != 0);
    i = 0;

    puts("\n");
    while(arr[i] != NULL)
    {
      puts(arr[i]);
      i++;
    }
    free_list(hr);
    return FALSE;
}


int get_text(netnode * hr,char * str)
{
    static netnode  * netPtr  = NULL;
    static nodenode * nodePtr = NULL;
    static node = 0;

  assert(stackavail() > 2000);
    if(netPtr == NULL)
    {
        netPtr = hr;
        nodePtr = netPtr->nodelist;
    }

    if(!node)
    {
      itoa(netPtr->net,str,10);
      strcat(str,"/");
      node = 1;
    }
    else
    {
        itoa(nodePtr->node,str,10);
        strcat(str," ");
        nodePtr = nodePtr->next;
        if(nodePtr == NULL)
        {
            node = 0;
            netPtr = netPtr->next;
            if(netPtr)
              nodePtr = netPtr->nodelist;
            else
              return FALSE;
        }
    }
    return netPtr->net;
}

void addlineheader(char * str)
{
    char header[] = {"SEEN-BY: "};
    memset(str,'\x0',LINE_LEN);
    strcpy(str,header);
}

int addto_seenby(netnode **theHR, char *str, int abortOnDupe,int PCBnet,int PCBnode)
{
/*********
 Declarations
 *********/

char       *chPtr1  = NULL;
char       *chPtr2  = NULL;
char       chPtr3   = NULL;
char       *isolPtr1= NULL;
char       *isolPtr2= NULL;
char       *isolPtr3= NULL;
netnode    *tnptr   = NULL;
nodenode   *tmpnode = NULL;
int         lastOne = 0;

/********
 Code
 ********/
  assert(stackavail() > 2000);

   isolPtr1 = str;
   while(!isdigit(*isolPtr1))
     isolPtr1++;
   while(!lastOne)
   {
      isolPtr2 = strchr(isolPtr1,'/');
      if(isolPtr2)
        isolPtr3 = strchr(isolPtr2+1,'/');
      if(!isolPtr3)
        lastOne = 1;
      else
      {
        while(*isolPtr3 != ' ' && *isolPtr3 != '\x0')
          isolPtr3--;
        *isolPtr3 = '\x0';
      }

      /* Build a linked list from str */
      chPtr1 = isolPtr1;
      chPtr2 = strchr(chPtr1,'/');   /* Isolate the net number */
      if(chPtr2)
      {
        *chPtr2 = '\x0';
        tnptr = (netnode *) malloc(sizeof(netnode));     /* Create a net node */
        tnptr->net = atoi(chPtr1);
        tnptr->next = NULL;
        tnptr->nodelist = NULL;
        *chPtr2 = '/';
        chPtr1 = ++chPtr2;

        /* Start processing nodes for the current net */
        while(*chPtr2 != NULL)
        {
            /* Isolate node number */
            while(*chPtr2 != ' ' && *chPtr2 != '\x0')
              ++chPtr2;

            chPtr3 = *chPtr2;
            *chPtr2 = '\x0';

            /* Create a node node */
            if((tmpnode=(nodenode *)malloc(sizeof(nodenode)))==NULL)
              {
                free_list(*theHR);
                free(tnptr);
                return(TRUE);
              }
            tmpnode->node = atoi(chPtr1);
            tmpnode->next = NULL;
            *chPtr2 = chPtr3;

            /* Add node to list */
            if(insertnode(&tnptr->nodelist,tmpnode,tnptr->net,abortOnDupe,PCBnet,PCBnode)==TRUE)
            {
              free(tnptr);
              return TRUE;
            }

            /* Move on to next node number */
            while(*chPtr2 == ' ')
              chPtr2++;

            /* get ready to start over */
            chPtr1 = chPtr2;
        }

        /* Insert Net node */
//        if((*theHR) == NULL)
//          (*theHR) = tnptr;
//        else
          if(insertnet(theHR,tnptr,PCBnet,PCBnode) == TRUE)
            return TRUE;

       chPtr1 = isolPtr1 = ++isolPtr3;
      }
    }
  return FALSE;
}

int insertnode(nodenode * *hr, nodenode * node, int currNet,int abortOnDupe,int PCBnet,int PCBnode)
{

/*********
 Declarations
 *********/
nodenode  * currnode = NULL;
nodenode  * tmpnode  = NULL;

/*********
  Initializations
 *********/

  assert(stackavail() > 2000);
  currnode = *hr;                /* Set current node to head record in list */

    /* Check for dupes */

    while(currnode != NULL)
    {
      if(currnode->node == node->node)
        {
          if(abortOnDupe == TRUE && currnode->node == PCBnode && currNet == PCBnet)
            {
              if(node!=NULL)
                {
                  free(node);
                  node=NULL;
                }
              return abortOnDupe;
            }
          if(node!=NULL)
            {
              free(node);
              node=NULL;
            }
          return FALSE;
        }
      currnode = currnode->next;
    }

    currnode = *hr;

    if(currnode==NULL)
      {
        if(node->node == PCBnode && currNet == PCBnet && abortOnDupe == TRUE)
          {
            if(node!=NULL)
              {
                free(node);
                node=NULL;
              }
            if(abortOnDupe==TRUE)
              return TRUE;
            else
              return FALSE;
          }
        *hr=node;
        node->next=NULL;
        return FALSE;
      }

    currnode = *hr;
    /* Handle second node case */
    if((*hr)->next == NULL)
    {
        if(node->node > (*hr)->node)
        {
            (*hr)->next = node;
            node->next = NULL;
            return FALSE;
        }
        if(node->node < (*hr)->node)
        {
          node->next = (*hr);
          (*hr) = node;
          return FALSE;
        }
    }

    if(node->node < currnode->node && currnode == *hr)
    {
        node->next = *hr;
        *hr = node;
        return FALSE;
    }

    /* Handle middle node case */
    while(currnode->next != NULL)
    {

        tmpnode = currnode->next;
        if( (currnode->node < node->node) && (tmpnode->node > node->node))
        {
            node->next = currnode->next;
            currnode->next = node;
            return FALSE;
        }
        currnode = currnode->next;
    }

    /* Handle last node case */
    currnode->next = node;
    node->next = NULL;
    return FALSE;
}

int insertnet(netnode  * * hr, netnode * net,int PCBnet,int PCBnode)
{


/*********
 Declarations
 *********/
netnode  * currnode = NULL;
netnode  * tmpnet   = NULL;
int abortFlag =          FALSE;


/*********
  Initializations
 *********/

  assert(stackavail() > 2000);

    currnode = *hr;                   /* Set current node to head record in list */
    if(net->nodelist == NULL)
    {
      free(net);
      net=NULL;
      return abortFlag;
    }

    /* Check for Dupes */

    while(currnode != NULL)
    {
        if(net->net == currnode->net)
          {
            nodenode * tmp, * tmp2;
            tmp = net->nodelist;
            while(tmp != NULL)
            {
              tmp2 = tmp->next;
              if(insertnode(&currnode->nodelist,tmp,currnode->net,TRUE,PCBnet,PCBnode)== TRUE)
                abortFlag = TRUE;
              tmp = tmp2;
            }
            if(net!=NULL)
              {
                free(net);
                net=NULL;
                return abortFlag;
              }
          }
        currnode = currnode->next;
    }

     currnode = *hr;
    /* Handle first node case */
    if(*hr == NULL)
    {
      *hr = net;
       return FALSE;
    }
    /* Handle second node case */
    if((*hr)->next == NULL)
    {
        if(net->net > (*hr)->net)
        {
            (*hr)->next = net;
            net->next = NULL;
            return abortFlag;
        }
        if(net->net < (*hr)->net)
        {
          net->next = (*hr);
          (*hr) = net;
          return abortFlag;
        }

        /* Handle == */
        if(currnode->net == net->net)
        {
          nodenode * tmpnode,* tmpnode2;
          tmpnode = net->nodelist;
          while(tmpnode != NULL)
          {
            tmpnode2 = tmpnode->next;
            insertnode(&currnode->nodelist,tmpnode,currnode->net,FALSE,-1,-1);
            tmpnode = tmpnode2;
          }
          free(net);
          net = NULL;
          return abortFlag;
        }
    }

    if(net->net < currnode->net && currnode == *hr)
    {
        net->next = *hr;
        *hr = net;
        return abortFlag;
    }

    /* Handle middle net case */
    while(currnode->next != NULL)
    {

        tmpnet = currnode->next;
        if( (currnode->net < net->net) && (tmpnet->net > net->net))
        {
            net->next = currnode->next;
            currnode->next = net;
            return abortFlag;
        }
        currnode = currnode->next;
    }

    /* Handle last net case */
    currnode->next = net;
    net->next = NULL;

    /* Handle == */
    if(currnode->net == net->net)
    {
      free(net);
      net = NULL;
    }
    return abortFlag;

}

void free_list(netnode * hr)
{
    netnode  * netPtr   = NULL;
    nodenode * nodePtr  = NULL;
    nodenode * freeNode = NULL;
    netnode  * freeNet  = NULL;

  assert(stackavail() > 2000);
          netPtr = hr;
          while(netPtr != NULL)
          {
            nodePtr=netPtr->nodelist;
            while(nodePtr != NULL)
            {
              freeNode = nodePtr;
              nodePtr = nodePtr->next;
              free(freeNode);
            }
            freeNet = netPtr;
            netPtr  = netPtr->next;
            free(freeNet);
          }
}

*/
