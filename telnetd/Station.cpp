/*--- Station.cpp -- Functions for handling station settings
 *
 *  Mark Riordan   11 January 2000
 */

#include "stdafx.h"
#include "Station.h"

BOOL CopyToDelim(const char * &cp, char *targ)
{
   BOOL bAtEnd = FALSE;
   while(*cp && *cp!=STN_DELIM && *cp!='\n') {
      *(targ++) = *(cp++);
   }
   *targ = '\0';
   if('n' == *cp) {
      bAtEnd = TRUE;
   } else {
      *cp++;
   }
   return bAtEnd;
}

BOOL CrackLine(const char *line, CStation &station)
{
   BOOL bOK=TRUE;
   //station.st_name[0] = '\0';
   //station.st_initial_tran[0] = '\0';
   const char *cp = line;
   CopyToDelim(cp, station.st_name);
   CopyToDelim(cp, station.st_initial_tran);

   return bOK;
}

int CStationTable::Init(const char *pszFilename)
{
   int errcode=0;
   strncpy(m_szStationFile, pszFilename, sizeof m_szStationFile);
   return errcode;
}

/*--- function CStationTable::FindStation -----------------------------------
 *
 *  Find a station in the station table.
 *
 *  Entry:  pszStation  points to the station name to search for.
 *
 *  Exit:   Returns TRUE if the station name or "Default" was found,
 *           in which case 
 *          station  contains the parsed entry.
 */
BOOL CStationTable::FindStation(const char *pszStation, CStation &station)
{
   BOOL bFound=FALSE, bFoundDefault=FALSE;
   char szMyStation[32];
   CStation stnDefault, stnCurrent;

   strncpy(szMyStation, pszStation, sizeof szMyStation);
   TrimTrailingSpaces(szMyStation);

   FILE *sttable = fopen(m_szStationFile,"r");
   char line[256];
   if(sttable) {
      while(fgets(line,sizeof line, sttable)) {
         CrackLine(line, stnCurrent);
         if(0==stricmp(stnCurrent.st_name,szMyStation)) {
            station = stnCurrent;
            bFound = TRUE;
            break;
         } else if(0==stricmp(stnCurrent.st_name,"Default")) {
            stnDefault = stnCurrent;
            bFoundDefault = TRUE;
         }
      }
      fclose(sttable);
   }
   if(!bFound && bFoundDefault) {
      station = stnDefault;
      bFound = TRUE;
   }
   return bFound;
}