' TSTOQ.vbs - Test STOQ
' Mark Riordan   6 Sep 1999

Const WAIT_TIMEOUT=1, WAIT_ODT=2, WAIT_STOQ=3
Esc = Chr(27)
Etx = Chr(3)
Do 
   which = Wait(99, "TSTOQ", STOQName)
   Select Case which
      Case WAIT_TIMEOUT
         SPOMsg "TSTOQ timed out"
      Case WAIT_ODT
         Accept ODTMsg
         If "QUIT" = ODTMSG Then Exit Do
      Case WAIT_STOQ
         retval = GetSTOQMsg("TSTOQ", Header, Buf)
         SPOMsg "Received " & Len(buf) & " byte msg from stn " & Left(Header,6) & ": " & Buf
         
         MyMsg = Esc & """ ("  &  "You said: " & Buf & Etx
         retval = SendMCSBuf(Header, MyMsg)
   End Select
Loop 
