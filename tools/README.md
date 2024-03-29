# A Guide to Use Database Tools

Note: all of the following are guides to setup/run the Database project (inside `Database` folder)
## Table of Contents
[Environment](#environment)

[Installation](#installation)

[Usage & Run Guide](#usage-and-run-guide)

[Cross-language Support](#cross\-language-api-support)

## Environment

System Requirements

```
Ubuntu 18.04.2 LTS
CentOS 7 (check release for binaries)
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

### Compilation
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

## Usage and Run Guide

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

ID: US20140369904A1
title: PROCESSES FOR PREPARING ALUMINA AND VARIOUS OTHER PRODUCTS
abstract: There are provided processes for preparing alumina. These processes can comprise leaching an aluminum-containing material with HCl so as to obtain a leachate comprising aluminum ions and a solid, and separating said solid from said leachate; reacting said leachate with HCl so as to obtain a liquid and a precipitate comprising said aluminum ions in the form of AlCl 3 , and separating said precipitate from said liquid; and heating said precipitate under conditions effective for converting AlCl 3  into Al 2 O 3  and optionally recovering gaseous HCl so-produced. These processes can also be used for preparing various other products such as hematite, MgO, silica and oxides of various metals, sulphates and chlorides of various metals, as well as rare earth elements, rare metals and aluminum. 
claim: 1 . A process for preparing alumina, said process comprising:

```

```
@error.txt

getInfoById: ID [US20140332418A1] does not exist in database
```

## Cross-language API Support
This project uses [SWIG](http://www.swig.org/) as a primariy tool to build cross-language wrapper 
to use the core database query functionality written & compiled in C++. Therefore, to be able to
compile a library compatible with user's OS system, additional tools and libraries used by SWIG must
be installed. Please follow the instructions of listed by 
[SWIG Github Wiki](https://github.com/swig/swig/wiki/Getting-Started)
to install prerequisites for SWIG installation.

After the environment for installing SWIG is setup. Download and extract swig from link

`https://github.com/swig/swig/archive/rel-4.0.0.tar.gz`

Following the guide on [SWIG's website](http://swig.org/svn.html):

Untar it with 
```shell script
tar zxf swig-rel-4.0.0.tar.gz
cd swig-rel-4.0.0
```
Run
```shell script
./autogen.sh
./configure
make -j8
sudo make install -j8
```
to install SWIG.

Lastly, Run
```
make -k check -j8
```
to check whether or not the installation is correct.

(Note: the -j option is dependent on the number of CPU cores on your machine, for n cores, use -jn to compile) 

### Python API Wrapper for `DatabaseQueryManager`
#### Compilation 
Run 
```shell script
cmake --build . --target DatabaseQueryPython -- -j 8
```
to compile and generate the shared library and Python wrapper.

The compiled Python Wrapper will be placed in `tools/swig_out/python`

Use test.py file located in `tools/swig_out/python` as an example
to use the generated python API wrapper.

For additional documentation, refer to PyDoc written in DatabaseQueryPython.py
for guidance.

### Java API Wrapper for `DatabaseQueryManager`
Run 
```shell script
cmake --build . --target DatabaseQueryJava -- -j 8
```
to compile and generate the shared library and Java wrapper.
