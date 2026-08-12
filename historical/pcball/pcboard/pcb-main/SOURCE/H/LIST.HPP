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


/******************************************************************************/
/*                                                                            */
/*                                  LIST.HPP                                  */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/*                                                                            */
/*                    A generic template based list class.                    */
/*                                                                            */
/*============================================================================*/
/*                                                                            */
/*                                 Written by                                 */
/*                             Scott Dale Robison                             */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/*                                                                            */
/*            Copyright (C) 1993 - Clark Development Company, Inc.            */
/*                                                                            */
/******************************************************************************/

#ifndef ___LIST_HPP___

#define ___LIST_HPP___

/******************************************************************************/

// Pragmas

/******************************************************************************/

// Included Files

#include    <stddef.h>
#include    <types.hpp>

/******************************************************************************/

// Defined Macros

#define DD_NO       0
#define DD_DEF      1
#define DD_YES      2

#define CACHESIZE   16

/******************************************************************************/

// Types

template <class T>
class cNODE
{

    public:

        cNODE(T & _data, cNODE<T> * _next) :
            data(_data),
            next(_next)
        { }

        T          data;
        cNODE<T> * next;

};

    /*--------------------------------------------------------------------*/

template <class T>
class cLIST
{

    public:

        cLIST(void);

        virtual ~cLIST(void)
        {
            flush();
        }

        int  LIBENTRY isEmpty   (void)
        {
            return (head == NULL);
        }

        void LIBENTRY flush     (void);
        int  LIBENTRY add       (T data);
        int  LIBENTRY detach    (T data);
        T *  LIBENTRY firstThat (int LIBENTRY (* match)(const T & obj, const void * data), const void * data);
        T *  LIBENTRY lastThat  (int LIBENTRY (* match)(const T & obj, const void * data), const void * data);
        void LIBENTRY forEach   (void LIBENTRY (* func)(T & obj));

    protected:

        cNODE<T> * head;
        cNODE<T> * Cache[CACHESIZE];

};

    /*--------------------------------------------------------------------*/

template <class T>
class cILIST : public cLIST<T *>
{

    public:

        cILIST(int _cdd, int _ap) :
            cLIST<T *>(),
            cdd(_cdd),
            arrPtr(_ap)
        { }

        virtual ~cILIST(void)
        {
            flush();
        }

        void LIBENTRY flush     (int dd = DD_DEF);
        int  LIBENTRY add       (T * data);
        T *  LIBENTRY firstThat (int LIBENTRY (* match)(const T & obj, const void * data), const void * data);
        T *  LIBENTRY lastThat  (int LIBENTRY (* match)(const T & obj, const void * data), const void * data);
        void LIBENTRY forEach   (void LIBENTRY (* func)(T & obj));

    protected:

        int cdd;
        int arrPtr;

};

/******************************************************************************/

// Constants

/******************************************************************************/

// Variables

/******************************************************************************/

// Function Prototypes

/******************************************************************************/

// Inline Functions

/******************************************************************************/

// Template Functions

template <class T>
cLIST<T>::cLIST(void) : head(NULL)
{
    for (int i = 0; i < CACHESIZE; ++i) Cache[i] = NULL;
}

    /*--------------------------------------------------------------------*/

template <class T>
void LIBENTRY cLIST<T>::flush(void)
{
    while (head != NULL)
    {
        cNODE<T> * ptr = head;
        head = head->next;
        delete ptr;
    }
}

    /*--------------------------------------------------------------------*/

template <class T>
int LIBENTRY cLIST<T>::add(T data)
{

    cNODE<T> * ptr = new cNODE<T>(data,head);
    if (ptr == NULL) return -1;
    head = ptr;
    return 0;
}

    /*--------------------------------------------------------------------*/

template <class T>
int LIBENTRY cLIST<T>::detach(T data)
{
    cNODE<T> * prev = NULL;
    cNODE<T> * cur  = head;

    while ((cur->data != data) && (cur != NULL))
    {
        prev = cur;
        cur  = cur->next;
    }

    if (cur != NULL)
    {
        if (prev != NULL)
            prev->next = cur->next;
        else
            head = cur->next;
        delete cur;
        return 0;
    }

    return -1;
}

    /*--------------------------------------------------------------------*/

