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


/****************************************************************************
*
*				   Pcboard's FIDO configuration class
*						  Author: Stan Paulsen
*					Copyright Clark Development 1995
*
*			  Prototypes for all objects in configuration data
* Classes included
* cAREAS, cORIGINS, cNODEARC, cXLATEPHONE, cAKAS, cNODELISTS, cFREQPATHS,
* cFREQDENY, cMAGICNAMES, cCONFIG
*****************************************************************************/

#ifndef DATA_H
#define DATA_H


#ifdef PCBSETUP
#define writeFidolog(a,b)
#endif
// headers needed for data type in this header
// PCBoard headers
#ifndef PCBSETUP
#include "pcboard.h"
#include "misc.h"
#include "tossmisc.h"
#endif
#include "screen.h"
#include "dosclass.hpp"

#include "structs.h"

template <class T>
class cCONFIGBASE
{

	public:
							cCONFIGBASE 	 (const char * filename, const bool path);
	 virtual			   ~cCONFIGBASE 	 (void);



				bool		addRec			 (const T & rec);
				bool		removeRec		 (unsigned int recnum);

	  virtual  unsigned int findRec 		 (const char *) {return 0;}
				bool		modifyRec		 (T & rec,unsigned int recnum);
				T	  & 	getRec			 (unsigned int recno);
				bool		errStat 		 (void){return err;}
	  unsigned	int 		totRecs 		 (void){return (unsigned int)( (f.size() - (long)sizeof(filever))/(long)sizeof(T));}
				bool		removeFile		 (void);
				void		packFile		 (void);
				uint		getVersion		 (void){return curver;}
//		virtual   bool		  isDupe		 (const T & rec);
				void		open			 (void){if(!f.isOpen() && f.open(datafile,OPEN_RDWR| OPEN_DENYNONE) != 0) err = TRUE;}
				void		close			 (void){if(f.isOpen()) f.close();}
				bool		lockHdr 		 (void){ return( f.lockChk(0,sizeof(filever)) == 0 );}
				bool		unlockHdr		 (void){ return( f.unlock (0,sizeof(filever)) == 0 );}
				bool		NeedToPack		 (void){return needtopack;}
				void		NeedToPack		 (bool _needToPack){needtopack = _needToPack;}
				bool		Err 			 (void){return err;}
				void		Err 			 (bool _err){err = _err;}

	protected:

	 cDOSFILE		 f;

	private:
	  // Data
	  char			  datafile[MAXFLEN];
	  bool			  openedbyme,err,needtopack;
	  uint			  filever;
	  uint			  curver;
	  uint			  recsize,totrecs;

	  // Functions
				bool		createFile		 (void);
				uint		getFileVer		 (void);
				long		seekToRec		 (unsigned int recnum);
				void		reportErr		 (const char * msg, const bool display);


};

template<class T>
cCONFIGBASE<T>::cCONFIGBASE(const char * filename, const bool path)
{
	// Generic initializations
	openedbyme = needtopack = FALSE;
	recsize = totrecs = curver = filever = err = 0;
	memset(datafile,'Z',sizeof(datafile));

	// The current file version is the same for all derived objects. This should
	// Be updated when the file format changes
	curver = 3;

	// Record size is the size of the template object
	recsize = sizeof(T);

	if(!path)
	{
	  // Build datafile name using filaname passed as parameter
	  maxstrcpy(datafile,PcbData.FidoLoc,sizeof(datafile));
	  char * t = findstartofname(datafile);
	  if(t) *t = '\x0';
	  strcat(datafile,filename);
	}
	else
	  maxstrcpy(datafile,(char *)filename,sizeof(datafile));

	// Now attempt to open the file if it is not already open
	if(fileexist(datafile) == 255) createFile();
	if(!f.isOpen() && f.open(datafile,OPEN_RDWR | OPEN_DENYNONE ) == 0)
	{
	  if(!f.isOpen())
		err = TRUE;
	  else
		openedbyme = TRUE;
	}
	else err = TRUE;


	// Check the datfile's version number and compare against current version number
	if(!err && getFileVer() != curver)
	{
	char logBuf[70];
		 sprintf(logBuf,"Wrong configuration file version, %s, Use conversion utility.",findstartofname(datafile));
		 reportErr(logBuf,TRUE);
		 err = TRUE;
	}

}
template<class T>
cCONFIGBASE<T>::~cCONFIGBASE(void)
{
   if(needtopack) packFile();
   if(openedbyme && f.isOpen()) f.close();
}

