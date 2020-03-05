   Const ForReading = 1, ForWriting = 2, ForAppending = 8

   Set fso = CreateObject("Scripting.FileSystemObject")
   Set logfile = fso.OpenTextFile("\BurSimAx.Log", ForAppending, True)
   line = Time & " " & Date & " BurSim script starting! " 
   logfile.WriteLine(line)
   logfile.Close
