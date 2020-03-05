' INSLIN.vbs - Simple script to test Emu's ability
' to interpret Esc L (Insert Line)
' Mark Riordan  2 May 2001
' Based on WSTFLD.vbs
' Based on FLIP.vbs of 25 January 2000  based on BIG.vbs of 5 Jan 2000

Dim STOQ, ODTMsg, Buf

Function Make80 (mystr)
   If Len(mystr) > 80 Then
      Make80 = Left(mystr,80)
   Else 
      Make80 = mystr & String(80-Len(mystr)," ")
   End If
End Function

Esc = Chr(27)
Etx = Chr(3)
BegF = Chr(31)
EndF = Chr(30)
BegProt = Chr(28)
Bright = Chr(26)
MsgStart = Esc & "X" & Chr(12)
MsgEnd   = Esc & "W" & Etx
LF = Chr(10)
Dim ScreenMain, ScreenExit, ScreenBrowse, ScreenMod
ScreenMain = MsgStart
ScreenMain = ScreenMain & Make80(Bright & "Welcome to INSLIN - Esc L tester     ")
ScreenMain = ScreenMain & Make80("Line 2  ===================================")
ScreenMain = ScreenMain & "This is line 3."
ScreenMain = ScreenMain & Esc & "W" ' Homes the cursor
ScreenMain = ScreenMain & "456" & LF ' Move cursor to col 4 line 2
ScreenMain = ScreenMain & Esc & "L" ' Insert Line


Const WAIT_TIMEOUT=1, WAIT_ODT=2, WAIT_STOQ=3
bRunning = true

Do While bRunning
   which = Wait(9999,"INSLIN",STOQ)
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
         retval = GetSTOQMsg("INSLIN", Header, InBuf)
         InputForm = GetInputField(FormIDMain,"FORM",InBuf)
         bProcessed = False
         If True Then
            ' Unrecognized form--send the initial form.
            retval = SendMCSBuf(Header, ScreenMain)
         End If         
   End Select
Loop

