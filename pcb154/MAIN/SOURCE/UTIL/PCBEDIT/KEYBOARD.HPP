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
/*                                KEYBOARD.HPP                                */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/*                                                                            */
/*                      Functions for keyboard handling.                      */
/*                                                                            */
/*============================================================================*/
/*                                                                            */
/*                       Written by Scott Dale Robison                        */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/*                                                                            */
/*            Copyright (C) 1993, Clark Development Company, Inc.             */
/*                                                                            */
/******************************************************************************/

#ifndef	___KEYBOARD_HPP___

#define	___KEYBOARD_HPP___

/******************************************************************************/

// Defined Macros

#define	BS				8
#define	TAB				9
#define	LF				10
#define	ENTER			13
#define	ESC				27

#define	STAB			(15+256)

#define	AA				(30+256)
#define	AB				(48+256)
#define	AC				(46+256)
#define	AD				(32+256)
#define	AE				(18+256)
#define	AF				(33+256)
#define	AG				(34+256)
#define	AH				(35+256)
#define	AI				(23+256)
#define	AJ				(36+256)
#define	AK				(37+256)
#define	AL				(38+256)
#define	AM				(50+256)
#define	AN				(49+256)
#define	AO				(24+256)
#define	AP				(25+256)
#define	AQ				(16+256)
#define	AR				(19+256)
#define	AS				(31+256)
#define	AT				(20+256)
#define	AU				(22+256)
#define	AV				(47+256)
#define	AW				(17+256)
#define	AX				(45+256)
#define	AY				(21+256)
#define AZ				(44+256)

#define	CA				1
#define	CB				2
#define	CC				3
#define	CD				4
#define	CE				5
#define	CF				6
#define	CG				7
#define	CH				8
#define	CI				9
#define	CJ				10
#define	CK				11
#define	CL				12
#define	CM				13
#define	CN				14
#define	CO				15
#define	CP				16
#define	CQ				17
#define	CR				18
#define	CS				19
#define	CT				20
#define	CU				21
#define	CV				22
#define	CW				23
#define	CX				24
#define	CY				25
#define CZ				26

#define	LEFT			(75+256)
#define	RIGHT			(77+256)
#define	UP				(72+256)
#define	DOWN			(80+256)
#define	PGUP			(73+256)
#define	PGDN			(81+256)
#define	HOME			(71+256)
#define	END				(79+256)

#define CLEFT           (115+256)
#define CRIGHT          (116+256)
#define CUP             (141+256)
#define CDOWN           (145+256)
#define	CPGUP			(132+256)
#define	CPGDN			(118+256)
#define	CHOME			(119+256)
#define	CEND			(117+256)

#define	INS				(82+256)
#define	DEL				(83+256)

#define	F1				(59+256)
#define	F2				(60+256)
#define	F3				(61+256)
#define	F4				(62+256)
#define	F5				(63+256)
#define	F6				(64+256)
#define	F7				(65+256)
#define	F8				(66+256)
#define	F9				(67+256)
#define	F10				(68+256)

#define	SF1				(84+256)
#define	SF2				(85+256)
#define	SF3				(86+256)
#define	SF4				(87+256)
#define	SF5				(88+256)
#define	SF6				(89+256)
#define	SF7				(90+256)
#define	SF8				(91+256)
#define	SF9				(92+256)
#define	SF10			(93+256)

#define	CF1				(94+256)
#define	CF2				(95+256)
#define	CF3				(96+256)
#define	CF4				(97+256)
#define	CF5				(98+256)
#define	CF6				(99+256)
#define	CF7				(100+256)
#define	CF8				(101+256)
#define	CF9				(102+256)
#define	CF10			(103+256)

#define	AF1				(104+256)
#define	AF2				(105+256)
#define	AF3				(106+256)
#define	AF4				(107+256)
#define	AF5				(108+256)
#define	AF6				(109+256)
#define	AF7				(110+256)
#define	AF8				(111+256)
#define	AF9				(112+256)
#define	AF10			(113+256)

/******************************************************************************/

// Function Prototypes

void pascal putMacro   (char * s);
void pascal flushMacro (void);
int  pascal keyHit     (void);
int  pascal getKey     (void);

/******************************************************************************/

#endif
