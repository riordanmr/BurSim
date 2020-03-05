// Station.h -- header file for Station.cpp
// For BurSim   Mark Riordan  11 January 2000

class CStation {
   public:
   char st_name[16];
   char st_initial_tran[20];

};

#define STN_DELIM ','

class CStationTable {
public:
   char m_szStationFile[_MAX_PATH];  // Station table name

public:
   int Init(const char *pszFilename);
   BOOL FindStation(const char *pszStation, CStation &station);

};

