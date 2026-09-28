/* e4error.c   (c)Copyright Sequiter Software Inc., 1990-1993.  All rights reserved. */

#ifdef S4OS2PM
   #define  E4MSGBOXID 9513
   static void do_error_box( void ) ;
#endif

#include "d4all.h"

#ifdef S4VB_DOS
  #define V4ERROR  1
  #define V4SEVERE 2
#endif

#ifndef S4UNIX
   #ifdef __TURBOC__
      #pragma hdrstop
   #endif  /* __TUROBC__ */
#endif  /* S4UNIX */

#ifndef S4WINDOWS
#ifndef S4UNIX
   #include <conio.h>
#endif  /* S4UNIX */
#endif  /* not S4WINDOWS */

#ifndef S4TEST
#ifdef S4ERROR_HOOK
/* Add alternative error display code here.
   Comment out this function if you choose to define this function
   from your own separate source code file.
*/
void S4FUNCTION e4hook( CODE4 S4PTR *c4, int err_code, char *desc1, char *desc2, char *desc3 )
{
   return ;
}

#endif
#endif

#ifdef S4WINDOWS
   #ifdef __TURBOC__
      #if __TURBOC__ == 0x297     /* if Borland C++ 2.0 */
         #ifdef __cplusplus
            extern "C"{ void FAR PASCAL FatalAppExit(WORD,LPSTR) ; }
         #else
            void FAR PASCAL FatalAppExit(WORD,LPSTR) ;
         #endif  /* __cplusplus */
      #endif  /* __TUROBC__ == 0x297 */
   #endif  /* __TUROBC__ */

   #ifdef __ZTC__
      #ifdef __cplusplus
         extern "C"{ void FAR PASCAL FatalAppExit(unsigned short,LPSTR) ; }
      #else
         void FAR PASCAL FatalAppExit(unsigned short,LPSTR) ;
      #endif  /* __cplusplus */
   #endif  /* __ZTC__ */

   #ifdef _MSC_VER
      #if _MSC_VER == 600
         #ifdef __cplusplus
            extern "C"{ void FAR PASCAL FatalAppExit(WORD,LPSTR) ; }
         #else
            void FAR PASCAL FatalAppExit(WORD,LPSTR) ;
         #endif  /* __cplusplus */
      #endif  /* _MSC_VER == 600 */
   #endif  /* _MSC_VER */
#endif  /* S4WINDOWS */

#ifndef S4OFF_ERROR
#ifndef S4LANGUAGE

ERROR_DATA e4error_data[] =
{
   /* General Disk Access Errors */
   { e4create,         "Creating File" },
   { e4open,           "Opening File" },
   { e4read,           "Reading File" },
   { e4seek,           "Seeking to File Position" },
   { e4write,          "Writing to File" },
   { e4close,          "Closing File" },
   { e4remove,         "Removing File" },
   { e4lock,           "Locking File" },
   { e4unlock,         "Unlocking File" },
   { e4len,            "Determining File Length" },
   { e4len_set,        "Setting File Length" },
   { e4rename,         "Renaming File" },

   /* Database Specific Errors */
   { e4data,           "File is not a Data File" },
   { e4record_len,     "Record Length is too Large" },
   { e4field_name,     "Unrecognized Field Name" },
   { e4field_type,     "Unrecognized Field Type" },

   /* Index File Specific Errors */
   { e4index,          "Not a Correct Index File" },
   { e4entry,          "Tag Entry Missing" },
   { e4unique,         "Unique Key Error" },
   { e4tag_name,       "Tag Name not Found" },

   /* Expression Evaluation Errors */
   { e4comma_expected, "Comma or Bracket Expected" },
   { e4complete,       "Expression not Complete" },
   { e4data_name,      "Data File Name not Located" },
   { e4length_err,     "IIF() Needs Parameters of Same Length" },
   { e4not_constant,   "SUBSTR() and STR() need Constant Parameters" },
   { e4num_parms,      "Number of Parameters is Wrong" },
   { e4overflow,       "Overflow while Evaluating Expression" },
   { e4right_missing,  "Right Bracket Missing" },
   { e4type_sub,       "Sub-expression Type is Wrong" }, 
   { e4unrec_function, "Unrecognized Function" },
   { e4unrec_operator, "Unrecognized Operator" },
   { e4unrec_value,    "Unrecognized Value"} ,
   { e4unterminated,   "Unterminated String"} ,

   /* Optimization Errors */
   { e4opt,            "Optimization Error"} ,
   { e4opt_suspend,     "Optimization Removal Failure"} ,
   { e4opt_flush,      "Optimization File Flushing Failure"} ,

   /* Relation Errors */
   { e4relate,         "Relation Error"} ,
   { e4lookup_err,     "Matching Slave Record Not Located"} ,

   /* Report Errors */
   { e4report,         "Report Error"} ,

   /* Critical Errors */
   { e4memory,         "Out of Memory"} ,
   { e4info,           "Unexpected Information"} ,
   { e4parm,           "Unexpected Parameter"} ,
   { e4demo,           "Exceeded Maximum Record Number for Demonstration"} ,
   { e4result,         "Unexpected Result"} ,

   /* Not Supported Errors */
   { e4not_memo,        "Function unsupported: library compiled with S4OFF_MEMO" },
   { e4not_write,       "Function unsupported: library compiled with S4OFF_WRITE" },
   { e4not_index,       "Function unsupported: library compiled with S4OFF_INDEX" },
   { e4not_rename,      "Function unsupported: library compiled with S4NO_RENAME" },
   { 0, 0 },
} ;

