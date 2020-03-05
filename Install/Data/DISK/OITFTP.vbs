' OITFTP.vbs - Partially simulate OpenIT FTP for BurSim
' Mark Riordan 10 Sep 1999

Function Format2(MyVal)
   str1 = FormatNumber(MyVal,0,TristateTrue,TristateFalse,TristateFalse)
   If MyVal < 10 Then str1 = "0" & str1
   Format2 = str1
End Function

' Format the date and time like: 09-05 12:05:25
CurDate = Date
MyDate = Format2(Month(CurDate)) & "-" & Format2(Day(CurDate))
CurTime = Time
MyTime = Format2(Hour(CurTime)) & ":" & Format2(Minute(CurTime)) & _
 ":" & Format2(Second(CurTime))
SPOMsg "OpenIT FTP v. 4.0.2.8     30000 started " & MyDate & " " & MyTime
SPOMsg "Using:  OpenIT IntLib v. 2.0.3 (SNIBIO)"

Randomize

Const WAIT_TIMEOUT=1, WAIT_ODT=2, WAIT_STOQ=3
Do
   which = Wait(rnd*900, "FLDTST", STOQName)
   Select Case which
      Case WAIT_TIMEOUT
         filename = "RPT" & FormatNumber(Int(Rnd*10),0)
         SPOMsg "RETR " & filename & " ON PACK DATA"
      Case WAIT_ODT
         Accept ODTMsg
         If "PQUIT" = ODTMSG Then Exit Do
         SPOMsg "Unrecognized command: " & ODTMsg
   End Select
Loop