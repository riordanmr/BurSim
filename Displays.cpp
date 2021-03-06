#include <stdio.h>

char *aryMix[] = {
  "13AX DRIVER 2 UP",
  "",
  " SNIV410                 8.171 MD Mem.  88.4% CPU Avail      05/13/1999 19:55:00",
  "",
  "123`456}789{0123~45678",
  "  Mix#  Active    Mix         P M  Mem .   Mix#  Waiting   Mix         P M  Mem",
  "     23 SCSDVR                9 9  632 .       7 OITCTL                9 9  110",
  "     13 SWITCH                8 8  942 .         <NO FILE SCSIRD/SCSIRD MTP",
  "     21 FDPIO7                8 8  600 .",
  "     11 FDPSPO/SNIPCK         8 8  316 . Null Schedule",
  "     20 FDPSWT/SNIPCK         7 7  304 .",
  "     26 RKM001                7 7   17 .",
  "     24 DMS/ISAMDB            7 7  371 .",
  " 12/ 12 RKM/WATCHR            6 6   93 .",
  "  6/  6 MRR/WATCHR            6 6   93 .",
  " 15/ 15 TONY/WATCHR           6 6   93 .",
  " 25/ 25 TPED/WATCHR           6 6   93 .",
  "     10 UGNSPO/SNIWRK         6 6  316 .",
  "     16 UGENVT                6 6  224 .",
  "      3 OITFTP                6 6  644 .",
  "      5 OITFMN                6 6   54 .",
  "      3 OITMGR                6 6  597 .",
  "     18 UNIFTP                5 5  530 .",
  "     19 OITODT                4 4   19 .",
  NULL };

char *aryDisks[] = {
  "13AX DRIVER 2 UP",
  "",
  " SNIV410                 8.171 MD Mem.  88.0% CPU Avail      05/13/1999 19:55:43",
  "",
  "",
  "cc/uu -Disk- Type  Serial S Status Avl%.",
  " 5/0  ID0011 100MB        1 In Use 49.9.",
  " 4/1  ID0031 100MB        1 In Use 50.5.",
  " 4/0  ID0021 100MB        1 In Use 50.9.",
  "",
  "cc/uu -Pack- Type  Idx  R M Status Avl%.",
  " 5/2 *SNIWRK 1.5GB  1       In Use 10.5.",
  " 5/1 *SNIPCK 563MB      R   In Use 55.4.",
  "",
  "cc/uu -Tape Label-- RI#  Pt#  Status M#.",
  "14/0 *UG00  /UG00     1       Available.",
  "14/1 *UG01  /UG01     1       Available.",
  "15/1 *UGAT01          1  2591 Output 23.",
  "15/0 *UGAT00          1  2590 Input  23.",
  "15/3                          Saved    .",
  "14/2 *UG02  /UG02     1       Available.",
  "16/5                          Saved    .",
  "",
  "",
   NULL};

char *aryMsgs[] = {
  "13AX DRIVER 2 UP",
  "",
  " SNIV410                 9.644 MD Mem.  84.1% CPU Avail      05/13/1999 20:17:14",
  "",
  "",
  "------------------------------[  System Messages  ]-----------------------------",
  "      OITMGR=024 FTPMGR v3.37",
  "      BOJ OITFMN=006 072098 20:12",
  "      OITFMN=005 OITFMN v1.00",
  "      BOJ DMS/ISAMDB=011 121294 20:13",
  "      EOJ DMS/ISAMDB=011 20:13 IN 00:16 CHG 00:04",
  "      EOJ OITMGR=024 20:13 IN 00:22 CHG 00:05",
  "      OITFMN=005 Closed and reopened log file.",
  "      OITFTP=019 STOR (TEST)MRFILE ON PACK RECORD 80 RPB 9 DATA",
  "      BOJ COMPIL=024 092595 20:16",
  "      COMPIL=024 Creating PSEUDO Card deck",
  "      #01368 LOCKED COMPIL=024",
  "      COMPIL=024 Compile Initiated",
  "      BOJ BENCH1/BPL=011 122394 20:15",
  "      EOJ COMPIL=024 20:15 IN 00:05 CHG 00:02",
  "      MRR/WATCHR=006 Ready for Next Request",
  "      @02924 BENCH1 OPEN OUT BENCH1/BPL=011",
  "      @02924 BENCH1 05/2 DPK LOCKED BENCH1/BPL=011",
  "      EOJ BENCH1/BPL=011 20:15 IN 00:14 CHG 00:06",

   NULL};

char **ScreenTable[] = {aryMix, aryDisks, aryMsgs};

