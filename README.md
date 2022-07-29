# ncAuth ~ Terminal TOTP Authenticator
A minimalist, semi-scriptable authenticator app for SXMO, written in the span of a couple days.


## Usage
### CLI
ncAuth acceps a few command line arguments to allow for automated usage.

`--help`  
Prints the help page & exits.

`--code [name] [code]`  
Allows you to specify a 2FA code to use only in this session, where `name` is the name to display & `code` is the secret key.

`--file [file]`  
Specifies a secrets file, which if unspecified reads `secrets.tsv` in the program's install directory.  
The format for each line in the file is `NAME\tSECRET`.  
If `file` is "none", no file will be loaded & no codes will be displayed unless codes are defined as arguments.

`--print`  
Prints all account names & generated codes, instead of running in interactive mode with ncurses, might be useful for automation.  
The format for each code is `NAME\tCODE`.  

### GUI
In the interactive GUI, you will see a list of your 2FA codes. The selected row has a progressbar of when the codes will switch.

`K` & `J`  
Select the above or below row. If you have more 2FA codes that can fit on your screen, they will scroll with the selection.

`C`  
Cycles the app colour/theme between your terminal colours. Will be saved to `colour` in the install directory only if the program is quit with `Q`.

`S`  
Runs the script `script.sh` in the install directory, passing the selected row's name & displayed code.  
This could be used for example to copy the code to the clipboard, but as clipboards are very WM-dependent, you'll have to write the script yourself.  
A workaround for SXMO (since nothing supports its clipboard) is to call `xdotool` to type the code after a few seconds.

`E`  
Runs the script `edit.sh` in the install directory, passing the current file name.  
Used to edit the current secrets file by opening an editor, because text input is hard with only `getch()`.  
Will block the program until it's closed, and will not do anything if the file is "none".

`N`  
Shows the next 30 second interval's codes instead of the current one, in italics. Also passes the next code to `script.sh`.  
Is automatically disabled when the next codes become current.

`Q`  
dies lol


## Building
Dependencies:
- `libncurses`
- Standard C libraries

Then run:  
`gcc src/main.c -lncurses -o ncauth`


## Notes
Actual HOTP code written by [this nigga](https://github.com/RealAlphabet/GoogleAuthentificatorC), modified bc idk what a makefile is