template <class T>
T * LIBENTRY cLIST<T>::firstThat(int LIBENTRY (* match)(const T & obj, const void * data),
    const void * data)
{
    cNODE<T> * node;

//    for (int X = 0; X < CACHESIZE; X++)
//        if (match(Cache[X]->data,data))
//            return(&(Cache[X]->data));

    for (node = head; node != NULL; node = node->next)
//    {
        if (match(node->data,data))
//        {
//            for (X = CACHESIZE-1; X > 0; X--)
//                Cache[X] = Cache[X-1];
//            Cache[0] = node;
            return(&(node->data));
//        }
//    }

    return(NULL);

//    for (node = head; node != NULL; node = node->next)
//        if (match(node->data,data))
//            return(&(node->data));
//
//    return(NULL);
}

    /*--------------------------------------------------------------------*/

template <class T>
T * LIBENTRY cLIST<T>::lastThat(int LIBENTRY (* match)(const T & obj,
    const void * data),
    const void * data)
{
    cNODE<T> * node;

    for (node = head; node != NULL; node = node->next)
        if (match(node->data,data))
            return(&(node->data));

    return(NULL);
}

    /*--------------------------------------------------------------------*/

template <class T>
void LIBENTRY cLIST<T>::forEach(void LIBENTRY (* func)(T & obj))
{
    cNODE<T> * node = head;
    while (node != NULL)
    {
        func(node->data);
        node = node->next;
    }
}

    /*--------------------------------------------------------------------*/

template <class T>
void LIBENTRY cILIST<T>::flush(int dd)
{
    while (head != NULL)
    {
        cNODE<T *> * ptr = head;
        head = head->next;
        if ((ptr->data != NULL) &&
            (((dd == DD_YES) && (cdd != DD_NO)) ||
            ((dd == DD_DEF) && (cdd == DD_YES))))
        {
            if (arrPtr)
                delete [] ptr->data;
            else
                delete ptr->data;
        }
        delete ptr;
    }
}

    /*--------------------------------------------------------------------*/

template <class T>
int LIBENTRY cILIST<T>::add(T * data)
{
    return cLIST<T *>::add(data);
}

    /*--------------------------------------------------------------------*/

template <class T>
T * LIBENTRY cILIST<T>::firstThat(int LIBENTRY (* match)(const T & obj, const void * data),
    const void * data)
{
    cNODE<T *> * node;
    int          X;

    for (X = 0; X < CACHESIZE; X++)
        if (match(*Cache[X]->data,data))
            return(&(*Cache[X]->data));

    for (node = head; node != NULL; node = node->next)
    {
        if (match(*node->data,data))
        {
            for (X = CACHESIZE-1; X > 0; X--)
                Cache[X] = Cache[X-1];
            Cache[0] = node;
            return(&(*node->data));
        }
    }

    return(NULL);
}

    /*--------------------------------------------------------------------*/

template <class T>
T * LIBENTRY cILIST<T>::lastThat(int LIBENTRY (* match)(const T & obj,
    const void * data),
    const void * data)
{
    T * ptr = NULL;
    cNODE<T *> * node = head;
    while (node != NULL)
    {
        if (match(*node->data,data)) ptr = node->data;
        node = node->next;
    }
    return ptr;
}

    /*--------------------------------------------------------------------*/

template <class T>
void LIBENTRY cILIST<T>::forEach(void LIBENTRY (* func)(T & obj))
{
    cNODE<T *> * node = head;
    while (node != NULL)
    {
        func(*node->data);
        node = node->next;
    }
}

/******************************************************************************/

// Test Code

#ifdef    ___TEST___

typedef  cLIST<int>  tINTLST;
typedef  cILIST<int> tINTILST;

tINTLST  intLst;
tINTILST intILst(DD_DEF);

#endif

/******************************************************************************/

#endif

