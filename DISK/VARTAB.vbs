' VARTAB.vbs - Program to test T27 variable tabs.
' Sets variable tabs.
'
' Mark Riordan 19 Feb 2001

Const WAIT_TIMEOUT=1, WAIT_ODT=2, WAIT_STOQ=3
Esc = Chr(27)
Etx = Chr(3)
BegF = Chr(31)
EndF = Chr(30)
BegProt = Chr(28)
Bright = Chr(26)
MsgStart = Esc & "X" & Chr(12)
MsgEnd   = Esc & "W" & Etx

Dim ScreenMain, ScreenToggle, ScreenReset
ScreenMain = MsgStart
ScreenMain = ScreenMain & Make80(Bright & "Welcome to the T27 Variable Tabs Tester                    " & EndF)
ScreenMain = ScreenMain & Make80(" ")
ScreenMain = ScreenMain & Make80(" Type TOGGLE to toggle tabs") & Make80("")
ScreenMain = ScreenMain & Make80("   or RESET to clear tabs")
ScreenMain = ScreenMain & Esc & "RS28This is 40 characters of status line    "
ScreenMain = ScreenMain & MsgEnd

ScreenToggle = ""
ScreenToggle = ScreenToggle & MsgStart
ScreenToggle = ScreenToggle & "12345" & Esc & ".678901234567890" & Esc & "." & "123456789012345678901234567890123456789012345678901234567890"
ScreenToggle = ScreenToggle & Make80("Your tabs have been toggled at 5 and 20") & Make80("")
ScreenToggle = ScreenToggle & MsgEnd

ScreenReset = MsgStart
ScreenReset = ScreenReset & Esc & "# Your tabs have been cleared."


Function Make80 (mystr)
   If Len(mystr) > 80 Then
      Make80 = Left(mystr,80)
   Else 
      Make80 = mystr & String(80-Len(mystr)," ")
   End If
End Function

Do 
   which = Wait(9999, "VARTAB", STOQName)
   Select Case which
      Case WAIT_TIMEOUT
         SPOMsg "Wait timed out"
      Case WAIT_ODT
         Accept ODTMsg
         If "QUIT" = ODTMSG Then Exit Do
         SPOMsg "Unrecognized command: " & ODTMsg
      Case WAIT_STOQ
         retval = GetSTOQMsg("VARTAB", Header, Buf)
         If Left(Buf,1) = "/" Then
            retval = SendMCSBuf(Header, ScreenMain)
         ElseIf Left(Buf,6) = "TOGGLE" Then
            retval = SendMCSBuf(Header, ScreenToggle)
         ElseIf Left(Buf,5) = "RESET" Then
            retval = SendMCSBuf(Header, ScreenReset)
         End If
         SPOMsg "Rec cmd: " & Buf
   End Select
Loop 
