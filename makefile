all: compile install

compile:
	pebble build

install:
	pebble install --emulator basalt

debug: install dubuger

debuger:
	pebble gdb
