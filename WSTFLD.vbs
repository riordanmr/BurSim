' WSTFLD.vbs - Westfield Electronics Inventory System
'
' Mark Riordan   6 Sep 1999

Const WAIT_TIMEOUT=1, WAIT_ODT=2, WAIT_STOQ=3
Esc = Chr(27)
Etx = Chr(3)
BegF = Chr(31)
EndF = Chr(30)
BegProt = Chr(28)
Bright = Chr(26)
MsgStart = Esc & "X" & Chr(12)
MsgEnd   = Esc & "W" & Etx
Dim ScreenMain, ScreenExit, ScreenBrowse, ScreenMod
ScreenMain = MsgStart
ScreenMain = ScreenMain & Make80(Bright & "Welcome to the Westfield Electronics Inventory System                    " & BegProt & "MAIN" & EndF) & Make80(" ")
ScreenMain = ScreenMain & Make80(" Make your selection with an X:") & Make80("")
ScreenMain = ScreenMain & Make80(" " & BegF & " " & EndF & " Browse products")
ScreenMain = ScreenMain & Make80(" " & BegF & " " & EndF & " Exit")
ScreenMain = ScreenMain & MsgEnd

ScreenExit = MsgStart & Make80("Good-bye from the Westfield Electronics Inventory System")
ScreenExit = ScreenExit & MsgEnd

Dim ItemNum(100), ItemQuan(100), ItemDesc(100)

ItemNum(0) = "A001"
ItemQuan(0) = "0003"
ItemDesc(0) = "Non-working Unisys Micro-A"
ItemNum(1) = "P286"
ItemQuan(1) = "0531"
ItemDesc(1) = "Non-working PC motherboard, 286"
ItemNum(2) = "P386"
ItemQuan(2) = "0122"
ItemDesc(2) = "Non-working PC motherboard, 386"
ItemNum(3) = "P486"
ItemQuan(3) = "0017"
ItemDesc(3) = "Non-working PC motherboard, 486"
ItemNum(4) = "P586"
ItemQuan(4) = "0000"
ItemDesc(4) = "Functional PC motherboard, Pentium"

NItems = 5

Do 
   which = Wait(9999, "WSTFLD", STOQName)
   Select Case which
      Case WAIT_TIMEOUT
         SPOMsg "Wait timed out"
      Case WAIT_ODT
         Accept ODTMsg
         If "QUIT" = ODTMSG Then Exit Do
         SPOMsg "Unrecognized command: " & ODTMsg
      Case WAIT_STOQ
         retval = GetSTOQMsg("WSTFLD", Header, Buf)
         SPOMsg "Received " & Len(buf) & " byte msg from stn " & Left(Header,6) & ": " & Buf
         Screen = Left(Buf,4)
         
         If Left(Buf,1) = "/" Then
            retval = SendMCSBuf(Header, ScreenMain)
          ElseIf Screen = "MAIN" Then
            If Mid(Buf,5,1)<>" " Then 
               SPOMsg "Sending browse screen"
               MakeBrowse
               retval = SendMCSBuf(Header,ScreenBrowse)
            ElseIf Mid(Buf,6,1) <> " " Then
               SPOMsg "Sending exit screen"
               retval = SendMCSBuf(Header,ScreenExit)
               
            End If
         ElseIf Screen = "BROW" Then
            ModItem = -1
            For j = 0 to NItems-1 
               If Mid(Buf,5+j,1) <> " " Then
                  ModItem = j      
               End If
            Next
            If ModItem >= 0 Then 
               'Send screen with product to modify
               MakeModScreen(ModItem)
               retval = SendMCSBuf(Header, ScreenMod)
            ElseIf Mid(Buf,4+NItems) <> " " Then
               retval = SendMCSBuf(Header, ScreenMain)
            End If
         ElseIf Screen = "MODI" Then
            iitem = Mid(Buf,5,4)
            SPOMsg "Modifying item " & iitem
            ItemNum(iitem) = Mid(Buf,9,4)
            ItemDesc(iitem) = Mid(Buf,13,40)
            ItemQuan(iitem) = Mid(buf,53,4)
            MakeBrowse
            retval = SendMCSBuf(Header,ScreenBrowse)
         End If
   End Select
Loop 

Function Make80 (mystr)
   If Len(mystr) > 80 Then
      Make80 = Left(mystr,80)
   Else 
      Make80 = mystr & String(80-Len(mystr)," ")
   End If
End Function

Function MakeLen(mystr, mylen)
   If Len(mystr) > mylen Then
      MakeLen = Left(mystr, mylen)
   Else 
      MakeLen = mystr & String(mylen-Len(mystr), " ")
   End If
End Function

Sub MakeBrowse()
   ScreenBrowse = MsgStart
   ScreenBrowse = ScreenBrowse & Make80(Bright & "Westfield Electronics - List of products                                 " & BegProt & "BROW" & EndF)
   ScreenBrowse = ScreenBrowse & Make80("")
   ScreenBrowse = ScreenBrowse & Make80(" Enter X before a product to update it.")
   ScreenBrowse = ScreenBrowse & Make80("")
   ScreenBrowse = ScreenBrowse & Make80("              # in             ")
   ScreenBrowse = ScreenBrowse & Make80("     Prod #  Stock  Description")
   '                                      [ ]   xxxx     xxxx  xxxxxxxxxxx
   
   For j = 0 to NItems-1
      line = " " & BegF & " " & EndF & "   " & ItemNum(j) & "   " & ItemQuan(j) & "  " & ItemDesc(j)
      ScreenBrowse = ScreenBrowse & Make80(line)
   Next
   ScreenBrowse = ScreenBrowse & Make80("")
   ScreenBrowse = ScreenBrowse & Make80("")
   ScreenBrowse = ScreenBrowse & Make80("")
   ScreenBrowse = ScreenBrowse & Make80(" Enter X to return to main: " & BegF & " " & EndF)
   ScreenBrowse = ScreenBrowse & MsgEnd
End Sub

Sub MakeModScreen(iitem)
   ScreenMod = MsgStart
   strItemNum = iitem
   Do While Len(strItemNum) < 4
      strItemNum = "0" & strItemNum
   Loop
   ScreenMod = ScreenMod & Make80(Bright & "Westfield Electronics - Enter/Update product                         " & BegProt & "MODI" & strItemNum & EndF)
   ScreenMod = ScreenMod & Make80("")   
   ScreenMod = ScreenMod & Make80(" Part number:      " & BegF & ItemNum(iitem) & EndF)   
   ScreenMod = ScreenMod & Make80(" Part description: " & BegF & MakeLen(ItemDesc(iitem),40) & EndF)   
   ScreenMod = ScreenMod & Make80(" Number in stock:  " & BegF & ItemQuan(iitem) & EndF)   
   ScreenMod = ScreenMod & Make80("")   
   ScreenMod = ScreenMod & Make80("")   
   ScreenMod = ScreenMod & Make80("")   
   ScreenMod = ScreenMod & Make80("")   
   ScreenMod = ScreenMod & Make80("")   
   ScreenMod = ScreenMod & Make80("")   
   ScreenMod = ScreenMod & Make80("")   
   ScreenMod = ScreenMod & MsgEnd   
End Sub

