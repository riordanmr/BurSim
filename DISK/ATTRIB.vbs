' ATTRIB.vbs - Simple script to test 34-line screens
' Mark Riordan  5 January 2000

FormFileID = OpenFormFile("ATTRIB.FRM")
FormIDMain = OpenForm(FormFileID,"MAIN")
Dim STOQ, ODTMsg, Buf

Const WAIT_TIMEOUT=1, WAIT_ODT=2, WAIT_STOQ=3
bRunning = true

Do While bRunning
   which = Wait(9999,"ATTRIB",STOQ)
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
         retval = GetSTOQMsg("ATTRIB", Header, InBuf)
         InputForm = GetInputField(FormIDMain,"FORM",InBuf)
         bProcessed = False
         If Not bProcessed Then
            Buf = InitFormBuf(FormIDMain)
            FillField FormIDMain,"FORM", Buf,"MAIN"
         End If         
         retval = SendMCSBuf(Header, Buf)
   End Select
Loop

