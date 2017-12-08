<p align="center"><img src="https://raw.githubusercontent.com/chengluyu/PL0/master/pl0.png" width=200></p>

Just another PL/0 interpreter.

## Features

1. Written in modern C++ and object-oriented paradigm.
2. Be able to generate versatile ast
3. Be able to generate picture of the ast 

## Build

PL/0 uses cmake as its building toolchains, which is available in all platforms. To build PL/0, first and formost, you should clone this project to your computer.
Then you can follow the instructions below:

<details>
<summary>For CLion Users</summary>
If you're using CLion as your C++ IDE, you can load this project directly into CLion by clicking "Import Project from Sources" on the startup screen of CLion.
</details>

<details>
<summary>For macOS Users</summary>
If you're using CLion as your C++ IDE, you can load this project directly into CLion by clicking "Import Project from Sources" on the startup screen of CLion.
</details>

<details>
<summary>For Linux (or Cygwin) Users </summary>
If you're using Linux (or Cygwin). The best way is using GNU make. Just `cd` into this project and execute following command.

```shell
mkdir -p cmake-build
cd cmake-build
cmake .. -G "Unix Makefiles"
```
</details>

<details>
<summary>For Windows Users</summary>
<br>
Make sure that the Microsoft C++ compiler is installed with your Visual Studio. Then execute `cmake .` in this project's directory. CMake will generate Visual Studio solution file automatically.
</details>

## Usage

Built binary of PL/0 interpreter has few options:

* `--compile-only`: compile but not run the code
* `--show-bytecode`: print bytecode after generating the code
* `--show-ast`: print ast after generating the ast
* `--plot-tree [output_file]`: save DOT (a graphics description language) into `output_file`, you can generate a picture of the ast by graphviz.

## License

MIT
