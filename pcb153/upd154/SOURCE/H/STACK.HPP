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


/****************************************************************************/
/*                                                                          */
/*                                STACK.HPP                                 */
/*                                                                          */
/*--------------------------------------------------------------------------*/
/*                                                                          */
/*                   A generic template based stack class.                  */
/*                                                                          */
/*==========================================================================*/
/*                                                                          */
/*                                Written by                                */
/*                            Scott Dale Robison                            */
/*                                                                          */
/*--------------------------------------------------------------------------*/
/*                                                                          */
/*           Copyright (C) 1993 - Clark Development Company, Inc.           */
/*                                                                          */
/****************************************************************************/

#ifndef    ___STACK_HPP___

#define    ___STACK_HPP___

/****************************************************************************/

// Pragmas

/****************************************************************************/

// Included Files

#include "list.hpp"

/****************************************************************************/

// Defined Macros

/****************************************************************************/

// Types

template <class T>
class cSTACK : public cLIST<T>
{

    public:

        int LIBENTRY push ( T data );
        T   LIBENTRY pop  ( void );
        T   LIBENTRY top  ( void );

};

    /*------------------------------------------------------------------*/

template <class T>
class cISTACK : public cILIST<T>
{

    public:

        cISTACK(int _cdd, int _ap) :
            cILIST<T>(_cdd,_ap)
        { }

        int LIBENTRY push (T * data);
        T * LIBENTRY pop  (void);
        T * LIBENTRY top  (void);

};

/****************************************************************************/

// Constants

/****************************************************************************/

// Variables

/****************************************************************************/

// Function Prototypes

/****************************************************************************/

// Inline Functions

/****************************************************************************/

// Template Functions

template <class T>
int LIBENTRY cSTACK<T>::push(T data)
{
    return add(data);
}

    /*------------------------------------------------------------------*/

template <class T>
T LIBENTRY cSTACK<T>::pop(void)
{
    T tmp;
    if (head != NULL)
    {
        tmp = head->data;
        cNODE<T> * ptr = head;
        head = head->next;
        delete ptr;
    }
    return tmp;
}

    /*------------------------------------------------------------------*/

template <class T>
T LIBENTRY cSTACK<T>::top(void)
{
    return head->data;
}

    /*------------------------------------------------------------------*/

template <class T>
int LIBENTRY cISTACK<T>::push(T * data)
{
    return add(data);
}

    /*------------------------------------------------------------------*/

template <class T>
T * LIBENTRY cISTACK<T>::pop(void)
{
    T * tmp = NULL;
    if (head != NULL)
    {
        tmp = head->data;
        cNODE<T *> * ptr = head;
        head = head->next;
        delete ptr;
    }
    return tmp;
}

    /*------------------------------------------------------------------*/

template <class T>
T * LIBENTRY cISTACK<T>::top(void)
{
    return head->data;
}

/****************************************************************************/

// Test Code

#ifdef    ___TEST___

//typedef cSTACK<int>  tINTSTK;
//typedef cISTACK<int> tIINTSTK;

//tINTSTK  intStk;
//tIINTSTK iIntStk(DD_DEF);

struct s
{
    int a;

    int operator == (s & S) { return a == S.a; }
    int operator != (s & S) { return !(*this == S); }
    int operator <  (s & S) { return a < S.a; }
    int operator <= (s & S) { return (*this < S) || (*this == S); }
    int operator >  (s & S) { return a > S.a; }
    int operator >= (s & S) { return (*this > S) || (*this == S); }

};

//typedef cSTACK<s>  tSSTK;
typedef cISTACK<s> tISSTK;

//tSSTK  sStk;
tISSTK iSStk(DD_DEF);

#endif

/****************************************************************************/

#endif
