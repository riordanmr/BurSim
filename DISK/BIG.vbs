' BIG.vbs - Simple script to test 34-line screens
' Mark Riordan  5 January 2000

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

FormFileID = OpenFormFile("BIG.FRM")
FormIDMain = OpenForm(FormFileID,"MAIN")
Dim STOQ, ODTMsg, Buf

Const WAIT_TIMEOUT=1, WAIT_ODT=2, WAIT_STOQ=3
bRunning = true

Do While bRunning
   which = Wait(9999,"BIG",STOQ)
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
         retval = GetSTOQMsg("BIG", Header, InBuf)
         InputForm = GetInputField(FormIDMain,"FORM",InBuf)
         bProcessed = False
         If False And "MAIN" = InputForm Then
            'ProcessMainInput Header,InBuf
            bProcessed = True
         End If
         If Not bProcessed Then
            Buf = InitFormBuf(FormIDMain)
            FillField FormIDMain,"FORM", Buf,"MAIN"
            SetDateAndTime FormIDMain,Buf
         End If         
         retval = SendMCSBuf(Header, Buf)
   End Select
Loop

