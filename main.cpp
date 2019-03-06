#include <stdio.h>  
#include <string.h>
#include <stdlib.h>  
#include <errno.h>  
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>  
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <sys/time.h> 
#include <inttypes.h>
#include <signal.h>
#include <capstone/capstone.h>
#include <pthread.h>

/*
 * capstone server v0.1
 * 
 * PURPOSE: Provide a capstone binding for yara-signator over TCP
 * (https://github.com/fxb-cocacoding/yara-signator)
 * 
 * DEPENDENCIES: pthrads, capstone
 * 
 * currently works for 32 bit disassembly, but you can change
 * CS_MODE_32 to CS_MODE_64 and get amd64 assembly output.
 * 
 * TCP select loop was adopted from here:
 * http://man7.org/linux/man-pages/man2/select_tut.2.html
 * 
 * capstone stuff is from the capstone C tutorial:
 * https://www.capstone-engine.org/lang_c.html
 * 
 * Compile it (cmake and ninja) and then you can do the following:
 * echo -ne "\x90\x90\x90\x90\x90\x90\x90\x90" | nc 127.0.0.1 12345
 * 
 * This should give you the following output:
 * 
 * 0x0     nop
 * 0x1     nop
 * 0x2     nop
 * 0x3     nop
 * 0x4     nop
 * 0x5     nop
 * 0x6     nop
 * 0x7     nop
 * 
 * Do not use this program for anything else than fun.
 * It crashes sometimes, if you find out why, please let me know.
 * 
 * It was developed very quick as a workaround, because the capstone JAVA bindings
 * were creating a large memory leak which crashed my process. TCP because you can
 * easy restart the server and TCP handles the lost packets for you.
 * 
 */

#define PORT 12345
#define IP "127.0.0.1"

struct args {
    int i;
    int sd;
    uint8_t *buffer;
    int buffer_size;
    int read_return;
} args;

void *capstone_worker(void* pthread_args) {
    
    struct args *arguments = (struct args *)pthread_args;
    const char *failure = "INVALID\n";
    csh handle;
    cs_insn *insn;
    size_t count;
    
    if (cs_open(CS_ARCH_X86, CS_MODE_32, &handle) != CS_ERR_OK) {
        return NULL;
    }
    
    count = cs_disasm(handle, arguments->buffer, arguments->read_return, 0x0000, 0, &insn);
    
    if (count > 0) {
        
        size_t j;
        
        for(j=0; j<count; j++) {
            //This is ugly but it works.
            int snprintf_return = snprintf((char*)arguments->buffer, 1024, "0x%"PRIx64"\t%s\t%s\n", insn[j].address, insn[j].mnemonic, insn[j].op_str);
            int write_return = write(arguments->sd, arguments->buffer, snprintf_return);
        }
    } else {
        //printf("ERROR: Failed to disassemble given code!\n");
        printf("ERROR in cs_disasm: %i\n", cs_errno(handle));
        write(arguments->sd, failure, strlen(failure));
    }
    cs_free(insn, count);
    
    // write_return = write(sd, buffer, read_return);
    
    cs_close(&handle);
}

void* handle_multithreading(void* arguments_supplied) {
    
    //printf("multithreading_handler\n");
    
    struct args *arguments = (struct args *) arguments_supplied;
    uint8_t *buffer = arguments->buffer;
    int client_socket = arguments->sd;
    int buffer_size = arguments->buffer_size;
    
    for(;;) {
        int read_return = read(client_socket ,buffer, buffer_size);
        
        if (read_return == 0) {
            //Host disconnected
            close(client_socket);
            break;
        } else {
            arguments->read_return = read_return;
            arguments->i = 0;
            capstone_worker((void*)arguments);
        }
    }
    
    free(((struct args *)arguments_supplied)->buffer);
    free(arguments_supplied);
    pthread_exit(NULL);
}

int multithreading(int listener_socket, int max_sd, const int max_clients, struct sockaddr_in *address, int *addrlen, const int buffer_size) {
    
    /*
     * Detached thread for every new client
     */
    
    pthread_t p_thread_container;
    pthread_attr_t pthread_attr;
    
    if (pthread_attr_init(&pthread_attr) != 0) {
        exit(EXIT_FAILURE);
    }
    
    if (pthread_attr_setdetachstate(&pthread_attr, PTHREAD_CREATE_DETACHED) != 0) {
        exit(EXIT_FAILURE);
    }
    
    for(;;) {
        int new_socket = accept(listener_socket, (struct sockaddr *)address, (socklen_t*)addrlen);
        if (new_socket < 0) {
            exit(EXIT_FAILURE);
        }
        
        //ugly, but I have a C++ compiler
        //printf("malloc ahead\n");
        struct args *arguments = (struct args *) malloc(sizeof(struct args));
        uint8_t *buffer = (uint8_t *) malloc(buffer_size* sizeof(char));
        
        arguments->buffer = buffer;
        arguments->sd = new_socket;
        arguments->buffer_size = buffer_size;
        
        if (pthread_create(&p_thread_container, &pthread_attr, handle_multithreading, (void *)arguments) != 0) {
            free(arguments);
            free(buffer);
            exit(EXIT_FAILURE);
        }
        
    }
    return 0;
}

