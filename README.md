# Blackberry engine

*Blackberry is an early stage 2D game engine in development engine which currently is only guaranteed to work on Windows.*

### Bugs are to be expected!

If you find any bugs using the engine please either create an issue or a pull request on Github.

## Getting started

*Currently only* **Windows** *is supported and* **Visual Studio 2022** *is recommended, we will add support for other build systems and platforms as the engine matures*

### 1. Cloning the repository

Start by cloning the Github repository by typing `git clone --recursive https://github.com/AlignedMods/Blackberry.git` into a terminal. This will clone the repository into a folder called Blackberry.

If you forgot to clone recursively you can run `git submodule update --init` while inside the folder where you cloned Blackberry.

### 2. Generating build files

After cloning the repository you can run the `gen_vs2022.bat` script inside the `scripts` directory to generate Visual Studio 2022 project files.

Now you can open the `Blackberry.sln` solution file generated in the root directory of the project inside of Visual Studio.

You can now freely edit/build the source code of the engine.
If you happen to add/remove any source file make sure to re-run the setup script! 

## The plan

*NOTE: This is subject to change over time*

The plan currently is to create a powerful 2D game engine while maintaining simplicity wherever possible, we still try to prioritize flexibility though.