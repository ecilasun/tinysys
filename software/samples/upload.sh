if [ $# -eq 0 ]
    then
        echo "Usage: upload.sh binaryfilename"
else
    ../build/release/riscvtool $1 -sendfile /dev/ttyUSB0
fi