#endif  /* not S4LANGUAGE */

#ifdef S4LANGUAGE
#ifdef S4GERMAN

ERROR_DATA e4error_data[] =
{
   /* Allgemeine Fehler beim Diskzugriff  (General Disk Access Errors) */
   { e4create,         "Anlegen einer Datei" },
   { e4open,           "™ffnen einer Datei" },
   { e4read,           "Lesen einer Datei" },
   { e4seek,           "Suchen einer Position in der Datei " },
   { e4write,          "Schreiben einer Datei" },
   { e4close,          "Schlieáen einer Datei" },
   { e4remove,         "L”schen einer Datei" },
   { e4lock,           "Locken einer Datei" },
   { e4unlock,         "Freigeben einer Datei" },
   { e4len,            "Festlegen der L„nge einer Datei" },
   { e4len_set,        "Einstellen der L„nge einer Datei" },
   { e4rename,         "Umnennen einer Datei" },

   /* Datenbank spezifische Fehler (Database Specific Errors) */
   { e4data,           "Datei is keiner DatenBank" },
   { e4record_len,     "Datensatzl„nge zu groá" },
   { e4field_name,     "Unbekannter Feldname" },
   { e4field_type,     "Feldtyp" },

   /* Indexdatei spezifische Fehler  (Index File Specific Errors) */
   { e4index,          "Datei is keine Indexdatei" },
   { e4entry,          "Indexdatei is veraltet" },
   { e4unique,         "Schulsel ist schon einmal vorhanden" },
   { e4tag_name,       "Name des 'Tag'"},

   /* Fehler bei der Bewertung von Ausdrcken   (Expressions Evaluation Errors) */
   { e4comma_expected, "\",\" oder \")\" erwartet" },
   { e4complete,       "Ausdruck ist nich vollst„ndig" },
   { e4data_name,      "Keine offene Datenbank" },
   { e4num_parms,      "Ungltige Anzahl von Parametern im Ausdruck"},
   { e4overflow,       "šberlauf bei der Auswertung eines Ausdrucks" },
   { e4right_missing,  "Rechte Klammer im Ausdruck fehlt" },
   { e4unrec_function, "Unbekannte Funktion im Ausdruck" },
   { e4unrec_operator, "Unbekannter Operator im Ausdruck" },
   { e4unrec_value,    "Unbekannter Wert im Ausdruck"} ,
   { e4unterminated,   "Nicht abgeschlossene Zeichenkette im Ausdruck"} ,

   /* Optimization Errors */
   { e4opt,            "Optimization Error"} ,   /*!!!GERMAN*/
   { e4opt_suspend,     "Optimization Removal Failure"} ,      /*!!!GERMAN*/
   { e4opt_flush,      "Optimization File Flushing Failure"} , /*!!!GERMAN*/

   /* Relation Errors */
   { e4lookup_err,     "Matching Slave Record Not Located"} ,

   /* Kritische Fehler  (Critical Errors) */
   { e4memory,         "Kein Speicher mehr verfgbar"} ,
   { e4info,           "Unerwartete Information" },
   { e4parm,           "Unerwarteter Parameter"},
   { e4demo,           "Exceeded Maximum Record Number for Demonstration"} , /*!!!GERMAN*/
   { e4result,         "Unerwartetes Ergebnis"},
   { 0, 0 },
} ;

