#!/bin/sh

if  lsmod | grep morse_encrypter 
	then
		echo "Removing already loaded module"
		rmmod morse_encrypter

fi

echo "Loading morse_encrypter module and starting application"
insmod morse_encrypter.ko $1 $2 && ./KonzApp


