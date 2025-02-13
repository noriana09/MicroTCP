# 🚀 MicroTCP: Lightweight TCP over UDP

## 📝 Description

This project implements a **lightweight TCP protocol** over **UDP**, designed for **IoT devices** with limited resources. It provides reliable communication using UDP, focusing on **connection establishment** and **termination**. The project is divided into two phases:

1. **Phase A**: Basic MicroTCP implementation, including **3-way handshake** and **connection termination**.
2. **Phase B**: Advanced features like acknowledgments, retransmissions, and congestion control.

This repository contains the **Phase A** implementation.

---

## 🛠️ Key Features

- **🔗 3-Way Handshake**: Establishes a reliable connection using SYN, SYN-ACK, and ACK packets.
- **🚪 Connection Termination**: Gracefully closes the connection with FIN and ACK packets.
- **📊 Bandwidth Testing Tool**: Measures bandwidth using MicroTCP and standard TCP.

---

## 📂 Project Structure

- **📁 lib**: Core MicroTCP implementation.
  - `microtcp.c`: MicroTCP functions (e.g., `microtcp_socket`, `microtcp_connect`).
  - `microtcp.h`: Header file for MicroTCP structures.
  
- **📁 test**: Bandwidth testing tool.
  - `bandwidth_test.c`: Measures bandwidth.

- **📁 utils**: Utility functions.
  - `crc32.h`: CRC32 checksum calculation.

---

## 🧩 MicroTCP Packet Header

| Field               | Size (bits) | Description                          |
|---------------------|-------------|--------------------------------------|
| **Sequence Number** | 32          | Packet sequence number.              |
| **ACK Number**      | 32          | Expected acknowledgment number.      |
| **Control**         | 16          | Flags (SYN, ACK, RST, FIN).          |
| **Window**          | 16          | Bytes the sender is willing to receive. |
| **Data Length**     | 32          | Size of data (excluding header).     |
| **CRC32 Checksum**  | 32          | Ensures data integrity.              |

---

## 🚀 How It Works

### 1. **3-Way Handshake** 🤝
- Client sends **SYN**.
- Server replies with **SYN-ACK**.
- Client sends **ACK** to establish the connection.

### 2. **Connection Termination** 🚪
- Client sends **FIN**.
- Server replies with **ACK** and sends its own **FIN**.
- Client sends **ACK** to close the connection.

### 3. **Bandwidth Testing Tool** 📊
- Measures bandwidth during file transfer using MicroTCP and TCP.

---

## 🛠️ Key Functions

- **`microtcp_socket`**: Creates a MicroTCP socket.
- **`microtcp_connect`**: Initiates a connection (client-side).
- **`microtcp_accept`**: Accepts a connection (server-side).
- **`microtcp_shutdown`**: Terminates the connection.

---

## 📊 Example Output

### Bandwidth Test