#endif  /* S4GERMAN  */

#ifdef S4FRENCH

ERROR_DATA e4error_data[] =
{
   /* General Disk Access Errors */
   { e4create,         "En cr‚ant le fichier" },
   { e4open,           "En engageant le fichier" },
   { e4read,           "En lisant le fichier" },
   { e4seek,           "En se pla‡ant dans le fichier" },
   { e4write,          "En ‚crivant dans le fichier" },
   { e4close,          "En lib‚rant le fichier" },
   { e4remove,         "En effa‡ant le fichier" },
   { e4lock,           "En bloquant le fichier" },
   { e4unlock,         "En d‚bloquant le fichier" },
   { e4len,            "En d‚terminant la longueur du fichier" },
   { e4len_set,        "Mise … jour de la longueur du fichier" },
   { e4rename,         "D‚nomination du fichier" },

   /* Database Specific Errors */
   { e4data,           "Le fichier n'est pas une base de donn‚es:" },
   { e4record_len,     "La fiche est trop grande" },
   { e4field_name,     "Champ inconnu" },
   { e4field_type,     "Type de champ inconnu" },     

   /* Index File Specific Errors */
   { e4index,          "Ce n'est pas un fichier d'indice" },
   { e4entry,          "Le fichier d'indice n'est pas … jour" },
   { e4unique,         "La clef n'est pas unique" },
   { e4tag_name,       "L'article d‚sign‚ par l'indice n'existe pas" },

   /* Expression Evaluation Errors */
   { e4comma_expected, "\",\" ou \")\" manquant dans l'expression" },
   { e4complete,       "Expression incomplŠte" },
   { e4data_name,      "La base r‚f‚r‚e dans l'expression n'est pas pr‚sente" },
   { e4num_parms,      "Nombre ill‚gal de critŠres dans l'expression"},
   { e4overflow,       "L'expression donne un r‚sultat trop grand" },
   { e4right_missing,  "ParenthŠse manquante dans l'expression" },
   { e4type_sub,       "Un paramŠtre est de la mauvaise sorte" },
   { e4unrec_function, "L'expression contient une fonction inconnue" },
   { e4unrec_operator, "L'expression contient un op‚rateur inconnu" },
   { e4unrec_value,    "L'expression contient une valeur inconnue"} ,
   { e4unterminated,   "Apostrophe manquante dans l'expression"} ,

   /* Optimization Errors */
   { e4opt,            "Optimization Error"} ,
   { e4opt_suspend,     "Optimization Removal Failure"} ,
   { e4opt_flush,      "Optimization File Flushing Failure"} ,

   /* Relation Errors */
   { e4lookup_err,     "Matching Slave Record Not Located"} ,

   /* Critical Errors */
   { e4memory,         "Plus de m‚moire disponible" } ,
   { e4info,           "Information inexpect‚e"} ,
   { e4parm,           "ParamŠtre inexpect‚"} ,
   { e4demo,           "Au maximum d'articles dans la version de d‚monstration" } ,
   { e4result,         "R‚sultat inexpect‚"} ,
   { 0, 0 },
} ;

#endif  /* S4FRENCH */

#ifdef S4SCANDINAVIAN

