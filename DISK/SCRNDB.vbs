' SCRNDB.vbs - Simple script to test ActHeat's
' screens database screen recognition w/ AND and OR
' "Initial Autoflip-to-local" setting.
' Mark Riordan  13 Nov 2000
' based on FLIP.vbs25 January 2000  based on BIG.vbs of 5 Jan 2000

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

FormFileID = OpenFormFile("SCRNDB.FRM")
FormIDMain = OpenForm(FormFileID,"MAIN")
FormIDResult = OpenForm(FormFileID,"Result")
Dim STOQ, ODTMsg, Buf

Const WAIT_TIMEOUT=1, WAIT_ODT=2, WAIT_STOQ=3
bRunning = true

Do While bRunning
   which = Wait(9999,"SCRNDB",STOQ)
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
         retval = GetSTOQMsg("SCRNDB", Header, InBuf)
         InputForm = GetInputField(FormIDMain,"FORM",InBuf)
         bProcessed = False

         If "MAIN  " = InputForm Then
            Buf = InitFormBuf(FormIDResult)
            FillField FormIDMain,"FORM", Buf,"RESULT"
            SetDateAndTime FormIDResult,Buf
            Dim f1, f2, f3
            f1 = GetInputField(FormIDMain,"f1",InBuf)
            f2 = GetInputField(FormIDMain,"f2",InBuf)
            f3 = GetInputField(FormIDMain,"f3",InBuf)
            If f1 <> " " Then FillField FormIDResult,"field1", Buf, "one"
            If f2 <> " " Then FillField FormIDResult,"field2", Buf, "two"
            If f3 <> " " Then FillField FormIDResult,"field3", Buf, "three"
         Else
            ' Unrecognized form--send the initial form.
            Buf = InitFormBuf(FormIDMain)
            FillField FormIDMain,"FORM", Buf,"MAIN"
            SetDateAndTime FormIDMain,Buf
         End If         
         retval = SendMCSBuf(Header, Buf)
   End Select
Loop

