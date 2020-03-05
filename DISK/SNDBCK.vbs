' SNDBCK.vbs - Simple script to test ActiveHEAT's
' ability to send back a screen when it gets a Esc ( in T27 mode.
' Mark Riordan  11 July 2002
' Based on INSLIN of 2 May 2001
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
' ScreenMain = ScreenMain & Esc & "(?+I" 
ScreenMain = ScreenMain & Esc & "(This is a post-back"

Const WAIT_TIMEOUT=1, WAIT_ODT=2, WAIT_STOQ=3
bRunning = true

FormFileID = OpenFormFile("SNDBCK.FRM")
FormIDMain = OpenForm(FormFileID,"MAIN")

Do While bRunning
   which = Wait(9999,"SNDBCK",STOQ)
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
         retval = GetSTOQMsg("SNDBCK", Header, InBuf)
         InputForm = GetInputField(FormIDMain,"FORM",InBuf)
         If Mid(InputForm, 1, 1) <> "/" Then
            ScreenMain = MsgStart & "I got this: " & InputForm
         End If         
         retval = SendMCSBuf(Header, ScreenMain)
   End Select
Loop

