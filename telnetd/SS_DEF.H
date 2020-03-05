/***************************************************************************/
/*									   */
/*			STANDARD NETWORKS, INC.				   */
/*									   */
/*	Filename:  ss_def.h						   */
/*									   */
/*	Description:  This include file contains definitions of commonly   */
/*			used things.                                       */
/*									   */
/*	Revison History							   */
/*		Date	  By	 Description				   */
/*              --------- ---    -----------                               */
/*              21 May 91 DAE	 Created                                   */
/*									   */
/*	Technical Information:						   */
/*                                        				   */
/*									   */
/*  THIS IS AN UNPUBLISHED WORK PROTECTED BY FEDERAL COPYRIGHT LAW.        */
/*  ANY UNAUTHORIZED REPRODUCTION IS PROHIBITED.                           */
/*  THE CONTENTS OF THIS WORK ARE CONFIDENTIAL AND MAY NOT BE              */
/*  DISCLOSED OR USED WITHOUT EXPRESS WRITTEN PERMISSION OF                */
/*                                                                         */
/*                      STANDARD NETWORKS, INC.                            */
/*                      4610 UNIVERSITY AVENUE, SUITE 1080                 */
/*                      MADISON, WI 53705                                  */
/*			(608) 231-3100                                     */
/*                                                                         */
/*   (C) COPYRIGHT STANDARD NETWORKS, INC.                                 */
/*   ALL RIGHTS RESERVED                                                   */
/***************************************************************************/

#ifndef SS_DEF
#define SS_DEF   

#ifdef BIT32
#define NULL    0
#endif

#ifndef FALSE
#define TRUE    1
#define	FALSE	0
#endif

#define	ON	1
#define OFF	0

#define DOWN 0
#define UP   1

#define YES     1
#define	NO	0

#define UNI_ERROR	-1
#define UNI_NOERROR 0

#define SP	0x20

#endif /* #define SS_DEF */


/*********************************** eof ************************************/
