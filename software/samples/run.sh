if [ $# -eq 0 ]
    then
        echo "Usage: run.sh binaryname"
else
    ../build/release/riscvtool $1 -run /dev/ttyUSB0
fi
