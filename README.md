# capstone_server
A minimal capstone disassembler server implementation

### How to compile it:
Install capstone bindings (and header files) for your operating system.
Make sure you have the libraries pthrads and capstone available for cmake.
```
git clone https://github.com/fxb-cocacoding/capstone_server.git
cd capstone_server
mkdir build
cd build
cmake ..
make all
```
Then you can run the program by `./capstone_server`. You can edit the source code to disable multi-threading or changing the IP (127.0.0.1 default) or PORT (12345 default).

### How to use it;

If the server is running, you can do for example this:

```
~ $ python2 -c "print 12*'\x90'" | nc 127.0.0.1 12345
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
