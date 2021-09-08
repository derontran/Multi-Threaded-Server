# Multi-Threaded Server 
+ Server and clients communicate to each other using Inter-process communication(IPC). Each time the server got a request, it will go through a sanitize process which is a state machine that process each byte of the request input ensuring that we don't have any buffer problems. Then the request input will be passed to the server's thread pool to do job on them and respond the output to the client acordingly. Note that the server has a limited number of worker thread and they are meant to be independent to the process that taking the client request that why we made sure to have a proper lock machanism so  that the worker-threads will never block the server from taking the client request.  


### to run please include template.txt for server to usę and data.txt needed for client 
example run :
1. make 
2. ./server & 
3. ./client -c 1 < data.txt & or sh run_clients.sh


### Discussion on number of thread: 
+ after experiment with different number of thread. I come to the conlusion that any thread number that can be use but 1-3 threads gave fairly better result. Although we make concurrency on message procesing to the client, the server still have to synchronizedly receice the requests and sanitize the message received, which slow down the work to be passed to the number of worker. So if we have such a high number of threads that just mean the job will still be done but some of the worker will haveto wait more hoping there is some work for them and if there is a work signal and they wake up finding out some other thread had taken that work from them then now they keep waiting or in some case some of them never get any work to do. This kind of wake up/waiting race cost us the time switching thread to thread, and checking "is there any work". 