ERROR_DATA e4error_data[] =
{
   /* General Disk Access Errors */
   { e4create,         "Creating File" },
   { e4open,           "Opening File" },
   { e4read,           "Reading File" },
   { e4seek,           "Seeking to File Position" },
   { e4write,          "Writing to File" },
   { e4close,          "Closing File" },
   { e4remove,         "Removing File" },
   { e4lock,           "Locking File" },
   { e4unlock,         "Unlocking File" },
   { e4len,            "Determining File Length" },
   { e4len_set,        "Setting File Length" },
   { e4rename,         "Renaming File" },

   /* Database Specific Errors */
   { e4data,           "File is not a Data File" },
   { e4record_len,     "Record Length is too Large" },
   { e4field_name,     "Unrecognized Field Name" },
   { e4field_type,     "Unrecognized Field Type" },

   /* Index File Specific Errors */
   { e4index,          "Not a Correct Index File" },
   { e4entry,          "Tag Entry Missing" },
   { e4unique,         "Unique Key Error" },
   { e4tag_name,       "Tag Name not Found" },

   /* Expression Evaluation Errors */
   { e4comma_expected, "Comma or Bracket Expected" },
   { e4complete,       "Expression not Complete" },
   { e4data_name,      "Data File Name not Located" },
   { e4length_err,     "IIF() Needs Parameters of Same Length" },
   { e4not_constant,   "SUBSTR() and STR() need Constant Parameters" },
   { e4num_parms,      "Number of Parameters is Wrong" },
   { e4overflow,       "Overflow while Evaluating Expression" },
   { e4right_missing,  "Right Bracket Missing" },
   { e4type_sub,       "Sub-expression Type is Wrong" }, 
   { e4unrec_function, "Unrecognized Function" },
   { e4unrec_operator, "Unrecognized Operator" },
   { e4unrec_value,    "Unrecognized Value"} ,
   { e4unterminated,   "Unterminated String"} ,

   /* Optimization Errors */
   { e4opt,            "Optimization Error"} ,
   { e4opt_suspend,     "Optimization Removal Failure"} ,
   { e4opt_flush,      "Optimization File Flushing Failure"} ,

   /* Relation Errors */
   { e4relate,         "Relation Error"} ,
   { e4lookup_err,     "Matching Slave Record Not Located"} ,

   /* Report Errors */
   { e4report,         "Report Error"} ,

   /* Critical Errors */
   { e4memory,         "Out of Memory"} ,
   { e4info,           "Unexpected Information"} ,
   { e4parm,           "Unexpected Parameter"} ,
   { e4demo,           "Exceeded Maximum Record Number for Demonstration"} ,
   { e4result,         "Unexpected Result"} ,
   { 0, 0 },
} ;
#endif  /* S4SCANDINAVIAN */

#endif  /* S4LANGUAGE */

#endif  /* S4ERROR_OFF */

int S4FUNCTION e4code( CODE4 S4PTR *c4 )
{
   return c4->error_code ;
}

int S4FUNCTION e4set( CODE4 S4PTR *c4, int new_err_code )
{
   int old_err_code ;

   old_err_code =  c4->error_code ;
   c4->error_code =  new_err_code ;
   return old_err_code ;
}

char S4PTR * S4FUNCTION e4text( int err_code )
{
   #ifndef S4OFF_ERROR
      int i ;

      for ( i=0; (int) e4error_data[i].error_num != 0; i++ )
         if ( e4error_data[i].error_num == err_code )
            return  e4error_data[i].error_data ;
   #endif
   return (char *)0 ;   /* err_code not matched */
}

void S4FUNCTION e4exit_test( CODE4 S4PTR *c4 )
{
   if ( c4->error_code < 0 )  e4exit(c4) ;
}

#ifdef S4OS2PM
void S4FUNCTION e4exit( CODE4 S4PTR *c4 )
{
   exit( 0 ) ;
}

int S4FUNCTION e4( CODE4 S4PTR *c4, int err_code, char *desc )
{
   return e4describe( c4, err_code, desc, 0, 0 ) ;
}

#ifndef S4ERROR_HOOK