template<class T>
long  cCONFIGBASE<T>::seekToRec(unsigned int recnum)
{
long offset = 0;

  offset =	((recnum - 1) * long(recsize)) + sizeof(filever);
  return (f.seek(offset,SEEK_SET));
}





template< class T >
uint cCONFIGBASE<T>::getFileVer(void)
{
	filever = 0;

	// If an error hasn't occured yet seek the file begin and read file version
	if(!err)
	{
	  f.seek(0,SEEK_SET);
	  if(f.read(&filever,sizeof(filever)) != sizeof(filever))
	  {
		 filever = 0;
		 err = TRUE;
	  }
	}
	// Returns 0 if there was an error
	return filever;
}

template< class T >
bool cCONFIGBASE<T>::createFile(void)
{
	if(fileexist(datafile) != 255) return FALSE;

	if(f.isOpen()) f.close();
	if(f.open(datafile,OPEN_RDWR | OPEN_CREATE | OPEN_DENYNONE)!= 0) err = TRUE;
	//f.lockChk(0L,2L);
	f.write(&curver,sizeof(curver));
	//f.unlock(0L,2L);

	//f.close();
	return err;
}
template< class T >
bool cCONFIGBASE<T>::removeFile(void)
{
	f.close();
	unlink(datafile);
	createFile();
	open();
	return TRUE;
}

template< class T >
void cCONFIGBASE<T>::reportErr(const char * msg, const bool display)
{
	#ifndef PCBSETUP
	writeFidolog(msg,BLOCK);
	#endif
	if(display) fastprint(0,MESSAGES,(char *)msg,0x0b);
}

template< class T >
void cCONFIGBASE<T>::packFile(void)
{
	T			  rec,mtyrec;
	uint		  ver = 0;
	cDOSFILE	  tfile;

	if(err || totRecs() == 0) return;

	memset(&mtyrec,0,sizeof(mtyrec));
	memset(&rec,0,sizeof(rec));

	// Build a temporary file to copy valid records into
	tfile.setName(f.getName());
	tfile.setExt(".TMP");

	tfile.unlink();
	// Open temporary file and start copying valid records into it
	if(tfile.openChk(OPEN_RDWR | OPEN_CREATE | OPEN_DENYRDWR) != 0) return;

	f.close();

	if(f.openChk(OPEN_READ|OPEN_DENYRDWR) != 0)
	{
	  tfile.close();
	  return;
	}

	f.seek(0,SEEK_SET);
	tfile.seek(0,SEEK_SET);

	if(f.read(&ver,sizeof(ver)) != sizeof(ver)) 				err = TRUE;
	if(!err && tfile.write(&curver,sizeof(ver)) != sizeof(ver))    err = TRUE;

	while(!err && f.read(&rec,sizeof(rec)) == sizeof(rec))
	{
	  if(memcmp(&rec,&mtyrec,sizeof(rec))==0) continue;
	  tfile.write(&rec,sizeof(rec));
	}

	// Now rename the temp file to the datafile
	  tfile.close();
	  f.close();
	  if(!err)
	  {
		uint count = 0;
		while(fileexist((char*)f.getName()) != 255 && count++ <=10)
		{
		  f.unlink();
		  #ifdef PCBSETUP
			 if(count >= 2) mydelay(100);
		  #else
			 if(count >= 2) tickdelay(ONESECOND);
		  #endif
		}
		if(fileexist((char*)f.getName()) == 255) tfile.rename(f.getName());
	  }
	  if(f.open(OPEN_RDWR | OPEN_DENYNONE ) != 0) err = TRUE;
}


