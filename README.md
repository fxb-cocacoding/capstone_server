# capstone_server
A minimal capstone disassembler server implementation

### How to compile it:

Install capstone bindings (and header files) for your operating system.
Make sure you have the libraries `pthreads` and `capstone` available for cmake.

```
git clone https://github.com/fxb-cocacoding/capstone_server.git
cd capstone_server
mkdir build
cd build
cmake ..
make all
```

Then you can run the program by `./daemonize.sh`. You can edit the source code to disable multi-threading or changing the IP (127.0.0.1 default) or PORT (12345 default). Although this is not recommended since these values are currently hardcoded in YARA-Signator, which is probably the reason you want to use this capstone daemon.

The software was built and testet on my developer station with GCC 9.2.0, CMake 3.16.5, capstone 4.0.1 and libpthreads 2.29 (should be part of glibc 2.29). If the software crashes or does not compile feel free to open an issue (even better with an appended stack trace).

### How to use it

If the server is running, you can do for example this:

```
~ $ python3 -c "print(12*'\x90')" | nc 127.0.0.1 12345
0x0     nop
0x1     nop
0x2     nop
0x3     nop
0x4     nop
0x5     nop
0x6     nop
0x7     nop
0x8     nop
0x9     nop
0xa     nop
0xb     nop
```
