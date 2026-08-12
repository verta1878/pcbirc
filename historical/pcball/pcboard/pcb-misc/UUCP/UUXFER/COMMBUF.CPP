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



#include    <mem.h>

#include    <constrea.h>
extern constream conerr;

#include    <commbuf.hpp>

// Largest UUCP data buffer is 4K, so make local buffer 5K just to be safe
static char   localCommBuf [ 5 * 1024 ];

// Buffer is initially empty
static int    localCommSize = 0;

// Pointer into buffer for next request address
static char * localCommPtr = localCommBuf;

// Pointer waiting is initialally empty too
static int    localCommWaiting = 0;

static void near pascal fillCommBuf ( void )
{
    // Check on the status of the comm buffer
    if (localCommSize < sizeof(localCommBuf))
    {
     // conerr << "localCommSize = (" << localCommSize << ',';
        int bytes = cgetbuf(localCommBuf+localCommSize,
            sizeof(localCommBuf)-localCommSize);
        localCommSize += bytes;
        localCommWaiting += bytes;
     // conerr << localCommSize << ')' << endl;
    }
}

//int pascal getCommBuf ( void * buf, int size )
//{
// // conerr << "getCommBuf(" << buf << ',' << size << ')' << endl;
//
//    fillCommBuf();
//
//    // If insufficient bytes, or the buffer isn't defined,
//    // or size isn't positive, error
//    if ((localCommSize < size) || (buf == NULL) || (size <= 0))
//    {
//        giveup();
//     // conerr << "returning no buffer from getCommBuf" << endl;
//        return -1;
//    }
//
//    // Copy as much as usable into the buffer
//    memcpy(buf,localCommBuf,size);
// // conerr << "copied " << size << " bytes from localCommBuf to buf" << endl;
//
//    // If there is extra in the local buffer, move it down
//    if ((localCommSize-size) > 0)
//    {
//        memcpy(localCommBuf,localCommBuf+size,
//            localCommSize-size);
//     // conerr << "shifted " << (localCommSize-size) <<
//     //     " bytes down in localCommBuf" << endl;
//    }
//
//    // Now adjust sizes based on amount used
// // conerr << "localCommSize = (" << localCommSize << ',';
//    localCommSize -= size;
// // conerr << localCommSize << ')' << endl;
//
//    // We're done!
// // conerr << "returning buffer from getCommBuf" << endl;
//    return 0;
//}

//int pascal ungetCommBuf ( void * buf, int size, int off )
//{
// // conerr << "ungetCommBuf(" << buf << ',' << size << ',' << off << ')' <<
// //     endl;
//
//    // If the buffer isn't defined, size isn't positive, or
//    // the offset is bigger than size, error
//    if ((buf == NULL) || (size <= 0) || (off > size))
//    {
//     // conerr << "returning unsuccessful from ungetCommBuf" << endl;
//        return -1;
//    }
//
//    // If there is local buffer data, move it up to
//    // make room for the data we're putting back
//    if (localCommSize > 0)
//    {
//        memmove(localCommBuf+(size-off),localCommBuf,localCommSize);
//     // conerr << "shifted " << (localCommSize-size) <<
//     //     " bytes up in localCommBuf" << endl;
//    }
//
//    // Adjust the local buffer size used
// // conerr << "localCommSize = (" << localCommSize << ',';
//    localCommSize += size-off;
// // conerr << localCommSize << ')' << endl;
//
//    // Copy in bytes we need to put back
//    memcpy(localCommBuf,((char*)buf)+off,size-off);
// // conerr << "copied " << size << " bytes from buf to localCommBuf" << endl;
//
//    // We're done!
// // conerr << "returning successful from ungetCommBuf" << endl;
//    return 0;
//}

void * pascal getCommPtr ( int size )
{
 // conerr << "getCommPtr(" << size << ')' << endl;

    fillCommBuf();

    // If insufficient bytes or size isn't positive, error
    if ((localCommWaiting < size) || (size <= 0))
    {
        giveup();
     // conerr << "returning no pointer from getCommPtr" << endl;
        return NULL;
    }

    // Get pointer
    void * ptr = localCommPtr;
    localCommPtr += size;
 // conerr << "got pointer to " << size << " bytes from localCommPtr" << endl;

    // Now adjust sizes based on amount used
 // conerr << "localCommWaiting = (" << localCommWaiting << ',';
    localCommWaiting -= size;
 // conerr << localCommWaiting << ')' << endl;

    // We're done!
 // conerr << "returning pointer from getCommPtr" << endl;
    return ptr;
}

void pascal ungetCommPtr ( int off )
{
    localCommPtr = localCommBuf+off;
    localCommWaiting = localCommSize-off;
}

void pascal syncCommPtr ( void )
{
    memcpy(localCommBuf,localCommPtr,localCommWaiting);
    localCommPtr = localCommBuf;
    localCommSize = localCommWaiting;
}

//inline int minimum ( int a, int b )
//{
//    return ((a < b) ? a : b);
//}
//
//void pascal putCommBuf ( void * buf, int size )
//{
//    const int sendBufSize = 1024;
//    for (int i = 0; i < size; i += sendBufSize)
//        sendstr(((char*)buf)+i,minimum(size-i,sendBufSize));
//}