template< class T >
T & cCONFIGBASE<T>::getRec(unsigned int recno)
{
  static _FAR_ T rec;
	memset(&rec,0,sizeof(rec));

	if(err) return rec;
	seekToRec(recno);
	if(f.read(&rec,sizeof(T)) != sizeof(T)) err = TRUE;
	return rec;
}


template< class T >
bool cCONFIGBASE<T>::modifyRec(T & rec,unsigned int recnum)
{
	// Seek to the record number and write out the new record
	if(err == TRUE) return FALSE;
	seekToRec(recnum);
	if(f.write(&rec,sizeof(rec)) != sizeof(rec)) err = TRUE;
	return (! (err == TRUE) );		   // Return FALSE if err == TRUE
}


template< class T >
bool cCONFIGBASE<T>::removeRec(unsigned int recnum)
{
  T mtyrec;

	memset(&mtyrec,0,sizeof(mtyrec));
	seekToRec(recnum);
	if(f.write(&mtyrec,sizeof(mtyrec)) != sizeof(mtyrec)) err = TRUE;
	f.flush();
	if(!err) needtopack = TRUE;
	return (! (err == TRUE) );		   // Return FALSE if err == TRUE
}

/*
template< class T >
bool cCONFIGBASE<T>::isDupe(const T & rec)
{
unsigned int i;
unsigned int totrecs=totRecs();
T			 trec;

  for(i=1;i<=totrecs;i++)
  {
	trec = getRec(i);
	if(memcmp(&trec,&rec,sizeof(T)) == 0) return TRUE;
  }
  return FALSE;
}
*/



template< class T >
bool cCONFIGBASE<T>::addRec(const T & rec)
{
  T mtrec;

	memset(&mtrec,0,sizeof(mtrec));
	if(memcmp(&mtrec,&rec,sizeof(T))==0) return FALSE;
	lockHdr();
	f.seek(0,SEEK_END);
	if(f.write((void *)&rec,sizeof(rec)) != sizeof(rec)) err = TRUE;
	unlockHdr();
	return (! (err == TRUE) );		   // Return FALSE if err == TRUE
}

class cAREAINDEX : public cCONFIGBASE<AINDEX>
{
  public:
				 cAREAINDEX(void)	: cCONFIGBASE<AINDEX>("AREAS.IDX",FALSE){}
	unsigned int findRec	 (const char *){return 0U;}
	AINDEX	  &  findIRec	 (unsigned int conf);
};

class cAREAS : public cCONFIGBASE<NAREA_STRUCT>
{

  private:
	cAREAINDEX	index;
	AINDEX *	irecs;			// Pointer to keep conference index in memory
	bool		irecsinmem; 	// Is the index durrently in memory
	// bool 	   err; 	 PC-Lint reported that this hides the err defined in cCONFIGBASE
  public:
				 cAREAS 		  (void) : cCONFIGBASE<NAREA_STRUCT>("AREAS.DAT",FALSE){irecs=NULL;irecsinmem=FALSE;Err(FALSE);}
				~cAREAS 		  (void){delete(irecs);}

	bool		 sortFile		  (void);
	void		 insertSorted	  (NAREA_STRUCT & rec);
	bool		 updateIndex	  (void);

	NAREA_STRUCT & getAreaRec		   (unsigned int conf);
	unsigned int findRec			   (const char * area);

	void		 updateLastActiveDate  (const char * areaname);
	char *		 getOrigin			   (unsigned int conf);

	char *		 getAKA 			   (unsigned int conf);
	bool		 gatPrimAkaForNet	   (FUSERS & akauser,FUSERS & destuser);
	void		 updateAkaIndex 	   (void);
	void		 updateOriginsIndex    (void);


  protected:

};


