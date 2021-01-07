#!/usr/bin/python3

import json
import configparser
import argparse
import re
import os
import datetime

travis_dir = os.path.dirname(os.path.abspath(__file__))
base_dir = os.path.abspath(travis_dir + "/../")

def write_header_file(version):
    hvs = version.split('.')
    intversion = int(hvs[0]) * 1000000 + int(hvs[1]) * 1000 + int(hvs[2])
    now = datetime.datetime.now()

    text = f'''/**
 * @file WebSocketsVersion.h
 * @date {now.strftime("%d.%m.%Y")}
 * @author Markus Sattler
 *
 * Copyright (c) 2015 Markus Sattler. All rights reserved.
 * This file is part of the WebSockets for Arduino.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifndef WEBSOCKETSVERSION_H_
#define WEBSOCKETSVERSION_H_

#define WEBSOCKETS_VERSION "{version}"

#define WEBSOCKETS_VERSION_MAJOR {hvs[0]}
#define WEBSOCKETS_VERSION_MINOR {hvs[1]}
#define WEBSOCKETS_VERSION_PATCH {hvs[2]}

#define WEBSOCKETS_VERSION_INT {intversion}

#endif /* WEBSOCKETSVERSION_H_ */
'''
    with open(f'{base_dir}/src/WebSocketsVersion.h', 'w') as f:
        f.write(text)


def get_library_properties_version():
    library_properties = {}
    with open(f'{base_dir}/library.properties', 'r') as f:
        library_properties = configparser.ConfigParser()
        library_properties.read_string('[root]\n' + f.read())
    return library_properties['root']['version']


def get_library_json_version():
    library_json = {}
    with open(f'{base_dir}/library.json', 'r') as f:
        library_json = json.load(f)
    return library_json['version']


def get_header_versions():
    data = {}
    define = re.compile('^#define WEBSOCKETS_VERSION_?(.*) "?([0-9\.]*)"?$')
    with open(f'{base_dir}/src/WebSocketsVersion.h', 'r') as f:
        for line in f:
            m = define.match(line)
            if m:
                name = m[1]
                if name == "":
                    name = "VERSION"
                data[name] = m[2]
    return data


parser = argparse.ArgumentParser(description='Checks and update Version files')
parser.add_argument(
    '--update', action='store_true', default=False)
parser.add_argument(
    '--check', action='store_true', default=True)

args = parser.parse_args()

if args.update:
    library_properties_version = get_library_properties_version()

    with open(f'{base_dir}/library.json', 'r') as f:
        library_json = json.load(f)

    library_json['version'] = library_properties_version

    with open(f'{base_dir}/library.json', 'w') as f:
        json.dump(library_json, f, indent=4, sort_keys=True)

    write_header_file(library_properties_version)


library_json_version = get_library_json_version()
library_properties_version = get_library_properties_version()
header_version = get_header_versions()

print("WebSocketsVersion.h", header_version)
print(f"library.json: {library_json_version}")
print(f"library.properties: {library_properties_version}")

if args.check:
    if library_json_version != library_properties_version or header_version['VERSION'] != library_properties_version:
        raise Exception('versions did not match!')

    hvs = header_version['VERSION'].split('.')
    if header_version['MAJOR'] != hvs[0]:
        raise Exception('header MAJOR version wrong!')
    if header_version['MINOR'] != hvs[1]:
        raise Exception('header MINOR version wrong!')
    if header_version['PATCH'] != hvs[2]:
        raise Exception('header PATCH version wrong!')

    intversion = int(hvs[0]) * 1000000 + int(hvs[1]) * 1000 + int(hvs[2])
    if int(header_version['INT']) != intversion:
        raise Exception('header INT version wrong!')