static void e4do_err_out( int err_code, char *desc1, char *desc2, char *desc3 )
{
   int i, pos,  desc_number = 1 ;
   char *ptr ;
   HAB  e4hab ;
   HMQ  e4hmq ;
   char e4error_str[100] ;

   strcpy( e4error_str, E4_ERROR ) ;
   strcat( e4error_str, " #: " ) ;
   c4ltoa45( err_code, (char *)e4error_str+9, 4 ) ;
   pos = 13 ;

   e4error_str[pos++] = '\n' ;

   for ( i=0; (int) e4error_data[i].error_num != 0; i++ )
      if ( e4error_data[i].error_num == err_code )
      {
         strcpy( e4error_str+pos, e4error_data[i].error_data ) ;
         pos += strlen( e4error_data[i].error_data ) ;
         e4error_str[pos++] = '\n' ;
         break ;
      }
   
   ptr =  desc1 ;
   while ( (ptr != (char *) 0) && (desc_number <= 3 ) )
   {
      if ( strlen(desc1)+pos+3 >= sizeof(e4error_str) )
         break ;
      strcpy( e4error_str + pos, ptr ) ;
      pos +=  strlen(ptr) ;
      e4error_str[pos++] = '\n' ;
      if ( desc_number++ == 1 )
         ptr = desc2 ;
      else
         ptr = desc3 ;
   }
   
   e4error_str[pos] =  0 ;

   /* In case the application has done no PM Initialization, set up an
      instance to allow for the error output to occur */

   e4hab = WinInitialize(0) ;
   if ( e4hab == NULLHANDLE )
      return ;

   e4hmq = WinCreateMsgQueue(e4hab, 0) ;

   if ( e4hmq == NULLHANDLE )
   {
      WinTerminate(e4hab) ;
      return ;
   }

   /* And print out the error via a desktop message box */
   WinMessageBox(HWND_DESKTOP, HWND_DESKTOP, e4error_str, "Error", E4MSGBOXID, MB_OK | MB_MOVEABLE | MB_CUACRITICAL ) ;
   WinDestroyMsgQueue( e4hmq );
   WinTerminate(e4hab) ;
}
#endif

void S4FUNCTION e4exit( CODE4 S4PTR *c4 )
{
   exit( 0 ) ;
}

int S4FUNCTION e4( CODE4 S4PTR *c4, int err_code, char *desc )
{
   return e4describe( c4, err_code, desc, 0, 0 ) ;
}

int S4FUNCTION e4describe( CODE4 S4PTR *c4, int err_code, char *desc1, char *desc2, char *desc3 )
{
   #ifdef S4ERROR_HOOK
      c4->error_code =  err_code ;
      e4hook( c4, err_code, desc1, desc2, desc3 ) ;
   #else
      c4->error_code = err_code ;

      #ifndef S4OFF_ERROR
         if ( c4->off_error == 0 )
            e4do_err_out( err_code, desc1, desc2, desc3 ) ;
      #endif
   #endif   /* ifdef S4ERROR_HOOK */

   return err_code ;
}

void  S4FUNCTION e4severe( int err_code, char *desc )
{
   #ifdef S4ERROR_HOOK
      e4hook( (char *)0, err_code, desc, (char *)0, (char *)0 ) ;
      FatalAppExit( 0, desc ) ;
   #else
      e4do_err_out( err_code, desc, 0, 0 ) ;
      e4exit( 0 ) ; ;
   #endif
}

#endif  /* S4OS2PM */

#ifndef S4OS2PM

#ifdef S4WINDOWS

/* S4WINDOWS */
void  S4FUNCTION e4exit( CODE4 S4PTR *c4 )
{
   FatalAppExit( 0, E4_MESSAG_EXI ) ;
}

/* S4WINDOWS */
int S4FUNCTION e4( CODE4 S4PTR *c4, int err_code, char *desc )
{
   return e4describe( c4, err_code, desc, 0, 0 ) ;
}

