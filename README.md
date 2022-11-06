# ncAuth ~ Terminal TOTP Authenticator
A minimalist, semi-scriptable authenticator app for SXMO, written in the span of a couple days.


## Usage
### CLI
*ncAuth* acceps a few command line arguments to allow for automated usage.

`--help`  
Prints the help page & exits.

`--code [name] [code]`  
Allows you to specify a 2FA secret to use only in this session, where `name` is the name to display & `code` is the secret key.

`--file [file]`  
Specifies a secrets file, which if unspecified reads `secrets.tsv` in the program's install directory.  
The format for each line in the file is `NAME\tSECRET`.  
If `file` is "none", no file will be loaded & no codes will be displayed unless codes are defined as arguments.

`--print`  
Prints all account names & generated tokens, instead of running in interactive mode with ncurses, might be useful for automation.  
The format for each code is `NAME\tCODE`.  

### GUI
In the interactive GUI, you will see a list of your 2FA codes. The selected row has a progressbar of when the tokens will switch.

`k` & `j`  
Select the above or below row. If you have more 2FA codes that can fit on your screen, they will scroll with the selection.

`c`  
Cycles the app colour/theme between your terminal colours. Will be saved to `colour` in the install directory only if the program is quit with `Q`.

`s`  
Runs the script `script.sh` in the install directory, passing the current token's name & displayed token.  
This could be used for example to copy the token to the clipboard, but as clipboards are very WM-dependent, you'll have to write the script yourself.  
If you can't access your clipboard, a workaround could be to call `xdotool` to type the token after a few seconds.

`e`  
Runs the script `edit.sh` in the install directory, passing the current file name.  
Used to edit the current secrets file by opening an editor, because text input is hard with only `getch()`.  
Will block the program until it's closed, and will not do anything if the file is "none".

`n`  
Shows the next 30 second interval's token instead of the current one, with the progressbar inverted. Also passes the next token to `script.sh`.  
Is automatically disabled when the next tokens become current.

`q`  
dies lol


## Building
Dependencies:
- `libncurses`
- Standard C libraries

Then run:  
`gcc src/ncauth.c -lncurses -o ncauth`

### Installing
I have not specified any installation directory; the secrets file and scripts are found in the working directory.  
To "install" it, move the scripts and the executable wherever you want to keep your secrets file (such as your home folder), then make a script to `cd` to & call it somewhere in your `$PATH`.

## Notes
Actual HOTP code written by [this nigga](https://github.com/RealAlphabet/GoogleAuthentificatorC), modified bc idk what a makefile is
