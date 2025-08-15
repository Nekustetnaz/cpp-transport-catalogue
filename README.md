# Transport Routing System

A C++ implementation of a public transport routing system that includes:
- Transport catalogue management
- Route information calculation
- Map rendering to SVG
- JSON input/output processing
- Optimal route finding with graph algorithms

## Features

### Core Functionality
- **Transport Catalogue**: Stores and manages bus stops and routes
- **Route Information**: Calculates route length, curvature, and stops count
- **Distance Tracking**: Manages real road distances between stops
- **Map Rendering**: Generates SVG maps of transport routes
- **Routing System**: Finds optimal routes between stops with wait/ride times

### Data Processing
- **JSON I/O**: Processes input requests and generates responses in JSON format
- **Request Handling**: Manages stat requests for buses, stops, and routes
- **Configuration**: Supports render and routing settings customization

## System Architecture
### Main Components
transport-catalague/

├── domain.h # Stop, Bus, and RouteInfo structures

├── geo.h/cpp # Geographical calculations

├── graph.h # Directed weighted graph implementation

├── router.h # Route finding algorithms

├── json.h/cpp # JSON node and document handling

├── json_builder.h/cpp# JSON builder pattern

├── json_reader.h/cpp # JSON request processing

├── map_renderer.h/cpp# SVG map generation

├── svg.h/cpp # SVG primitives and rendering

├── transport_catalogue.h/cpp # Transport data management

├── transport_router.h/cpp # Routing logic

└── request_handler.h/cpp # Request processing

### Key Data Structures
- **Stop**: Represents a transport stop with name and coordinates
- **Bus**: Contains route information including stops and roundtrip flag
- **RouteInfo**: Calculated route metrics (length, stops, curvature)
- **Graph**: Directed weighted graph for route optimization
- **JSON Nodes**: Variant-based JSON data representation

## Build and Run

### Prerequisites
- C++17 compatible compiler
- CMake (recommended)

### Build Instructions
mkdir build && cd build
cmake ..
make

## Running the Program
./transport_router < input.json > output.json

## Configuration

### Render Settings
Configure map appearance via JSON:
- Dimensions, padding, colors
- Stop/bus label settings
- Line widths and radii

### Routing Settings
Customize routing behavior:
- Bus wait time (minutes)
- Bus velocity (km/h)

Performance Considerations
- Graph pre-building for fast route queries
- Efficient spatial indexing for large datasets
- Optimized SVG rendering