class cORIGINS : public cCONFIGBASE<ORIGIN>
{
  public:
				 cORIGINS(void) : cCONFIGBASE<ORIGIN>("ORIGINS.DAT",FALSE){};
				~cORIGINS(void){};
	unsigned int findRec	   (const char * orig);
  protected:
  private:
	ORIGIN origin;
};


class cAKAS : public cCONFIGBASE<NADDRESS>
{
  public:
				 cAKAS(void) : cCONFIGBASE<NADDRESS>("AKAS.DAT",FALSE){};
				~cAKAS(void){};
	unsigned int findRec	  (const char * aka);

  protected:


  private:
};

#ifndef PCBFU

class cNODEARC : public cCONFIGBASE<NNODE_T>
{
  public:
				 cNODEARC(void) : cCONFIGBASE<NNODE_T>("NODEARC.DAT",FALSE){};
				~cNODEARC(void){};
	unsigned int findRec	   (const char * addr);

  protected:
  private:
};

class cXLATEPHONE : public cCONFIGBASE<TRANSLATE>
{
  public:
				 cXLATEPHONE(void) : cCONFIGBASE<TRANSLATE>("PHONEX.DAT",FALSE){};
				~cXLATEPHONE(void){};
unsigned int	 findRec(const char * in);
	bool		 findAndReplace    (char * phonenum);

  protected:
  private:
	void		 processPreSuffix  (char * phone);
};

class cNODELISTS : public cCONFIGBASE<NODELIST>
{
  public:
				 cNODELISTS(void) : cCONFIGBASE<NODELIST>("NODELIST.DAT",FALSE){};
				~cNODELISTS(void){};
	unsigned int findRec   (const char * nodelist);



  protected:

  private:
};

class cFREQPATHS : public cCONFIGBASE<NFREQ_PATH>
{
  public:
				 cFREQPATHS(void) : cCONFIGBASE<NFREQ_PATH>("FREQPATH.DAT",FALSE){};
				//~cFREQPATHS(void){};
unsigned int	 findRec(const char * path);

  protected:
  private:
};

class cMAGICNAMES : public cCONFIGBASE<NFREQ_MAGIC>
{
  public:
				 cMAGICNAMES	(void) : cCONFIGBASE<NFREQ_MAGIC>("MAGICNAM.DAT",FALSE){};
				~cMAGICNAMES	(void){};
	unsigned int findRec		(const char * magicname);
	bool		 checkPassword	(unsigned int recnum,const char * password);

  protected:
  private:
};

#endif // ifndef PCBFU

class cFREQDENY : public cCONFIGBASE<NADDRESS>
{
  public:
				 cFREQDENY	  (void) : cCONFIGBASE<NADDRESS>("FREQDENY.DAT",FALSE){};
	unsigned int findRec	  (const char * addr);

  protected:
  private:
};

//#ifndef PCBSETUP
class cSEND : public cCONFIGBASE<FSEND>
{
  public:
				 cSEND		   (void) : cCONFIGBASE<FSEND>("SENDS.NFO",FALSE){};
	unsigned int findRec	   (const char * entry);
	void		 process	   (void);
};
//#endif

class cCONFIG
{

	private:
	  char			  datafile[MAXFLEN];
	  bool			  openedbyme,err;
	  uint			  filever;
	  uint			  curver;
	  unsigned long   diroffset,emsioffset,freqoffset,archoffset;
	  cDOSFILE		  f;
	public:
				cCONFIG(void);
			   ~cCONFIG(void){if(openedbyme) f.close();}
	void		create(void);
	bool		getDirs(DIRECTORIES & dirs);
	bool		getEMSI(EMSI_DATA & edata);
	bool		getFREQInfo(FREQ_INFO & finfo);
	bool		getArchInfo(ARCHIVERS & archinfo);
	bool		putDirs(DIRECTORIES & dirs);
	bool		putEMSI(EMSI_DATA & edata);
	bool		putFREQInfo(FREQ_INFO & finfo);
	bool		putArchInfo(ARCHIVERS & archinfo);


	protected:


};

#endif
