/***************************************************************************/
/*									   */
/*			STANDARD NETWORKS, INC.				   */
/*									   */
/*	Filename:  ss_type.h						   */
/*									   */
/*	Description:  This include files contains typedefs of commonly     */
/*			used items and items used for standardization.     */
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
/*  ANY UNAUTHORIZED DUPLICATION IS PROHBITED. 				   */
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

#ifndef SS_TYPE
#define SS_TYPE  

/* Typedef for boolean */

typedef	unsigned char	boolean;


/* Typedefs for character types */

typedef	signed char	char8;
typedef unsigned char	uchar;
typedef	unsigned char	uchar8;


/* Typedefs for integer types */

typedef signed char             int8;
typedef signed short         int16;
typedef signed long          int32;
typedef	unsigned char		uint8;
typedef unsigned short       uint16;
typedef unsigned long       uint32;

/* Typedefs for floating types */

typedef	float	fl32;
typedef	double	fl64;

#endif /* #define SS_TYPE */


/*********************************** eof ************************************/
