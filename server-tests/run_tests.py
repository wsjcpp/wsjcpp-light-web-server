#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from pprint import pprint
import liblightwebservertest
import requests

TEST_URL = "http://localhost:1234"

try:
    print(" > > > TESTS: begin ")
    liblightwebservertest.start_server("../", ["./wsjcpp-light-web-server", "start"], 1234)

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
            print("index.css - ok")

    

finally:
    liblightwebservertest.stop_server()

