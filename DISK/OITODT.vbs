' OITODT.vbs - Partially simulator the OpenIT ODT program terminator.
' Mark Riordan  10 Sep 1999
retcode = Wait(2,"",STOQ)
SPOMsg "OITODT: OPERATOR ODT-INPUT 7.0.2.05 (980901)"
retcode = Wait(99999,"",STOQ)