/* S4WINDOWS */
int S4FUNCTION e4describe( CODE4 S4PTR *c4, int err_code, char *desc1, char *desc2, char *desc3 )
{
   #ifdef S4ERROR_HOOK
      c4->error_code =  err_code ;
      e4hook( c4, err_code, desc1, desc2, desc3 ) ;
   #else
      char error_str[257], *ptr ;
      int i, pos,  desc_number = 1 ;
      WORD wType ;
   
      c4->error_code =  err_code ;
   
      #ifndef S4OFF_ERROR
         if ( c4->off_error == 0 )
         {
            strcpy( error_str, E4_ERROR ) ;
            strcat( error_str, " #: " ) ;
            c4ltoa45( err_code, (char S4PTR *)error_str+9, 4 ) ;
            pos = 13 ;
      
            error_str[pos++] = '\n' ;
      
            for ( i=0; (int) e4error_data[i].error_num != 0; i++ )
               if ( e4error_data[i].error_num == err_code )
               {
                  strcpy( error_str+pos, e4error_data[i].error_data ) ;
                  pos +=  strlen( e4error_data[i].error_data ) ;
                  error_str[pos++] = '\n' ;
                  break ;
               }
      
            ptr =  desc1 ;
            while ( (ptr != (char *) 0) && (desc_number <= 3 ) )
            {
               if ( strlen(desc1)+pos+3 >=  sizeof(error_str) )
                  break ;
               strcpy( error_str+pos, ptr ) ;
               pos +=  strlen(ptr) ;
               error_str[pos++] = '\n' ;
               if ( desc_number++ == 1 )
                  ptr = desc2 ;
               else
                  ptr = desc3 ;
            }
      
            error_str[pos] =  0 ;
      
            OemToAnsi( error_str, error_str ) ;
      
            wType =  MB_OK | MB_ICONSTOP ;
      
            if ( err_code == e4memory )
               wType |=  MB_SYSTEMMODAL ;
      
            #ifdef S4VBASIC
               if ( MessageBox( 0, error_str, E4_ERROR_BAS, wType ) == 0 )
            #else
               if ( MessageBox( 0, error_str, E4_ERROR_COD, wType ) == 0 )
            #endif
               FatalAppExit( 0, E4_MEMORY_ERR ) ;
      
         }
      #endif
   #endif   /* ifdef S4ERROR_HOOK */

   return err_code ;
}

/* S4WINDOWS */
void  S4FUNCTION e4severe( int err_code, char *desc )
{
   #ifdef S4ERROR_HOOK
      e4hook( (char *)0, err_code, desc, (char *)0, (char *)0 ) ;
      FatalAppExit( 0, desc ) ;
   #else
      char error_str[257] ;
      int pos, i ;
   
      strcpy( error_str, E4_ERROR_SEV ) ;
      strcat( error_str, " #: " ) ;
      c4ltoa45( err_code, (char S4PTR *)error_str+16, 4 ) ;
      pos =  20 ;
   
      error_str[pos++] = '\n' ;
   
      for ( i=0; e4error_data[i].error_num != 0; i++ )
         if ( e4error_data[i].error_num == err_code )
         {
            strcpy( error_str+pos, e4error_data[i].error_data ) ;
            pos +=  strlen( e4error_data[i].error_data ) ;
            error_str[pos++] = '\n' ;
            break ;
         }
   
      if ( strlen(desc)+pos+4 < sizeof(error_str) )
      {
         strcpy( error_str+pos, desc ) ;
         pos +=  strlen(desc) ;
         error_str[pos++] = '\n' ;
      }
   
      error_str[pos] =  0 ;
   
      OemToAnsi( error_str, error_str ) ;
   
      MessageBox( 0, error_str, E4_ERROR_CDS, MB_OK | MB_ICONSTOP ) ;
   
      FatalAppExit( 0, error_str ) ;
   #endif  /* ifdef S4ERROR_HOOK  */
}

/* S4WINDOWS */
#ifdef S4VBASIC
   void  S4FUNCTION e4severe_vbasic( int err_code, char *desc )
   {
      char error_str[257] ;
      int i, pos ;
   
      strcpy( error_str, E4_ERROR_SEV ) ;
      strcat( error_str, " #: " ) ;
      c4ltoa45( err_code, (char S4PTR *)error_str+27, 4 ) ;
      pos =  31 ;
   
      error_str[pos++] = '\n' ;
   
      for ( i=0; e4error_data[i].error_num != 0; i++ )
         if ( e4error_data[i].error_num == err_code )
         {
            strcpy( error_str+pos, e4error_data[i].error_data ) ;
            pos +=  strlen( e4error_data[i].error_data ) ;
            error_str[pos++] = '\n' ;
            break ;
         }
   
      if ( strlen(desc)+pos+4 < sizeof(error_str) )
      {
         strcpy( error_str+pos, desc ) ;
         pos +=  strlen(desc) ;
         error_str[pos++] = '\n' ;
      }
   
      error_str[pos] =  0 ;

      OemToAnsi( error_str, error_str ) ;

      MessageBox( 0, error_str, E4_ERROR_BAS, MB_OK | MB_ICONSTOP ) ;
   }
