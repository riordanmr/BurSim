' TPEDECHO.VBS - Here we go again!
' Anonymous   26 Sept 1999


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

Sub SetDateAndTime(FormID,Buf)
   Dim MyStr
   MyStr = MakeDate(Date) & " " & MakeTime(Time)
   FillField FormID,"DateAndTime",Buf,MyStr
End Sub

Sub ProcessMainInput(Header, InBuf)
   If " " <> GetInputField(FormIDMain,"Add_Swimmer",InBuf) Then
      FormID = FormIDEdit
      Buf = InitFormBuf(FormID)
      FillField FormID,"FORM", Buf,"EDIT"
      SetDateAndTime FormID,Buf
      FillField FormID,"Operation",Buf,"Adding a new swimmer"
   ElseIf "" <> Trim(GetInputField(FormIDMain,"Search",InBuf)) Then
      SQL = "SELECT * FROM Swimmers"
      LastName = Trim(GetInputField(FormIDMain,"LASTNAME",InBuf))
      SWNum = Trim(GetInputField(FormIDMain,"SWNum",InBuf))
      If "" <> LastName Then
         SQL = SQL & " WHERE Swimmers.LastName Like '" & LastName & "%'"
      ElseIf "" <> SWNum Then
         SQL = SQL & " WHERE SwimmerID = '" & SWNum & "'"
      End If
      Set RS1 = CreateObject("ADODB.Recordset" )
      Connect = "Provider=Microsoft.Jet.OLEDB.3.51;Persist Security Info=False;Data Source=SWIM.mdb"
      RS1.Open SQL, Connect, adOpenStatic
      'SPOMsg "SQL=" & SQL & "; RCnt=" & RS1.RecordCount
      RecNum = 1
      SwimOrdinal = 1
      MaxPerScreen = 15
      FormID = FormIDSearchResults
      Buf = InitFormBuf(FormID)
      SetDateAndTime FormID,Buf
      FillField FormID,"FORM",Buf,"SRES"
      Do Until RS1.Eof Or SwimOrdinal>MaxPerScreen
         FillField FormID,"N-"&SwimOrdinal,Buf,RS1("SwimmerID")
         FillField FormID,"LastName-"&SwimOrdinal,Buf,RS1("LastName")
         FillField FormID,"FirstName-"&SwimOrdinal,Buf,RS1("FirstName")
         RecNum = RecNum + 1
         SwimOrdinal = SwimOrdinal + 1
         RS1.MoveNext
      Loop
      ' Suppress any unused checkboxes.
      Do While SwimOrdinal <= MaxPerScreen
         SuppressField FormID,"X-"&SwimOrdinal,Buf
         SwimOrdinal = SwimOrdinal + 1
      Loop
      ' If we got exactly one swimmer, don't bother to show
      ' screen of search results--just show the swimmer.
      If RecNum = 2 Then
         FormID = FormIDEditSwimmer
         Buf = InitFormBuf(FormID)
         SetDateAndTime FormID,Buf
         FillField FormID,"FORM",Buf,"EDIT"
         RS1.MovePrevious
         FillField FormID,"SWNum",Buf,RS1("SwimmerID")
         FillField FormID,"LastName",Buf,RS1("LastName")
         FillField FormID,"FirstName",Buf,RS1("FirstName")
         FillField FormID,"Address",Buf,RS1("Address")
         FillField FormID,"City",Buf,RS1("City")
         FillField FormID,"State",Buf,RS1("State")
         FillField FormID,"Zip",Buf,RS1("Zip")
         MyPhone = RS1("Phone")
         FillField FormID,"Phone",Buf,RS1("Phone")
      End If
   End If
End Sub

FormFileID = OpenFormFile("SWIM.FRM")
FormIDMain = OpenForm(FormFileID,"MAIN")
FormIDEdit = OpenForm(FormFileID,"EditSwimmer")
FormIDSearchResults = OpenForm(FormFileID,"SearchResults")
FormIDEditSwimmer = OpenForm(FormFileID,"EditSwimmer")
Dim STOQ, ODTMsg, Buf
Dim RS1

Const WAIT_TIMEOUT=1, WAIT_ODT=2, WAIT_STOQ=3
bRunning = true

Do While bRunning
   which = Wait(9999,"TD8-ECHO",STOQ)
   Select Case which
      Case WAIT_TIMEOUT
      Case WAIT_ODT
         Accept ODTMsg
         If "QUIT" = ODTMsg Then 
            SPOMsg "QUIT?! .... NO! I don't wanna!"
         ElseIf "STOP" = ODTMsg Then
            bRunning = false
         Else
            SPOMsg ODTMsg & "?  Please Stop AXing me that!"  
         End If
      Case WAIT_STOQ
         retval = GetSTOQMsg("TD8_ECHO", Header, InBuf)
         bProcessed = False
         If "DONE" = InputForm Then
            ProcessMainInput Header,InBuf
            bProcessed = True
         ElseIf "USER" = InputForm Then
            If " " <> GetInputField(FormID2,"X",InBuf) Then
               Buf = InitFormBuf(FormIDLogOff)
               FillField FormIDLogOff,"FORM",Buf,"LOGO"
               bProcessed = True
            End If
         End If
         If Not bProcessed Then
            Buf = InitFormBuf(FormIDMain)
            FillField FormIDMain,"FORM", Buf,"MAIN"
            SetDateAndTime FormIDMain,Buf
         End If         
         retval = SendMCSBuf(Header, Buf)
   End Select
Loop

