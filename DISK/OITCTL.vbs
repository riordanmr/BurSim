' OITCTL.vbs - Partially simulate OpenIT OITCTL
' Mark Riordan  10 Sep 1999
SPOMsg "*OpenI.T.* 7.0.7.13 (980416) M-Dialog CTL *"
SPOMsg " Professional Networking Consultants, Inc. "
SPOMsg "ENABLE FDPIOX SINGLE STOQUE"
ZIP "EX OITFTP PR 6"
retcode = Wait(4, "", STOQ)
SPOMsg "OITCTL SCSI PATH ACTIVATED"
SPOMsg "TURBO MODE ENABLED - TURBO SCSI DRIVER"


retcode = Wait(9999,"", STOQ)
