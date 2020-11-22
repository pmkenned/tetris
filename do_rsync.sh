#!/bin/bash

if [[ $# -lt 1 ]]; then
    echo "specify destination (pixel, x200, mingw, mac, copy [dir])"
    exit 2
fi

#TODO: unify the paths
#PATH=~/github/tetris/
DEST_NAME=$1
DEST=""
SCP=0

if [ "$DEST_NAME" = "pixel" ]; then
    echo "rsync to pixel"
    DEST=(u0_a362@192.168.1.67:~/github/tetris/)
    E_ARG=(-e 'ssh -p 8022')
elif [ "$DEST_NAME" = "x200" ]; then
    echo "rsync to x200"
    DEST=(paul@192.168.1.225:~/github/tetris/)
elif [ "$DEST_NAME" = "cygwin" ]; then
    echo "scp to cygwin"
    # NOTE: destination dir must exist
    DEST=(Paul@192.168.1.251:C:/cygwin64/home/Paul/github/tetris/)
    SCP=1
    SCP_FILES=(*.c Makefile do_rsync.sh)
elif [ "$DEST_NAME" = "mac" ]; then
    echo "rsync to mac"
    DEST=(caitlinperrone@192.168.1.28:~/github/tetris/)
elif [ "$DEST_NAME" = "mingw" ]; then
    echo "rsync to copy to mingw"
    DEST=(~/mingw/github/tetris/)
elif [ "$DEST_NAME" = "copy" ]; then
    echo "rsync to copy"
    if [[ $# -lt 2 ]]; then
        echo "specify destination directory"
        exit 2
    fi
    DEST=($2)
else
    echo "unknown destination"
    exit 3
fi

# TODO: consider putting this in a file and using --exclude-from=FILE
EXCLUDES=()
EXCLUDES+=(--exclude ".*")
EXCLUDES+=(--exclude build)
EXCLUDES+=(--exclude tetris --exclude *.exe)

if [ -z "$DEST" ]; then
    echo "DEST is blank";
else
    if [ "$SCP" -eq "0" ]; then
        rsync -avzh "${E_ARG[@]}" --delete --progress ./ "${EXCLUDES[@]}" "${DEST[@]}"
    else
        scp "${SCP_FILES[@]}" "${DEST[@]}"
    fi
fi
