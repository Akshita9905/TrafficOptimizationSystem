TRAFFIC OPTIMIZATION SYSTEM
===========================

Technology:
- C++17 backend
- HTML/CSS/JavaScript frontend
- Leaflet + OpenStreetMap map
- Graph, Dijkstra, priority queue, traffic sensors

WINDOWS RUN INSTRUCTIONS
========================

1. Open this folder in VS Code.

2. Open Terminal and make sure the current directory is the project root.

3. Compile:

   g++ -std=c++17 backend/main.cpp backend/Graph.cpp backend/TrafficSensor.cpp backend/TrafficOptimizer.cpp -I. -lws2_32 -o TrafficOptimization.exe

4. Run:

   .\TrafficOptimization.exe

5. Open in your browser:

   http://localhost:8080

IMPORTANT:
- Keep httplib.h in the project root, next to CMakeLists.txt.
- Keep index.html, style.css and script.js inside frontend.
- Do not open index.html directly. Start the C++ server first.
- The included httplib.h is adjusted for the older MinGW/GCC toolchain used by this project.

If compilation fails, send the complete terminal error output.

THREE ROAD ROUTES
=================
The frontend requests up to three alternative road routes from the public OSRM demo routing service.
Route colors: blue (primary), orange (alternative), purple (alternative). OSRM may return fewer than three routes. Internet access is required.
The C++ Dijkstra graph route/cost and OSRM road alternatives are separate calculations in this demo.