#endif  /* S4VBASIC */

#endif  /* S4WINDOWS  */

#ifndef S4WINDOWS

#ifdef S4VB_DOS

/*  S4VB_DOS */
int S4FUNCTION e4( CODE4 S4PTR *c4, int err_code, char *desc )
{
   return e4describe( c4, err_code, desc, 0, 0 ) ;
}

/* S4VB_ DOS */
int S4FUNCTION e4describe( CODE4 S4PTR *c4, int err_code, char *desc1, char *desc2, char *desc3 )
{
   char error_str[257], *ptr ;
   int i, pos,  desc_number = 1 ;
   int err_type = V4ERROR ;

   c4->error_code =  err_code ;

   strcpy( error_str, E4_ERROR ) ;
   strcat( error_str, " #: " ) ;
   c4ltoa45( err_code, (char far *)error_str+9, 4 ) ;
   pos = 13 ;

   error_str[pos++] = '\n' ;

   for ( i=0; (int) e4error_data[i].error_num != 0; i++ )
      if ( e4error_data[i].error_num == err_code )
      {
         strcpy( error_str+pos, e4error_data[i].error_data ) ;
         pos +=  strlen( e4error_data[i].error_data ) ;
         error_str[pos++] = '\n' ;
         break ;
      }

   ptr =  desc1 ;
   while ( (ptr != (char *) 0) && (desc_number <= 3 ) )
   {
      if ( strlen(desc1)+pos+3 >=  sizeof(error_str) )  break ;
      strcpy( error_str+pos, ptr ) ;
      pos +=  strlen(ptr) ;
      error_str[pos++] = '\n' ;
      if ( desc_number++ == 1 )
         ptr = desc2 ;
      else
         ptr = desc3 ;
   }

   error_str[pos] =  0 ;

   u4MsgBox( v4str(error_str), (int near *)&err_type);

   return err_code ;
}

/* S4VB_DOS */
void  S4FUNCTION e4severe( int err_code, char *desc )
{
   char error_str[257] ;
   int pos, i ,err_type = V4SEVERE ;

   strcpy( error_str, E4_ERROR_SEV ) ;
   strcat( error_str, " #: " ) ;
   c4ltoa45( err_code, (char far *)error_str+27, 4 ) ;
   pos =  31 ;

   error_str[pos++] = '\n' ;

   for ( i=0; e4error_data[i].error_num != 0; i++ )
      if ( e4error_data[i].error_num == err_code )
      {
         strcpy( error_str+pos, e4error_data[i].error_data ) ;
         pos +=  strlen( e4error_data[i].error_data ) ;
         error_str[pos++] = '\n' ;
         break ;
      }

   if ( strlen(desc)+pos+4 < sizeof(error_str) )
   {
      strcpy( error_str+pos, desc ) ;
      pos +=  strlen(desc) ;
      error_str[pos++] = '\n' ;
   }

   error_str[pos] =  0 ;

   u4MsgBox( v4str(error_str), (int near *)&err_type);

   exit(err_code) ;
}

/* S4VB_DOS */
void  S4FUNCTION e4severe_vbasic( int err_code, char *desc )
{
   char error_str[257] ;
   int i, pos, err_type = V4SEVERE ;

   strcpy( error_str, E4_ERROR_SEV ) ;
   strcat( error_str, " #: " ) ;
   c4ltoa45( err_code, (char far *)error_str+27, 4 ) ;
   pos =  31 ;

   error_str[pos++] = '\n' ;

   for ( i=0; e4error_data[i].error_num != 0; i++ )
      if ( e4error_data[i].error_num == err_code )
      {
         strcpy( error_str+pos, e4error_data[i].error_data ) ;
         pos +=  strlen( e4error_data[i].error_data ) ;
         error_str[pos++] = '\n' ;
         break ;
      }

   if ( strlen(desc)+pos+4 < sizeof(error_str) )
   {
      strcpy( error_str+pos, desc ) ;
      pos +=  strlen(desc) ;
      error_str[pos++] = '\n' ;
   }

   error_str[pos] =  0 ;

   u4MsgBox( v4str(error_str), (int near *)&err_type);
}

