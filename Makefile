fuser:
	gcc -Wall file_system_driver.c -D_FILE_OFFSET_BITS=64 -I/usr/include/fuse  -pthread -lfuse -lrt -ldl -o file_system
clean:
	rm -f file_system
