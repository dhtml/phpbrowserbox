Set WshShell = CreateObject("WScript.Shell") 
WshShell.Run "..\phpbrowserbox.exe /doctor", 1, True

Set WshShell = Nothing