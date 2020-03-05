' SNISWIM.vbs - Implement the "mainframe" side of a 
' CoolICE demo.
' Mark Riordan  11 January 2000

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
End Sub

Sub SetTime(FormID,Buf)
   FillField FormID, "Time", Buf, MakeTime(Time)
End Sub

Sub AddSwimmer(SwimStr)
   Swimmers(NSwimmers) = SwimStr
   NSwimmers = NSwimmers+1
End Sub

Sub InitSwimmers
   AddSwimmer "10001|Lukey|Mike|345 Chelsea Pass||Madison|WI|53711|608-234-5541|12/30/1984|"
   AddSwimmer "10002|Frasier|Sally|1725 Franklin St||Madison|WI|53706|608-778-3092|09/13/1983|"
   AddSwimmer "10003|Bosko|Elizabeth|344 Lafayette Dr||Middleton|WI|53717|608-266-5028|05/31/1983|"
End Sub

Function GetSwimmerByID(SwimmerID, SwLName, SwFName, SwAddr1, SwAddr2, SwCity, SwState, SwZip, SwPhone, SwBirthDt)
   For j = 0 To NSwimmers-1
      If SwimmerID = Left(Swimmers(j),InStr(Swimmers(j),"|")-1) Then
         idx = InStr(Swimmers(j),"|") + 1
         idxnext = InStr(idx, Swimmers(j),"|")

         SwLName = Mid(Swimmers(j),idx, idxnext-idx)
         idx = idxnext+1
         idxnext = InStr(idx, Swimmers(j),"|")
         SwFName = Mid(Swimmers(j),idx, idxnext-idx)

         idx = idxnext+1
         idxnext = InStr(idx, Swimmers(j),"|")
         SwAddr1 = Mid(Swimmers(j),idx, idxnext-idx)

         idx = idxnext+1
         idxnext = InStr(idx, Swimmers(j),"|")
         SwAddr2 = Mid(Swimmers(j),idx, idxnext-idx)

         idx = idxnext+1
         idxnext = InStr(idx, Swimmers(j),"|")
         SwCity = Mid(Swimmers(j),idx, idxnext-idx)

         idx = idxnext+1
         idxnext = InStr(idx, Swimmers(j),"|")
         SwState = Mid(Swimmers(j),idx, idxnext-idx)

         idx = idxnext+1
         idxnext = InStr(idx, Swimmers(j),"|")
         SwZip = Mid(Swimmers(j),idx, idxnext-idx)

         idx = idxnext+1
         idxnext = InStr(idx, Swimmers(j),"|")
         SwPhone = Mid(Swimmers(j),idx, idxnext-idx)

         idx = idxnext+1
         idxnext = InStr(idx, Swimmers(j),"|")
         SwBirthDt = Mid(Swimmers(j),idx, idxnext-idx)

         GetSwimmerByID = j
         Exit Function
      End If
   Next
   GetSwimmerByID = -1
End Function

Sub ProcessLogin(InBuf)
   Dim Usercode, Password
   Usercode = Trim(GetInputField(FormIDMain,"Usercode",InBuf))
   Password = Trim(GetInputField(FormIDMain,"Password",InBuf))
   If (Usercode="SNISWIM" And Password="SNISWIM") Or _
     Usercode="S" Or Usercode = "SNISWIM SNISWIM" Then
      Buf = InitFormBuf(FormIDFirstMenu)
      SetTime FormIDFirstMenu,Buf
      FillField FormIDFirstMenu, "FORM", Buf, "MENU"
   Else
      Buf = InitFormBuf(FormIDMain)
      SetTime FormIDMain,Buf
      FillField FormIDMain,"BottomMsg1", Buf,"The USERCODE or PASSWORD you have entered is not valid. Please reenter your"
      FillField FormIDMain,"BottomMsg2", Buf,"USERCODE and PASSWORD or see your system administrator for assistance."
   End If
End Sub

Sub ProcessFirstMenu(InBuf)
   If " " <> GetInputField(FormIDFirstMenu,"Query",InBuf) Then
      Buf = InitFormBuf(FormIDQueryMenu)
      SetTime FormIDQueryMenu,Buf
      FillField FormIDQueryMenu, "FORM", Buf, "QUERYM"
   ElseIf " " <> GetInputField(FormIDFirstMenu,"Exit",InBuf) Then
      Buf = InitFormBuf(FormIDMain)
      SetTime FormIDMain,Buf
   ElseIf " " <> GetInputField(FormIDFirstMenu, "Update",InBuf) Then
      Buf = InitFormBuf(FormIDUpdateMenu)
      SetTime FormIDUpdateMenu,Buf
      FillField FormIDQueryMenu, "FORM", Buf, "UPDMEN"
   Else
      SetTime FormIDFirstMenu,Buf
   End If
