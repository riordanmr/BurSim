   Const ForReading = 1, ForWriting = 2, ForAppending = 8

   Set fso = CreateObject("Scripting.FileSystemObject")
   Set logfile = fso.OpenTextFile("\BurSimAx.Log", ForAppending, True)
   line = Time & " " & Date & " BurSim script here. " 
   logfile.WriteLine(line)
   logfile.Close
   Dim MyMsg
   For j= 1 to 4
     SPOMsg j & "=" & Time & " " & Date & " BurSim script here. "
     waittype = WaitForMsg(2,MyMsg)
   Next