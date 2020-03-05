' FLDTST.vbs - Test form fields
FormFileID = OpenFormFile("FLDTST.txt")
FormID = OpenForm(FormFileID,"Form1")
FormID2 = OpenForm(FormFileID,"Form2")
FormIDLogOff = OpenForm(FormFileID,"LogOff")
SPOMsg "FormFileID = " & FormFileID & " FormID = " & FormID

Const WAIT_TIMEOUT=1, WAIT_ODT=2, WAIT_STOQ=3
Do
   which = Wait(9999, "FLDTST", STOQName)
   Select Case which
      Case WAIT_TIMEOUT
         SPOMsg "Wait timed out"
      Case WAIT_ODT
         Accept ODTMsg
         If "QUIT" = ODTMSG Then Exit Do
         SPOMsg "Unrecognized command: " & ODTMsg
      Case WAIT_STOQ
         retval = GetSTOQMsg("FLDTST", Header, InBuf)
         InputForm = GetInputField(FormID,"FORM",InBuf)
         bProcessed = False
         If "MAIN" = InputForm Then
            Buf = InitFormBuf(FormID2)
            FillField FormID,"FORM", Buf,"USER"
            FillField FormID2,"Username",Buf, GetInputField(FormID,"UserName",InBuf)
            FillField FormID2,"Password",Buf, GetInputField(FormID,"Password",InBuf)
            bProcessed = True
         ElseIf "USER" = InputForm Then
            If " " <> GetInputField(FormID2,"X",InBuf) Then
               Buf = InitFormBuf(FormIDLogOff)
               FillField FormIDLogOff,"FORM",Buf,"LOGO"
               bProcessed = True
            End If
         End If
         If Not bProcessed Then
            Buf = InitFormBuf(FormID)
            FillField FormID,"FORM", Buf,"MAIN"
            FillField FormID,"WelcomeMsg", Buf, "Hi there"
            FillField FormID,"Time", Buf, Time
         End If         
         retval = SendMCSBuf(Header, Buf)
   End Select
Loop