End Sub

Sub DoQuery(SwimmerID)
   Buf = InitFormBuf(FormIDQueryResults)
   SetTime FormIDQueryResults,Buf
   FillField FormIDQueryResults, "FORM", Buf, "QUERYR"
   Dim SwLName, SwFName, SwAddr1, SwAddr2, SwCity, SwState, SwZip, SwPhone, SwBirthDt
   If GetSwimmerByID(SwimmerID, SwLName, SwFName, SwAddr1, SwAddr2, SwCity, SwState, _
     SwZip, SwPhone, SwBirthDt) >= 0 Then
      FillField FormIDQueryResults, "SwmID", Buf, SwimmerID
      FillField FormIDQueryResults, "LastName", Buf, SwLName
      FillField FormIDQueryResults, "FirstName", Buf, SwFName
      FillField FormIDQueryResults, "Address1", Buf, SwAddr1
      FillField FormIDQueryResults, "Address2", Buf, SwAddr2
      FillField FormIDQueryResults, "City", Buf, SwCity
      FillField FormIDQueryResults, "ST", Buf, SwState
      FillField FormIDQueryResults, "ZipCode", Buf, SwZip
      FillField FormIDQueryResults, "Phone", Buf, SwPhone
      FillField FormIDQueryResults, "BirthDt", Buf, SwBirthDt
   Else
      FillField FormIDQueryResults, "Results", Buf, "No swimmer was found for ID " & SwimmerID
   End If
End Sub

Sub ProcessQueryMenu(InBuf)
   If " " <> GetInputField(FormIDQueryMenu,"Previous",InBuf) Then
      Buf = InitFormBuf(FormIDFirstMenu)
      SetTime FormIDFirstMenu,Buf
      FillField FormIDFirstMenu, "FORM", Buf, "MENU"
   ElseIf " " <> GetInputField(FormIDQueryMenu,"QueryByID",InBuf) Then
      SwimmerID = GetInputField(FormIDQueryMenu,"SwmID",InBuf)
      DoQuery SwimmerID
   Else
      SetTime FormIDQueryMenu,Buf
   End If
End Sub

Sub ProcessQueryResults(InBuf)
   If " " <> GetInputField(FormIDQueryResults,"Previous",InBuf) Then
      Buf = InitFormBuf(FormIDQueryMenu)
      SetTime FormIDQueryMenu,Buf
      FillField FormIDQueryMenu, "FORM", Buf, "QUERYM"
   Else
      SetTime FormIDQueryResults,Buf
   End If
End Sub

Sub ShowUpdateScreen(SwimmerID)
   Buf = InitFormBuf(FormIDUpdateInput)
   SetTime FormIDUpdateInput,Buf
   FillField FormIDUpdateInput, "FORM", Buf, "UPDINP"
   Dim SwLName, SwFName, SwAddr1, SwAddr2, SwCity, SwState, SwZip, SwPhone, SwBirthDt
   If GetSwimmerByID(SwimmerID, SwLName, SwFName, SwAddr1, SwAddr2, SwCity, SwState, _
     SwZip, SwPhone, SwBirthDt) >= 0 Then
      FillField FormIDUpdateInput, "SwmID", Buf, SwimmerID
      FillField FormIDUpdateInput, "LastName", Buf, SwLName
      FillField FormIDUpdateInput, "FirstName", Buf, SwFName
      FillField FormIDUpdateInput, "Address1", Buf, SwAddr1
      FillField FormIDUpdateInput, "Address2", Buf, SwAddr2
      FillField FormIDUpdateInput, "City", Buf, SwCity
      FillField FormIDUpdateInput, "ST", Buf, SwState
      FillField FormIDUpdateInput, "ZipCode", Buf, SwZip
      FillField FormIDUpdateInput, "Phone", Buf, SwPhone
      FillField FormIDUpdateInput, "BirthDt", Buf, SwBirthDt
   Else
      FillField FormIDUpdateInput, "Results", Buf, "No swimmer was found for ID " & SwimmerID
   End If

End Sub

