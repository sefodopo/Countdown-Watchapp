all: compile
	pebble install --emulator basalt

compile:
	pebble build