int select_loop(int listener_socket, int max_sd, const int max_clients, struct sockaddr_in *address, int *addrlen, const int buffer_size) {
        
    struct args *arguments[max_clients];
    uint8_t buffer[max_clients][buffer_size];
    
    
    for(int i=0; i<max_clients; i++) {
        arguments[i] = (struct args*) malloc(sizeof(struct args));
        memset(buffer[i], 0, buffer_size);
    }
        
    int client_socket[max_clients+1];
    
    for(int i=0;i< max_clients+1; i++) {
        client_socket[i] = 0;
    }
    fd_set readfds;
    
    
    for(;;) {
        FD_ZERO(&readfds);
        FD_SET(listener_socket, &readfds);
        max_sd = listener_socket;
        
        for (int i=0; i<max_clients; i++) {
            int sd = client_socket[i];
            
            if(sd > 0) {
                FD_SET(sd, &readfds);
            }
            
            if(sd > max_sd) {
                max_sd = sd;
            }
        }
        
        int return_code = select( max_sd + 1 , &readfds , NULL , NULL , NULL);
        
        if ((return_code < 0)) {
            printf("select error\n");
            return EXIT_FAILURE;
        }
        
        if (FD_ISSET(listener_socket, &readfds)) {
            int new_socket = accept(listener_socket, (struct sockaddr *)address, (socklen_t*)addrlen);
            if (new_socket < 0) {
                printf("new socket error\n");
                exit(EXIT_FAILURE);
            }
            
            printf("new client - remote info: socket %d, ip : %s, port : %d\n" , new_socket , inet_ntoa(address->sin_addr) , ntohs(address->sin_port));
            
            //write(new_socket, message, strlen(message));
            
            for (int i=0; i<max_clients; i++) {
                if( client_socket[i] == 0 ) {
                    //the next empty socket is zero, so " client_socket[i] == 0 " check
                    client_socket[i] = new_socket;
                    printf("adding new client at fd %d\n" , i);
                    break;
                }
            }
        }
        
        for (int i=0; i<max_clients; i++) {
            int sd = client_socket[i];
            
            if (FD_ISSET( sd , &readfds)) {
                int read_return = read(sd ,buffer[i], buffer_size);
                
                if (read_return == 0) {
                    
                    //Host disconnected
                    
                    close(sd);
                    client_socket[i] = 0;
                    
                } else {
                    arguments[i]->buffer = buffer[i];
                    arguments[i]->buffer_size = buffer_size;
                    arguments[i]->i = i;
                    arguments[i]->sd = sd;
                    arguments[i]->read_return = read_return;
                    capstone_worker((void*)arguments[i]);
                    //printf("gone\n");
                }
            }
        }
    }
    return 0;
}

int main(int argc , char *argv[]) {
    const char *message = "CNPTAESO RRVSEE v0.1\n\n";
    const char *failure = "INVALID\n";
    const int port = PORT;
    const int ip_addr = inet_addr(IP);
    const int max_waiting_connections = 8;
    const int buffer_size = 8192;
    const int max_clients = 16;
    
    int listener_socket;
    int addrlen;
    int max_sd;
    struct sockaddr_in address;
    int flag = 1;
    int return_code = 0;
    
    
    listener_socket = socket(AF_INET , SOCK_STREAM , 0);
    
    if( listener_socket == 0) {
        exit(EXIT_FAILURE);
    }
    
    return_code = setsockopt(listener_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&flag, sizeof(flag));
    
    if(return_code  < 0 ) {
        exit(EXIT_FAILURE);
    }
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = ip_addr;
    address.sin_port = htons(port);
    
    return_code = bind(listener_socket, (struct sockaddr *)&address, sizeof(address));
    
    if (return_code < 0) {
        printf("bind error, is the server already online?\n");
        exit(EXIT_FAILURE);
    }
    printf("Listening on port: %i on address: %s\n", port, IP);
    printf("...\n");
    return_code = listen(listener_socket, max_waiting_connections);
    
    if (return_code < 0) {
        exit(EXIT_FAILURE);
    }
    
    /*
     * Handle SIGPIPEs in write/read errors.
     * We ignore them and continue
     */
    signal(SIGPIPE, SIG_IGN);
    addrlen = sizeof(address);
    
    
    /*
     * Uncomment select_loop and comment multithreading if you want to use a single-threaded solutions without posix threads.
     * 
     */
    
    //select_loop(listener_socket, max_sd, max_clients, &address, &addrlen, buffer_size);
    multithreading(listener_socket, max_sd, max_clients, &address, &addrlen, buffer_size);
    
    return 0;   
}

