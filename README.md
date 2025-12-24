# Graphing Calculator

A high-performance graphing calculator written in C using Raylib.

## Features

- **Equation Graphing**: Support for multiple equations (`y = ...`, `y < ...`, etc.).
- **Inequalities**: Graph regions using inequalities (`<`, `>`, `<=`, `>=`).
- **Interactive UI**:
  - Click-to-edit equation fields.
  - Virtual Keyboard for easy input.
- **Controls**:
  - **Pan**: Drag with Left Mouse Button (outside input fields) or Right Mouse Button.
  - **Zoom**: Mouse Wheel.
  - **Focus**: Click input field to type.
  - **Toggle Keyboard**: Press `K` or click the toggle text.
  - **Clear Line**: Press `C` on virtual keyboard.

## Building parts

### Windows (MSVC/MinGW)
1. Ensure you have a C compiler (gcc)
2. Run `build.bat` in the terminal.
   ```cmd
   .\build.bat
   ```

## Usage
Run `graph_calc.exe` after building.
