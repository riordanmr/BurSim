' FDPIO7.vbs - Partially simulate the behavior of OpenIT I/O Driver
' Mark Riordan  10 Sep 1999
Const WAIT_TIMEOUT=1, WAIT_ODT=2, WAIT_STOQ=3

SPOMsg "* OpenI.T. IOD-STQ1 * 7.0.8.02  (990901) *"
SPOMsg "*Professional Networking Consultants, Inc.*"
SPOMsg "GET PROGRAM CONFIGURATION INFO"
SPOMsg "SWITCH AUDIT OFF AT IOD-BOJ"
SPOMsg "*WARNING* 'CTL NOT IN MIX  *"
SPOMsg "********* ZIP EXECUTE 'CTL *"
ZIP "EX OITCTL PR 9 LOCK"
retcode = Wait(4,"", STOQ)
SPOMsg "********* ZIP EXECUTE 'ODT *"
ZIP "EX OITODT LOCK"
retcode = Wait(5,"",STOQ)
SPOMsg "My Host# = 0001"
SPOMsg "My NetW# = 0000"
SPOMsg "My Node# = 0001"
SPOMsg "Console# = 0001 on My Node"
SPOMsg "SRI-Initiator = /"
SPOMsg "NODE INFO COMPLETED"
SPOMsg "REQUESTING PROGRAM INFORMATION"
retcode = Wait(3,"",STOQ)
SPOMsg "* PGM ENTRIES = 0140"
SPOMsg "* PGM HIGHEST = 0145"
SPOMsg "PROGRAM INFO COMPLETED"
SPOMsg "*WARNING: FDPIOX NOT RETRIEVING NSEC STOQUES*"
SPOMsg "WAIT FOR SWITCH BOJ HANDSHAKE"
SPOMsg "OPTION TO ZIP SWITCH SET"
ZIP "EX FDPSWT PR 7 LOCK"

retcode = Wait(600,"TO_FDPIO7",STOQ)
if WAIT_TIMEOUT = retcode Then
   SPOMsg "Timed out waiting for SWITCH.  Resuming initialization."
End If
retcode = GetSTOQMsg("TO_FDPIO7", Header, STOQMsg)
SPOMsg "OIT DRIVER LINKED TO SWITCH"
SPOMsg "---> MCS TABLE INFORMATION COMPLETED"
SPOMsg "* OpenI.T. IOD-STQ1 * 7.0.8.02  (990901) *"
SPOMsg "MAXPGM=350  I0D7  NTWRK=0000  SINGLE-STOQUE CTL-NSEC"
SPOMsg "Advantage NetWork Routing DISABLED"
SPOMsg "---> ENTER MAIN-LINE LOOP"
SPOMsg "*START-UP* HANDLER(0002) DOWN HL-STN"
SPOMsg "*START-UP* HANDLER(0004) UP HL-STN"
SPOMsg "---> INITIALIZATION COMPLETE"

retcode = Wait(99999,"TO_FDPIO7",STOQ)
