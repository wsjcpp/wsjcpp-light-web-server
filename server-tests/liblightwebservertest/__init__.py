import sys
import os
import subprocess
import time
import signal
import socket

test_name = 'Start wsjcpp-light-web-server'
p_server = None

def check_port(host, port):
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    result = sock.connect_ex((host,port))
    return result == 0

def start_server(work_directory, program_args, port):
    print("Start wsjcpp-light-web-server")
    global p_server
    wd = os.getcwd()
    print(wd)
    os.chdir(wd + "/" + work_directory)
    p_server = subprocess.Popen(program_args)
    os.chdir(wd)

    wait_max = 20
    wait_i = 0
    result_check_port = False
    while wait_i < wait_max:
        wait_i = wait_i + 1
        time.sleep(1)
        result_check_port = check_port('127.0.0.1', port)
        if not result_check_port:
            print(" =====> " + str(wait_i) + ": port not available... ")
        else:
            break

    if not result_check_port:
        print("Port not available... failed")
        exit(-1)
    else:
        print("Port available... OK!")

def stop_server():
    print("Stop fhq-server")
    global p_server
    if p_server != None:
        print("Kill process " + str(p_server.pid))
        os.kill(p_server.pid, signal.SIGKILL)