compile:
	gcc -o server server.c
	gcc -o client client.c
	#************  for test ************
	# ipAddress: 127.0.0.1
	# port: 4444
	# client shell command will work if you write only uppercase letters
runServer:
	./server
runClient:
	./client
