# Firmware Development

I will not be going thought the firmware line by line as that will be boring.  I will leave it up to you to decide you wish to do that.  Instead I will go through the why of each library and what to look out for.  By the time these blogs are published I will have commented each library and give some examples where I think the code is not clear enough.

## Multiple Threads/Tasks

The ESP32 device has 2 cores, this allows for multiple processes/task be performed at once.  The timing is will be issue here and the possible race conditions that could follow, like reading of pins that share the same hardware interrupts. With that being said each task will take Mutex before beginning and use the 