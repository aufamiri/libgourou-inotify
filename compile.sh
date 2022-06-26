#!/bin/bash

g++ -Wall -fPIC -I/home/aufanabilamiri/Projects/libgourou-inotify/libgourou/include -I/home/aufanabilamiri/Projects/libgourou-inotify/libgourou/lib/pugixml/src/ -O2 adept_inotify.cpp utils.a  -L/home/aufanabilamiri/Projects/libgourou-inotify/libgourou -lcrypto -lzip -lz -lcurl -lgourou -o adept_inotify

