' OITMGR.vbs - Simulates OpenIT FTPMGR
' Mark Riordan   26 Sept 1999

Function Format2(MyVal)
   str1 = FormatNumber(MyVal,0,TristateTrue,TristateFalse,TristateFalse)
   If MyVal < 10 Then str1 = "0" & str1
   Format2 = str1
End Function

Function MakeTime(CurTime)
   MakeTime = Format2(Hour(CurTime)) & ":" & Format2(Minute(CurTime)) & _
     ":" & Format2(Second(CurTime))
End Function

Function MakeDate(CurDate)
   MakeDate = Year(CurDate) & "/" & Format2(Month(CurDate)) & "/" & Format2(Day(CurDate))
End Function

Sub ShowStatus()
   ' Format the date and time like 1999/09/03 16:00:04
   SPOMsg "OITMGR status as of "& MakeDate(Date) & " " & MakeTime(Time) 
   SPOMsg "No active sessions."
   SPOMsg "No transfers scheduled."
End Sub

SPOMsg "FTPMGR v3.39"

Const WAIT_TIMEOUT=1, WAIT_ODT=2, WAIT_STOQ=3
Do
   which = Wait(5, "OITMGR", STOQName)
   Select Case which
      Case WAIT_TIMEOUT
         NewHour = Hour(Time)
         If NewHour <> OldHour Then
            ShowStatus
            OldHour = NewHour
         End If
      Case WAIT_ODT
         Accept ODTMsg
         If "QUIT" = ODTMSG Then Exit Do
   End Select
Loop
