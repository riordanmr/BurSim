' TRMTST.VBS - Simple script to test terminal API.
'
' Mark Riordan  17 October 1999


Function GetHeaderConnNum(Header)
   GetHeaderConnNum = Mid(Header,13,6)
End Function

Function GetHeaderStationName(Header)
   GetHeaderStationName = Trim(Mid(Header,20,18))
End Function
   

Const WAIT_TIMEOUT=1, WAIT_ODT=2, WAIT_STOQ=3
bRunning = true

Do While bRunning
   which = Wait(9999,"TRMTST",STOQ)
   Select Case which
      Case WAIT_TIMEOUT
      Case WAIT_ODT
         Accept ODTMsg
         If "QUIT" = ODTMsg Then 
            bRunning = false
         Else
            SPOMsg "Unrecognized command: " & ODTMsg
         End If
      Case WAIT_STOQ
         retval = GetSTOQMsg("TRMTST", Header, InBuf)
         Buf = "You are connection number " & GetHeaderConnNum(Header) & _
          "   Station """ & GetHeaderStationName(Header) & """"

         retval = SendMCSBuf(Header, Buf)
   End Select
Loop

