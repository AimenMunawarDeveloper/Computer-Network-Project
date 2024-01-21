# Network Simulation Project

## Overview

This repository contains the source code for a network simulation project developed as part of the Computer Networks course (EE353) at the National University of Sciences and Technology. The project focuses on simulating a complex network scenario with 50 nodes, implementing various features such as point-to-point links, autonomous system (AS) divisions, routing algorithms, and packet transmission.

## Key Components

### 1. Node Setup and Connectivity

- 50 nodes are created and equipped with the Internet stack.
- Point-to-point links with random bandwidth values are established between nodes.
- Nodes are organized into five Autonomous Systems (AS) with specific ranges.

### 2. Dijkstra's Algorithm for Routing

- Dijkstra's algorithm is implemented for finding the shortest paths within and between ASes.
- Optimal paths from the source node (Node 0) to the destination node (Node 47) are calculated.

### 3. Animation Interface

- An animation interface is created using the NetAnim module to visualize the network topology.
- Node positions are set based on AS divisions to provide a clear understanding of the network structure.

### 4. Application and Traffic Generation

- UDP sockets are created for source and destination nodes to simulate packet transmission.
- The simulation is configured to transmit multiple packets, and the average delay and packet delivery ratio are calculated.

## Code Organization

The code is organized into different sections, including node setup, link configuration, routing algorithm implementation, animation interface setup, and application layer functionality. Each section is encapsulated into functions or classes to enhance code readability and maintainability.

## Usage

To run the simulation, follow these steps:

1. Clone the repository to your local machine.
2. Install the ns-3 network simulator (version XYZ) if not already installed.
3. Compile and execute the simulation script using the ns-3 tools.

```bash
$ ./waf configure
$ ./waf
$ ./waf --run simulation_script
```

## Results

After running the simulation, the console output will provide information about the network's performance, including the total shortest path, average delay, and packet delivery ratio.

### Example Console Output:

```bash
Overall Shortest path from node 0 to 48:
Shortest paths from node 0 to gateway 6: 3.4
Link cost for Gateway 6 to gateway 27: 200
Shortest paths from node 27 to gateway 23: 2.8
Link cost for Gateway 23 to gateway 49: 150
Shortest paths from node 49 to node 48: 4.1
Total Shortest Path from node 0 to 48: 360.3

Average Delay: 0.0023 seconds
Packet Delivery Ratio: 1.0
```
## Additional Considerations

- Randomization is used for node positions, link bandwidth, and AS divisions to simulate diverse network conditions.
- The NetAnim module visualizes the network topology, providing insights into AS divisions and interconnections.

## Acknowledgements

Special thanks to the contributors: Aimen Munawar, Maheen Akhtar Khan, Muqaddas Anees, and Hadia Ali.

## Contributing

Feel free to open issues for bug reports or feature requests. Contributions are welcome through pull requests.
