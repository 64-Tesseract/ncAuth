#!/bin/sh

# $1: File name

# Opens the current secrets file in an editor
case "$TERM" in
    *foot*)
        # Hide debug text
        sxmo_terminal.sh -d error $EDITOR "$1"
        ;;
    *st*)
        sxmo_terminal.sh $EDITOR "$1"
        ;;
    *)
        # Just run in current window if terminal not known
        nvim "$1"
        ;;
esac
