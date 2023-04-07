#!/bin/bash
#sudo su - postgres & psql & CREATE DATABASE MATCH_ENGINE
make clean
make all
echo 'start running server...'
./main
while true ; do continue ; done
