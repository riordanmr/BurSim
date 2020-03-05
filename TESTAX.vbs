' TESTAX.vbs - Test ODT input   /mrr  6 Sep 1999
SPOMsg "TestAX running"
Do
   Accept MyText
   If MyText = "QUIT" Then Exit Do
   If IsNumeric(MyText) Then
      SPOMsg "The square root of " & MyText & " is " & SQR(MyText)
   Else
      SPOMsg "I can't get the square root of '" & MyText & "'"
   End If
Loop 