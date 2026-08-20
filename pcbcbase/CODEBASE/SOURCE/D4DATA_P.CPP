/* d4data.cpp (c)Copyright Sequiter Software Inc., 1990-1993.  All rights reserved. */

#include "d4all.h"
#pragma hdrstop

#include "d4data.hpp"

Code4::Code4()
{
   d4init( this ) ;
   memset( string_types, 0, sizeof(string_types) ) ;
}

Data4 Code4::data( char *alias )
{
   Data4 temp ;
   temp.data =  d4data( this, alias ) ;
   return temp ;
}

Index4 Data4::index( char *name )
{
   Index4 temp ;
   temp.index =  i4open( data, name ) ;
   return temp ;
}

Data4 Field4::data()
{
   Data4 temp ;
   temp.data = field->data ;
   return temp ;
}

Field4memo::Field4memo( Data4& data, char *name ) : Field4( data, name )
{
}

Field4memo::Field4memo( Data4& data, int j ) : Field4( data, j )
{
}

void Field4memo::changed()
{
   field->data->record_changed = 1 ;
   if ( field->memo != 0 )
      field->memo->is_changed =  1 ;
}
