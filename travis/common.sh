#!/bin/bash

function build_sketches()
{
    local arduino=$1
    local srcpath=$2
    local platform=$3
    local sketches=$(find $srcpath -name *.ino)
    for sketch in $sketches; do
        local sketchdir=$(dirname $sketch)
        if [[ -f "$sketchdir/.$platform.skip" ]]; then
            echo -e "\n\n ------------ Skipping $sketch ------------ \n\n";
            continue
        fi
        echo -e "\n\n ------------ Building $sketch ------------ \n\n";
        $arduino --verify $sketch;
        local result=$?
        if [ $result -ne 0 ]; then
            echo "Build failed ($sketch) build verbose..."
            $arduino --verify --verbose --preserve-temp-files $sketch
            result=$?
        fi
        if [ $result -ne 0 ]; then
            echo "Build failed ($1) $sketch"
            return $result
        fi
    done
}


function get_core()
{
    echo Setup core for $1

    cd $HOME/arduino_ide/hardware

    if [ "$1" = "esp8266" ] ; then
        mkdir esp8266com
        cd esp8266com
        git clone https://github.com/esp8266/Arduino.git esp8266
        cd esp8266/tools
        python get.py
    fi

    if [ "$1" = "esp32" ] ; then
        mkdir espressif
        cd espressif
        git clone https://github.com/espressif/arduino-esp32.git esp32
        cd esp32/tools
        python get.py
    fi

}
