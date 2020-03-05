' FLIP.vbs - Simple script to test Emu's new 
' "Initial Autoflip-to-local" setting.
' Mark Riordan  25 January 2000  based on BIG.vbs of 5 Jan 2000

Function Format2(MyVal)
   str1 = FormatNumber(MyVal,0,TristateTrue,TristateFalse,TristateFalse)
   If MyVal = 0 Then
      ' For some reason, this is necessary to get double zeros.
      str1 = "00" 
   ElseIf MyVal < 10 Then
      str1 = "0" & str1
   End If
   Format2 = str1
End Function

Function MakeTime(CurTime)
   MakeTime = Format2(Hour(CurTime)) & ":" & Format2(Minute(CurTime)) & _
     ":" & Format2(Second(CurTime))
End Function

Function MakeDate(CurDate)
   MakeDate = Year(CurDate) & "/" & Format2(Month(CurDate)) & "/" & Format2(Day(CurDate))
End Function

Sub SetDateAndTime(FormID,Buf)
   Dim MyStr
   MyStr = MakeDate(Date) & " " & MakeTime(Time)
   FillField FormID,"DateAndTime",Buf,MyStr
   FillField FormID,"DateAndTime2",Buf,MyStr
End Sub

FormFileID = OpenFormFile("FLIP.FRM")
FormIDMain = OpenForm(FormFileID,"MAIN")
FormIDFlip1 = OpenForm(FormFileID,"Flip1")
Dim STOQ, ODTMsg, Buf

Const WAIT_TIMEOUT=1, WAIT_ODT=2, WAIT_STOQ=3
bRunning = true

Do While bRunning
   which = Wait(9999,"FLIP",STOQ)
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
         retval = GetSTOQMsg("FLIP", Header, InBuf)
         InputForm = GetInputField(FormIDMain,"FORM",InBuf)
         bProcessed = False
         If "MAIN  " = InputForm Then
            ' If the user sends the MAIN form, then do the test
            ' by sending 4 screens in rapid succession.
            For iflip = 1 To 4
               Buf = InitFormBuf(FormIDFlip1)
               FillField FormIDFlip1,"FORM", Buf, "FLIP" & iflip
               FillField FormIDFlip1,"WhichPage", Buf, "This is page " & iflip
               SetDateAndTime FormIDFlip1, Buf
               retval = SendMCSBuf(Header, Buf)
            Next      
         Else
            ' Unrecognized form--send the initial form.
            Buf = InitFormBuf(FormIDMain)
            FillField FormIDMain,"FORM", Buf,"MAIN"
            SetDateAndTime FormIDMain,Buf
            retval = SendMCSBuf(Header, Buf)
         End If         
   End Select
Loop

