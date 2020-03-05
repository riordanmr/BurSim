' FDPSWT.vbs - Partially simulate OpenIT switch
' Mark Riordan   10 Sep 1999
SPOMsg "FDPSWT VERSION 3.6.4 VS OF 06/03/97 13:12"
SPOMsg "AUDIT WILL BE SELECTED BY THE F.E.P."
SPOMsg "AUDITING MSGS TO PROCESSES"
SPOMsg "WW- DSXAMI Parameters missing for FDPSWT"
SPOMsg "WW- Default values will be used AX to continue"

Accept TextIgnored
' Need to send STOQ to FDPIO7 here

SendSTOQMsg "TO_FDPIO7",""

retcode = Wait(2,"",STOQ)
ZIP "EX OITMGR"

retcode = Wait(99999,"", STOQ)
