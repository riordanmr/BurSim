' STATLN.vbs - Program to test the T27 status line
' and its interaction with multiple pages.
' Used to investigate changes to Emu in which there
' will be 1 status line per session, not per page.
'
' Mark Riordan 12 Dec 2000

Const WAIT_TIMEOUT=1, WAIT_ODT=2, WAIT_STOQ=3
Esc = Chr(27)
Etx = Chr(3)
BegF = Chr(31)
EndF = Chr(30)
BegProt = Chr(28)
Bright = Chr(26)
MsgStart = Esc & "X" & Chr(12)
MsgEnd   = Esc & "W" & Etx

Dim ScreenMain, ScreenSecond
ScreenMain = MsgStart
ScreenMain = ScreenMain & Make80(Bright & "Welcome to the T27 Status Line Tester                    " & BegProt & "MAIN" & EndF)
ScreenMain = ScreenMain & Make80(" ")
ScreenMain = ScreenMain & Make80(" Notice the text on the status line, then hit Transmit to flip to a new page.") & Make80("")
ScreenMain = ScreenMain & Esc & "RS28This is 40 characters of status line    "
ScreenMain = ScreenMain & MsgEnd

ScreenSecond = ""
ScreenSecond = ScreenSecond & Esc & "$!" & Esc & "&"
ScreenSecond = ScreenSecond & MsgStart
ScreenSecond = ScreenSecond & Make80("This is a new page.  Is the status line still there?") & Make80("")
ScreenSecond = ScreenSecond & MsgEnd

Function Make80 (mystr)
   If Len(mystr) > 80 Then
      Make80 = Left(mystr,80)
   Else 
      Make80 = mystr & String(80-Len(mystr)," ")
   End If
End Function

Do 
   which = Wait(9999, "STATLN", STOQName)
   Select Case which
      Case WAIT_TIMEOUT
         SPOMsg "Wait timed out"
      Case WAIT_ODT
         Accept ODTMsg
         If "QUIT" = ODTMSG Then Exit Do
         SPOMsg "Unrecognized command: " & ODTMsg
      Case WAIT_STOQ
         retval = GetSTOQMsg("STATLN", Header, Buf)
         If Left(Buf,1) = "/" Then
            retval = SendMCSBuf(Header, ScreenMain)
         Else
            retval = SendMCSBuf(Header, ScreenSecond)
         End If
   End Select
Loop 
