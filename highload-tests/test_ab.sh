#!/bin/bash

# ln -s `pwd`/../web /var/www/html/wsjcpp-light-web-server

echo "Apache"
ab -n 10000 -c 10 http://localhost/wsjcpp-light-web-server/ > localhost_apache.log
echo "wsjcpp"
ab -n 10000 -c 10 http://localhost:1234/ > localhost_wsjcpp.log