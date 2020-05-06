#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from pprint import pprint
import liblightwebservertest
import requests
import socket
import sys
import time

SERVER_HOST = "localhost"
SERVER_PORT = 1234
TEST_URL = "http://" + SERVER_HOST + ":" + str(1234)

try:
    print(" > > > TESTS: begin ")
    liblightwebservertest.start_server("../", ["./wsjcpp-light-web-server", "folder", "./web"], 1234)
    print("index.html - testing...")
    index_html = open("../web/index.html", 'r').read()
    r = requests.get(TEST_URL)
    if r.status_code != 200:
        raise Exception("index.html - Wrong status code expected 200, but got " + str(r.status_code))
    else:
        if r.text != index_html:
            raise Exception("index.html - expected content like in file ../web/index.html")
        else:
            print("index.html - ok")

    print("index.css - testing...")
    index_css = open("../web/css/index.css", 'r').read()
    r = requests.get(TEST_URL + "/css/index.css?some=1&ddd=1")
    if r.status_code != 200:
        raise Exception("index.css - Wrong status code expected 200, but got " + str(r.status_code))
    else:
        if r.text != index_css:
            raise Exception("index.css - expected content like in file ../web/css/index.css")
        else:
            print("index.css - ok")

    print("app.js - testing...")
    app_js = open("../web/js/app.js", 'r').read()
    r = requests.get(TEST_URL + "/js/app.js?v1=20200101")
    if r.status_code != 200:
        raise Exception("app.js - Wrong status code expected 200, but got " + str(r.status_code))
    else:
        if r.text != app_js:
            raise Exception("app.js - expected content like in file ../web/js/index.js")
        else:
            print("app.js - ok")

    print("not-found - testing...")
    r = requests.get(TEST_URL + "/not-found 0101")
    if r.status_code != 404:
        raise Exception("not-found - Wrong status code expected 404, but got " + str(r.status_code))
    print("not-found 0101 - ok")

    print("\n\n >>>>>>>>>>>>>>>>>>\n")
    print(">>>> TEST keep-alive \n")
    test_keep_alive_request = '''GET /js/app.js?v1=20200101 HTTP/1.1
Host: '''  + SERVER_HOST + ''':''' + str(SERVER_PORT) + '''
User-Agent: tcp
Accept-Encoding: gzip, deflate
Accept: */*
Connection: keep-alive

'''
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.settimeout(0.3)
    sock.connect((SERVER_HOST, SERVER_PORT))
    print(">>>> send first")
    sock.send(test_keep_alive_request.encode("utf8"))
    r1 = sock.recv(1024)
    r1 = r1.decode("utf8")
    # print(r1)
    time.sleep(1)
    print(">>>> send again by same connection")
    sock.send(test_keep_alive_request.encode("utf8"))
    r2 = sock.recv(1024)
    r2 = r2.decode("utf8")
    # print(r2)
    if len(r1) != len(r2):
        raise Exception("test keep-alive - expected same length. Expected " + str(len(r1)) + " bytes, but got " + str(len(r2)) + " bytes")
except socket.timeout:
    print("FAILED by timeout")
except Exception as e:
    print("FAILED: ", e)
except:
    print("Unexpected error:", sys.exc_info()[0])
finally:
    liblightwebservertest.stop_server()

