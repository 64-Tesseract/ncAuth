#!/bin/sh

# $1: Account name
# $2: Auth code

# Sample code, workaround for no clipboard libs working on PinePhone
# 5 seconds should be enough to find the right window, might be too long tbh
(sleep 5 && xdotool type "$2")&

# Show notification
notify-send "$1" "`echo $2 | head -c 3` `echo $2 | tail -c 4`"