/* S4VB_DOS */
void S4FUNCTION e4exit( CODE4 S4PTR *c4 )
{
   if ( c4 == 0 )
      exit(0) ;
   else
      exit( c4->error_code ) ;
}

#else /* S4VB_DOS */


/* not S4WINDOWS */
void S4FUNCTION e4exit( CODE4 S4PTR *c4 )
{
   if ( c4 == 0 )
      exit(0) ;
   else
      exit( c4->error_code ) ;
}

/* not S4WINDOWS */
#ifndef S4OFF_ERROR
static void  e4error_out( char *ptr )
{
   #ifdef S4UNIX
      printf("%s", ptr ) ;
   #else
      #ifdef S4TESTING
         fprintf(stdprn, "%s", ptr ) ;
      #endif
      write( 1, ptr, (unsigned int) strlen(ptr) ) ;
   #endif
}

/* not S4WINDOWS */
static void display( int err_code )
{
   char buf[11] ;
   int i ;

   c4ltoa45( (long) err_code, buf, 6 ) ;
   buf[6] =  0 ;
   e4error_out( buf ) ;

   for ( i = 0; e4error_data[i].error_data != 0; i++ )
      if ( e4error_data[i].error_num == err_code )
      {
         e4error_out( "\r\n" ) ;
         e4error_out( e4error_data[i].error_data ) ;
         break ;
      }
}
#endif

/* not S4WINDOWS */
int S4FUNCTION e4( CODE4 S4PTR *c4, int err_code, char *desc )
{
   return e4describe( c4, err_code, desc, 0, 0 ) ;
}

int S4FUNCTION e4describe( CODE4 S4PTR *c4, int err_code, char *desc1, char *desc2, char *desc3 )
{
   #ifdef S4ERROR_HOOK
      e4hook( c4, err_code, desc1, desc2, desc3 ) ;
   #else
      char *ptr ;
      int desc_number = 1 ;
   
      c4->error_code =  err_code ;
   
      #ifndef S4OFF_ERROR
         if ( c4->off_error == 0 )
         {
            e4error_out( E4_ERROR_NUM ) ;
            display ( err_code ) ;

            ptr =  desc1 ;
            while ( (ptr != (char *) 0) && (desc_number <= 3 ) )
            {
               e4error_out( "\r\n" ) ;
               e4error_out( ptr ) ;
               if ( desc_number++ == 1 )
                  ptr = desc2 ;
               else
                  ptr = desc3 ;
            }
      
            #ifdef S4UNIX
               e4error_out( E4_ERROR_ENT ) ;
               getchar() ;
            #else
               e4error_out( E4_ERROR_KEY ) ;
               #ifndef S4TESTING
                  getch() ;
               #endif
            #endif
         }
      #endif
   
   #endif  /* ifdef S4ERROR_HOOK  */

   return( err_code ) ;
}

void S4FUNCTION e4severe( int err_code, char *desc )
{
   #ifdef S4ERROR_HOOK
      e4hook( 0, err_code, desc, (char *)0, (char *)0 ) ;
   #else
      #ifndef S4OFF_ERROR
         e4error_out( E4_ERROR_SEV ) ;
         display( err_code ) ;
   
         e4error_out( "\r\n" ) ;
         e4error_out( desc ) ;
   
         #ifdef S4UNIX
            e4error_out( E4_ERROR_ENT ) ;
            getchar() ;
         #else
            e4error_out( E4_ERROR_KEY ) ;
            #ifndef S4TESTING
               getch() ;
            #endif
         #endif
      #endif
   #endif   /* ifdef S4ERROR_HOOK */

   exit(1) ;
}

#endif  /* S4VB_DOS */

#endif  /* S4WINDOWS  */

#endif  /* S4OS2PM */