Sub ProcessUpdateMenu(InBuf)
   If " " <> GetInputField(FormIDUpdateMenu,"Previous",InBuf) Then
      Buf = InitFormBuf(FormIDFirstMenu)
      SetTime FormIDFirstMenu,Buf
      FillField FormIDFirstMenu, "FORM", Buf, "MENU"
   ElseIf " " <> GetInputField(FormIDUpdateMenu,"QueryByID",InBuf) Then
      SwimmerID = GetInputField(FormIDUpdateMenu,"SwmID",InBuf)
      ShowUpdateScreen SwimmerID
   Else
      SetTime FormIDUpdateMenu,Buf
   End If
End Sub

Sub ProcessUpdateInput(InBuf)
   If " " <> GetInputField(FormIDUpdateInput,"Previous",InBuf) Then
      Buf = InitFormBuf(FormIDUpdateMenu)
      SetTime FormIDUpdateMenu,Buf
      FillField FormIDUpdateMenu, "FORM", Buf, "UPDMEN"
   Else
      SetTime FormIDUpdateInput,Buf
      Dim SwLName, SwFName, SwAddr1, SwAddr2, SwCity, SwState, SwZip, SwPhone, SwBirthDt
      SwimmerID = GetInputField(FormIDUpdateInput,"SwmID",InBuf)
      idx = GetSwimmerByID(SwimmerID, SwLName, SwFName, SwAddr1, SwAddr2, SwCity, SwState, _
        SwZip, SwPhone, SwBirthDt)
      If idx >= 0 Then
         MyStr = SwimmerID & "|" & _
          Trim(GetInputField(FormIDUpdateInput,"LastName",InBuf)) & "|" & _
          Trim(GetInputField(FormIDUpdateInput,"FirstName",InBuf)) & "|" & _
          Trim(GetInputField(FormIDUpdateInput,"Address1",InBuf)) & "|" & _
          Trim(GetInputField(FormIDUpdateInput,"Address2",InBuf)) & "|" & _
          Trim(GetInputField(FormIDUpdateInput,"City",InBuf)) & "|" & _
          Trim(GetInputField(FormIDUpdateInput,"ST",InBuf)) & "|" & _
          Trim(GetInputField(FormIDUpdateInput,"ZipCode",InBuf)) & "|" & _
          Trim(GetInputField(FormIDUpdateInput,"Phone",InBuf)) & "|" & _
          Trim(GetInputField(FormIDUpdateInput,"BirthDt",InBuf)) & "|"
         Swimmers(idx) = MyStr
         ShowUpdateScreen SwimmerID         
         FillField FormIDUpdateInput, "Results", Buf, "Update complete."
      End If
   End If

End Sub

FormFileID = OpenFormFile("SNISWIM.FRM")
FormIDMain = OpenForm(FormFileID,"MAIN")
FormIDFirstMenu = OpenForm(FormFileID,"FirstMenu")
FormIDQueryMenu = OpenForm(FormFileID,"QueryMenu")
FormIDQueryResults = OpenForm(FormFileID,"QueryResults")
FormIDUpdateMenu   = OpenForm(FormFileID,"UpdateMenu")
FormIDUpdateInput  = OpenForm(FormFileID,"UpdateInput")

Dim STOQ, ODTMsg, Buf, Form
Dim Swimmers(100), NSwimmers

InitSwimmers

Const WAIT_TIMEOUT=1, WAIT_ODT=2, WAIT_STOQ=3
bRunning = true

Do While bRunning
   which = Wait(9999,"SNISWIM",STOQ)
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
         retval = GetSTOQMsg("SNISWIM", Header, InBuf)
         'SPOMsg "Len(InBuf)=" & Len(InBuf) & ":" & InBuf
         If Len(InBuf)>7 And Left(InBuf,8) = "/SNISWIM" Then
            Buf = InitFormBuf(FormIDMain)
            SetTime FormIDMain,Buf
         ElseIf Len(InBuf) = 38 Then
            ProcessLogin InBuf
         Else
            Form = Trim(GetInputField(FormIDFirstMenu,"FORM",InBuf))
            If Form = "MENU" Then
               ProcessFirstMenu InBuf
            ElseIf Form = "QUERYM" Then
               ProcessQueryMenu InBuf
            ElseIf Form = "QUERYR" Then
               ProcessQueryResults InBuf
            ElseIf Form = "UPDMEN" Then
               ProcessUpdateMenu InBuf
            ElseIf Form = "UPDINP" Then
               ProcessUpdateInput InBuf
            Else
               SPOMsg "Invalid screen: '" & Form & "'"
               Buf = InitFormBuf(FormIDMain)
               SetTime FormIDMain,Buf
            End If
         End If         
         retval = SendMCSBuf(Header, Buf)
   End Select
Loop

