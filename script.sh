#!/bin/sh

# $1: Account name
# $2: Auth code

# Write to clipboard - won't work on non-Xorg desktops
echo -n $2 | xclip

# Show notification
notify-send "$1" "`echo $2 | head -c 3` `echo $2 | tail -c 4`"
