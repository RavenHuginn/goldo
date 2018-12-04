# Goldo

Goldo is a novel coin based on the modified Cryptonote PoW algorithm called SoftCrypton.

Goldo was designed as a light alternative to current Cryptonote-based coins.
It should be mineable even with low-end PCs, including phones and tablets.
Blockchain size and network load plan to stay at reasonable level as well.
Transfer's tiny flat fee is another key point of Goldo for everyday use.

## Windows Users

### Download pre-compiled Goldo apps

Latest Goldo releases could be found at https://github.com/monselice/goldo/releases

Download ZIP-archive and unpack it to a folder (c:\Bin\Bit)

### Archive content

this archive contains several apps and batch files.

sldd.exe - coin's daemon

sldw.exe - wallet app

sldm.exe - CPU solo miner app

sldp.exe - payments app for exchanges

new-wallet.bat - batch file to generate new wallet

daemon.bat - batch file to start coin's daemon

wallet.bat - batch file to start wallet app with your wallet file and password

miner.bat - batch file to run mining


### Generating a new wallet

Edit new-wallet.bat file, change the name and location of your wallet file as well as password.

Save.

Don't start daemon, but run new-wallet.bat instead.

You will get new wallet file with password supplied as result.

WARINING - backup *.wallet file ASAP to another pc, remote location of flash drive.

### Edit a miner batch file

Open *.wallet.address file in notepad and copy your SLD address to clipboard [Ctrl+C]

Edit miner.bat file, paste just copied address instead of default.

Change number of mining threads if you need.

Save.

### Prepare a wallet batch file

Edit wallet.bat, change the name and location of your wallet file as well as password.

The easiest way is simple copy this params from new-wallet.bat file.

Save.

### Start the daemon

Run daemon.bat.

Wait to full sync.

If daemon will not find any seed - check your windows firewall and home/office router NAT and firewall rules.

If still failed - ask for assistance at official Ryver forum - https://zzl.ryver.com/application/signup/members/h07SVMfFAvFRGiZ

### Start mining

As soon as your daemon found any seed and became fully synced you can start mining.

If you ready to solo mining you have 3 ways to initiate it [but in any case you must have your daemon started and full synced]:

#### By daemon
Type command __start_mining__ in the daemon app:
 - mining will start inside daemon app;
 - CPU recources will be used at computer where is the daemon located;
 - demon app must stay worked.

```
start_mining your_sld_wallet_address threads_count
```

#### By wallet
Type command __start_mining__ in the WALLET app:
 - mining will start inside daemon app;
 - CPU recources will be used at computer where is the daemon located;
 - demon app must stay worked, wallet app can be closed.

```
start_mining threads_count
```

#### By miner
Use miner app:
 - CPU recources will be used at computer where is the miner located;

Start miner.bat.

### Building Goldo

You have to install follow software:

#### MSVC 2015 Community Edition 

http://download.microsoft.com/download/0/B/C/0BC321A4-013F-479C-84E6-4A2F90B11269/vs2015.com_enu.iso

!!! IMPORTANT !!!

You will need to uninstall vs2015 update3 then install vs2015 rtm!

Make sure to uncheck box "Visual Studio 2015 Update 3" else update 3 will be downloaded automatically!!!

#### CMake 2.8.6 or later

https://cmake.org/files/v3.10/cmake-3.10.0-rc5-win64-x64.msi

#### Boost 1.56 or later

https://dl.bintray.com/boostorg/release/1.65.1/source/boost_1_65_1.7z
or
https://dl.bintray.com/boostorg/release/1.65.1/source/boost_1_65_1.zip

Download source code from:

https://github.com/monselice/sld
or clone from Github
git clone https://github.com/monselice/sld.git sld

To build Goldo apps, change to a directory where this file is located (x:\git\), and run commands:

```
x:
cd x:\git\sld\
mkdir build
cd build

cmake -G "Visual Studio 14 2015 Win64" -DBOOST_ROOT=X:/boost_1_65_1 -DBOOST_LIBRARYDIR=X:/boost_1_65_1/lib64-msvc-14.0 ..
MSBuild Goldo.sln /p:Configuration=release /m
```
The resulting executables can be found in X:\Git\sld\build\src\Release

### Maintenance

Sometimes daemon get stuck and you need to perform some actions.

#### Clear blockchain and other data

To remove Goldo data, press [Win+R] key, type %APPDATA%, and delete sld folder if you have one.


## Linux Users

### Building Goldo

Dependencies: GCC 4.7.3 or later, CMake 2.8.6 or later, and Boost 1.56 or later.

You may download them from:

- http://gcc.gnu.org/

- http://www.cmake.org/

- http://www.boost.org/

Alternatively, it may be possible to install them using a package manager.

To build run these commands:

```
sudo apt install git
sudo apt install cmake
sudo apt install build-essential
sudo apt install libboost-dev libboost-all-dev 

cd ~
git clone https://github.com/monselice/sld.git sld
cd sld
mkdir build
cd build
cmake ..
cd ..
make
```

The resulting executables can be found in `sld/build/release/src`.

