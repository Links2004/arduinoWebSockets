#!/bin/bash
set -e

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

function build_sketch_cli()
{
    local sketch=$1
    local board=$2
    arduino-cli --log --log-level info compile -b "$board" "$sketch"
    result=$?
    if [ $result -ne 0 ]; then
        echo "Build failed ($sketch) build verbose..."
        arduino-cli --log --log-level debug compile -b "$board" "$sketch"
        result=$?
    fi
    if [ $result -ne 0 ]; then
        echo "Build failed ($1) $sketch"
        return $result
    fi
}

function build_sketch()
{
    local arduino=$1
    local sketch=$2
    $arduino --verify --verbose $sketch;
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
}

function get_sketches_json()
{
    local arduino=$1
    local srcpath=$2
    local platform=$3
    local sketches=($(find $srcpath -name *.ino))
    echo -en "["
    for sketch in "${sketches[@]}" ; do
        local sketchdir=$(dirname $sketch)
        if [[ -f "$sketchdir/.$platform.skip" ]]; then
            continue
        fi
        echo -en "\"$sketch\""
        if [[ $sketch != ${sketches[-1]} ]] ; then
            echo -en ","
        fi

    done
    echo -en "]"
}

function get_sketches_json_matrix()
{
    local arduino=$1
    local srcpath=$2
    local platform=$3
    local cliversion=$4
    local board=$5
    local sketches=($(find $srcpath -name *.ino))
    for sketch in "${sketches[@]}" ; do
        local sketchdir=$(dirname $sketch)
        local sketchname=$(basename $sketch)
        if [[ -f "$sketchdir/.$platform.skip" ]]; then
            continue
        fi
        echo -en "{\"name\":\"$sketchname\",\"board\":\"$board\",\"cliversion\":\"$cliversion\",\"cpu\":\"$platform\",\"sketch\":\"$sketch\"}"
        if [[ $sketch != ${sketches[-1]} ]] ; then
            echo -en ","
        fi
    done
}

function get_core_cli() {
    export ARDUINO_BOARD_MANAGER_ADDITIONAL_URLS="https://arduino.esp8266.com/stable/package_esp8266com_index.json https://espressif.github.io/arduino-esp32/package_esp32_index.json https://github.com/earlephilhower/arduino-pico/releases/download/3.9.2/package_rp2040_index.json"
    arduino-cli core update-index
    arduino-cli core install esp8266:esp8266
    arduino-cli core install esp32:esp32
    arduino-cli core install rp2040:rp2040
}

function get_core()
{
    echo Setup core for $1

    mkdir -p $HOME/arduino_ide/packages/hardware
    cd $HOME/arduino_ide/packages/hardware

    if [ "$1" = "esp8266" ] ; then
        mkdir esp8266com
        cd esp8266com
        git clone --depth 1 https://github.com/esp8266/Arduino.git esp8266
        cd esp8266/
        git submodule update --init
        rm -rf .git
        cd tools
        python get.py
    fi

    if [ "$1" = "esp32" ] ; then
        mkdir espressif
        cd espressif
        git clone --depth 1 https://github.com/espressif/arduino-esp32.git esp32
        cd esp32/
        rm -rf .git
        cd tools
        python get.py
    fi

}

function clone_library() {
    local url=$1
    echo clone $(basename $url)
    mkdir -p $HOME/Arduino/libraries
    cd $HOME/Arduino/libraries
    git clone --depth 1 $url
    rm -rf */.git
    rm -rf */.github
    rm -rf */examples
}

function hash_library_names() {
    cd $HOME/Arduino/libraries
    ls | sha1sum -z | cut -c1-5
}