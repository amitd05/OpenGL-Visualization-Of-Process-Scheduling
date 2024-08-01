
# Process Scheduling Algorithm Visualization

This project provides a visualization of various process scheduling algorithms using OpenGL. It is designed to help understand how different scheduling algorithms work and how they manage process execution in a simulated environment.

## Features

- Visualization of common process scheduling algorithms:
  - First-Come, First-Served (FCFS)
  - Shortest Job Next (SJN)
  - Priority Scheduling
  - Round Robin (RR)
  - Preemptive Priority Scheduling
- Real-time simulation with interactive controls
- Detailed views of process states and transitions
- Customizable simulation parameters

## Prerequisites

Before you begin, ensure you have met the following requirements:

- **Operating System:** Windows, macOS, or Linux
- **Development Environment:** C++ compiler, OpenGL libraries, and a compatible IDE (e.g., Visual Studio, Code::Blocks)
- **Dependencies:** 
  - OpenGL
  - GLUT (OpenGL Utility Toolkit)
  - GLU (OpenGL Utility Library)

## Installation

1. **Clone the repository:**

    ```sh
    git clone https://github.com/amitd05/OpenGL-Visualization-Of-Process-Scheduling.git
    cd OpenGL-Visualization-Of-Process-Scheduling
    ```

2. **Install dependencies:**

   Ensure that OpenGL, GLUT, and GLU are installed on your system. You may need to install additional packages depending on your operating system. Here are some examples:

   - **Windows:** Install freeglut or use the libraries provided with your IDE.
   - **macOS:** Use Homebrew to install freeglut.
     ```sh
     brew install freeglut
     ```
   - **Linux:** Install the required packages using your package manager.
     ```sh
     sudo apt-get install freeglut3 freeglut3-dev
     ```

3. **Compile the project:**

   Use your preferred C++ compiler to build the project. Here is an example using `g++`:
   
   ```sh
   g++ -o scheduler main.cpp scheduler.cpp visualization.cpp -lGL -lGLU -lglut
   ```

## Usage

1. **Run the executable:**

    ```sh
    ./scheduler
    ```

2. **Interact with the simulation:**

   - Select the scheduling algorithm you want to visualize.
   - Adjust simulation parameters such as process arrival times, burst times, priorities, and time quantum (for Round Robin).

3. **View process states and transitions:**

   Observe how the chosen scheduling algorithm manages process execution. The visualization will show processes moving through different states, including ready, running, and waiting.

## License

This project is licensed under the Apache License, Version 2.0. See the [LICENSE](LICENSE) file for details.

## Contributing

Contributions are welcome! Please follow these steps to contribute to this project:

1. Fork the repository.
2. Create a new branch.
3. Make your changes.
4. Submit a pull request.

## Authors

- Amit Sandeep Raikar(https://github.com/amitd05)

## Acknowledgments

- OpenGL community and contributors
- Any other libraries or tools you used



.
