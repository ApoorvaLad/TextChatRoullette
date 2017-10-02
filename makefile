#Use gcc as a standard compiler
COMPILE=gcc

#Turn on flags: -lpthread and -w
THREAD=-w -lpthread

all:
#Compile server
	@echo "--------------Server Compilation Started--------------"
	$(COMPILE) server.c -o server $(THREAD)
	@echo "--------------Server Compilation ENDED--------------"

#Compile client
	@echo "--------------Client Compilation Started--------------"
	$(COMPILE) client.c -o client $(THREAD)
	@echo "--------------Client Compilation Started--------------"
	@echo "You can start running client by entering ./client and server by entering ./server"	
