# LH_Injector
Loop Hero Injector lets you inject mods for Loop Hero.

Comes with discord presence for loop hero.

MD5 hash for current release:  7a4000d8a3ef66dca607c51fa514035a 
## Disclaimer
DLL files (.dll) contain compiled code, just like other programs. If you do not trust software, do not run it on your PC. The files I created are harmless, but I have no control over other people's mods. Stay safe!

# Usage / Setup

drag and drop "Injector.exe" , "GMRPC.dll" and "discord-rpc.dll" in the gamefolder, where Loop Hero.exe is.

To start Loop Hero *unmodded*, double click on Loop Hero.exe

To start Loop Hero *modded*, launch "Injector.exe".

- This will launch Loop Hero and then inject whatever file you pass as first startup parameter.
- The first startup parameter defaults to "raw", meaning it won't do anything except GMRPC.dll and discord-rpc.dll

*Don't be scared of the **terminal window** that pops up. This is mainly used for debug printing and quitting the whole thing (press 'q')*

# Passing other mods
If you have a custom dll / mod you want to load, pass the relative filepath to that file *(example: mymod.dll, where mymod.dll resides in the same folder as Injector.exe)*
- To make life easier, I recommend creating shortcuts with startup parameters for Injector.exe so you don't need to use the terminal for this everytime.

# Unistalling

To uninstall, delete all your downloaded .dll files and Injector.exe
The game's files aren't modified by the Injector.
