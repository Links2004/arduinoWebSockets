#!/bin/bash

set -ex

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

function build_sketch()
{
    local arduino=$1
    local sketch=$2
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
    local ideversion=$4
    local board=$5
    local sketches=($(find $srcpath -name *.ino))
    for sketch in "${sketches[@]}" ; do
        local sketchdir=$(dirname $sketch)
        local sketchname=$(basename $sketch)
        if [[ -f "$sketchdir/.$platform.skip" ]]; then
            continue
        fi
        echo -en "{\"name\":\"$sketchname\",\"board\":\"$board\",\"ideversion\":\"$ideversion\",\"cpu\":\"$platform\",\"sketch\":\"$sketch\"}"
        if [[ $sketch != ${sketches[-1]} ]] ; then
            echo -en ","
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