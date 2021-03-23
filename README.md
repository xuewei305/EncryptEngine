# EncryptEngine
This is a Driver Program base on Windows System,which purpose is to encrypt file of Office File as transparent way.

Summarization

Hi Guys,this is a Driver Program base on Windows System,which purpose is to encrypt file of Office File as transparent way.

Explanation Of Folder

user:		R3 Files as exe
engine:		R0 Files as sys 
RestoreTool:	Usage to Decrypt Document Files encrypted by engine.sys

1.Steps For Compilation the whole program :
	
	For user.exe with "build" Command in Console：

		C:\Windows\System32\cmd.exe /k C:\WinDDK\7600.16385.1\bin\setenv.bat C:\WinDDK\7600.16385.1\ chk x86 WIN7
	
	For engine.sys with "build" Command in Console：
		
		C:\Windows\System32\cmd.exe /k C:\WinDDK\7600.16385.1\bin\setenv.bat C:\WinDDK\7600.16385.1\ chk x64 WIN7	
	
	For RestoreTool.sys:
		
		to compile it in Visual Studio Directly.
		
		Steps to Enabling test mode:

			1. Open CMD with administrative privileges.
			2. Type: bcdedit -set TESTSIGNING ON
			3. Reboot computer.

2.Steps For Testing the program:

	Firstly:
		install & deploy engine.sys to Directory of system32\drivers by engine\engine.inf

	Secondly:
		Start Driver by "net start engine" in console as administrator

	Thirdly:
		Add Monitoring Process list to engine.sys launched by running "user.exe" in console as administrator

3.Recommendation ConductingEvery Testing in VMware Workstation.

	The one more thihg is to lose efficacy if you save document to C Disk.
	
Finally I Hope you can enjoy this , but don't to try using it to commercial circumstance ......
