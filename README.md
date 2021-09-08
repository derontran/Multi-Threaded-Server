# to run please include template.txt for server to usę and data.txt needed for client 
# example run :
# make 
# ./server & 
# ./client -c 1 < data.txt & or sh run_clients.sh


# Discussion on number of thread: 
# after experiment with different number of thread. I come to the conlusion that any thread number that can be use but 1-3  
# threads gave fairly better result. Although we make concurrency on message procesing to the client, the server still have to 
# synchronizedly receice the requests and sanitize the message received, which slow down the work to be passed to the number of 
# worker. So if we have such a high number of threads that just mean the job will still be done but some of the worker will have
# to wait more hoping there is some work for them and if there is a work signal and they wake up finding out some other thread 
# had taken that work from them then now they keep waiting or in some case some of them never get any work to do. This kind of 
# wake up/waiting race cost us the time switching thread to thread, and checking "is there any work". 
