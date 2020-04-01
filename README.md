[![Build Status](https://drone.magical.rocks/api/badges/SetZero/G474RE-CORDIC/status.svg)](https://drone.magical.rocks/SetZero/G474RE-CORDIC) ![GitHub release](https://img.shields.io/github/v/release/SetZero/G474RE-CORDIC?include_prereleases) ![GitHub](https://img.shields.io/github/license/SetZero/G474RE-CORDIC)

# G474RE-CORDIC

This project deals with the realization of a C++ implementation for the use of the CORDIC hardware and comparison of the performance with the software-based trigonometric functions of GCC (g++) and CLang (clang++). In addition to the comparison, special emphasis is also placed on the documentation in order to make it as easy as possible for subsequent users to get started.

## What is CORDIC?

CORDIC (for COordinate Rotation DIgital Computer), also known as Volder's algorithm, is a simple and efficient algorithm to calculate hyperbolic and trigonometric functions, typically converging with one digit (or bit) per iteration. CORDIC is therefore also an example of digit-by-digit algorithms. CORDIC and closely related methods known as pseudo-multiplication and pseudo-division or factor combining are commonly used when no hardware multiplier is available (e.g. in simple microcontrollers and FPGAs), as the only operations it requires are addition, subtraction, bitshift and table lookup. As such, they belong to the class of shift-and-add algorithms.


## Requirements 

This project requires ```arm-none-eabi-gcc``` and ```st-flash```. The following describes how these packages can be installed under the respective Linux distributions

### Arch Linux

```bash
sudo pacman -S arm-none-eabi-newlib stlink arm-none-eabi-gcc
```

## Bulid Instructions

Make sure that the requirements are met and then execute the following commands.

Clone the repository (if not done already)

```bash
git clone https://github.com/SetZero/G474RE-CORDIC/
cd G474RE-CORDIC
```

Build using ```make```

```bash
make
```
