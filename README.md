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
