# OpenAL EAX POC

Simple Audio manipulation using OpenAL

### NOTE EAX NOT YET WORKING ON MAC :(

### Required Tools
* C/C++
* OpenAL the 3d Audio library

### How to Build
Go to the root directory where main.cpp file is located and run:

**On MAC** (`must have Xcode and the OpenAL.framework installed`)
```sh
$ g++ -framework OpenAL main.cpp -o oal_eax_poc
```
**On GNU/Linux** (`must have c++/g++ and  OpenAL installed`)
```sh
$ g++ -lAL main.cpp -o oal_eax_poc 
```
