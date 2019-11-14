#!/usr/bin/env bash

apt-get update
apt-get upgrade -y
apt-get install -y gcc
apt-get install -y gdb
apt-get install -y git
apt-get install -y make
apt-get install -y python
apt-get install -y valgrind

echo "cd /vagrant" >> /home/vagrant/.bashrc
