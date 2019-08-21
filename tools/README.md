# A Guide to Use Database Tools

## Environment

System Requirements

```
Ubuntu 18.04.2 LTS (GNU/Linux 4.15.0-55-generic x86_64)
```

The database tools are written in C++. 

If not installed, install the following packages before proceeding to installation guide. Cmake and a minimum of GCC 8 are required to compile

```sh
> cmake --version
cmake version 3.14.5
```

```sh
sudo apt-get install cmake gcc-8 g++-8
```

## Installation
This section assumes you have properly setup the environment in the last step.

### External Library
#### cpptqdm
Inside `PatentTagging` run
```sh
git submodule update --init --recursive
```
this will clone the `cpptqdm` submodule

#### pugixml
Xml parsing library

To install run 
```sh
sudo apt-get install libpugixml-dev
```
This will install and automatically add pugixml library to the default C++ include path

#### Compilation
Inside the `PatentTagging/tools` directory, run 
```sh
cmake -DCMAKE_BUILD_TYPE=Release .
```
```sh
make
```
or
```sh
cmake --build . --target DatabaseGenerator -- -j4
cmake --build . --target DatabaseQuerySelectorExe -- -j4
```
This will compile and drop executables inside `tools/bin` directory.

## Usage & Run Guide

### DatabaseGenerator

#### Usage

```
DatabaseGenerator <in-path-file> <out-data-dir> <out-index-file> <n-readers> <n-workers>

Options:
    <in-path-file>      file of patent xml-file path
    <out-data-dir>      output binary file folder
    <out-index-file>    output index file
    <n-readers>         n threads
    <n-workers>         n threads
```

#### Sample

> DatabaseGenerator 2014-path-test.txt test/ test.idx 4 4

```
  xmls parsed:████████████████████████████████████████▏ 100.0% [10000/10000 | 2.4 kHz | 4s<0s]

  data files written:████████████████████████████████████████▏ 100.0% [   4/   4 | 1.4 kHz | 0s<0s]

  data files written:████████████████████████████████████████▏ 100.0% [   4/   4 | 1.4 kHz | 0s<0s]

```

```
@test.idx

Publication ID	Application ID	Application Date	Classification IPC	Bin ID	Offset	Title Index	Abstract Index	Claim Index	Description Index
US20140244363A1	US13781044	20130228	G06Q-10/06	0	12	4	77	544	5382
US20140257512A1	US14198769	20140306	A61F-2/36	0	41462	4	45	997	4331
US20140251245A1	US14203116	20140310	F01L-1/344	0	68364	4	38	983	5444
US20140006094A1	US13539995	20120702	G06Q-10/06	0	130196	4	75	973	9792
US20140264570A1	US14288167	20140527	H01L-29/66,H01L-29/78	0	181132	4	60	966	5066
US20140169200A1	US14109747	20131217	H04J-11/00	0	218312	4	42	546	5304
US20140295342A1	US14227564	20140327	G03G-9/113	0	265962	4	127	799	2708
US20140309136A1	US14284720	20140522	C12Q-1/68	0	307025	4	110	728	21129
US20140235606A1	US14262863	20140428	C07D-403/12,A61K-45/06,A61K-31/506	0	460188	4	109	429	4983
```

### DatabaseQuerySelectorExe

#### Usage

```
DatabaseQuerySelectorExe <index-file> <data-dir> <request-id-file>

Options:
    <index-file>        .tsv
    <data-dir>          binary files folder
    <request-id-file>   patent publicid list(US2014369***A1...)


```

#### Sample

> DatabaseQuerySelectorExe test/index.tsv test/data/ query_test.txt > output.txt 2> error.txt  

```
@output.txt

read 82309793 bytes
ID: US20140369904A1
title: PROCESSES FOR PREPARING ALUMINA AND VARIOUS OTHER PRODUCTS
abstract: There are provided processes for preparing alumina. These processes can comprise leaching an aluminum-containing material with HCl so as to obtain a leachate comprising aluminum ions and a solid, and separating said solid from said leachate; reacting said leachate with HCl so as to obtain a liquid and a precipitate comprising said aluminum ions in the form of AlCl 3 , and separating said precipitate from said liquid; and heating said precipitate under conditions effective for converting AlCl 3  into Al 2 O 3  and optionally recovering gaseous HCl so-produced. These processes can also be used for preparing various other products such as hematite, MgO, silica and oxides of various metals, sulphates and chlorides of various metals, as well as rare earth elements, rare metals and aluminum. 
claim: 1 . A process for preparing alumina, said process comprising:

```

```
@error.txt

getInfoById: ID [US20140332418A1] does not exist in database
```