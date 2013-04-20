ictOS
=====

New Concept Operating System


Abstract
========

* This OS is a micro kernel OS. It's IPC is base on dynamic message pool, which is designed as asynchronous communication.

* My DMP(Dynamic Message Pool) is kernel memory major user, and kernel memory is managed by memory daemon, which kernel process can easily use kernel memory by call function.

* So, there is an new concept named "kernel process", KP(Kernel Process) is an special process running in ring1 (kernel running in ring0). Each kernel process have entry that point to an message buffer linklist, each element of this linklist is a message buffer, which located in DMP.

* Almost all kernel process is a daemon, each of them manage a system resource or function. all process use DMP to communicate. and this OS is running in this environment, which each kernel process send and receive message constantly, so that this OS works very complex.

* User Process is an different process. UP(User Process) is managed by KP, so we can consider it as an thread of KP. so there is two schedule level, kernel schedule is schedule KP, and some UP management daemon(is a KP) have schedule function that can schedule UP that managed by this KP.
