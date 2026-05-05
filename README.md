# tiny-ipc - C++ IPC Library

High performance inter process communication library based on shared memory.

## Features

- **Shared Memory Based**: High-performance IPC using shared memory
- **Two Transmission Modes**:
  - **Route (Broadcast)**: N-to-N communication - multiple writers can send to multiple readers
  - **Channel (Unicast)**: N-to-1 communication - multiple writers send to a single reader
- **Callback Driven**: Asynchronous event handling for connection and message events
- **Cross Platform**: Supports Windows and Linux

## API Overview

### Core Classes

- `ipc::Ipc<Wr<Transmission::BROADCAST>>` (Route): Broadcast channel for N-to-N communication
- `ipc::Ipc<Wr<Transmission::UNICAST>>` (Channel): Unicast channel for N-to-1 communication
- `ipc::Buffer`: Smart pointer managed buffer for message data
- `ipc::Callback`: Base class for handling async events

### Connection Modes

```cpp
ipc::SENDER    // Sender mode
ipc::RECEIVER  // Receiver mode
```

### Transmission Types

```cpp
ipc::Transmission::UNICAST   // N-to-1 communication (Channel)
ipc::Transmission::BROADCAST // N-to-N communication (Route)
```

### Callback Events

- `connected()`: Connection established
- `connection_lost()`: Connection lost
- `message_arrived()`: Message received
- `delivery_complete()`: Message sent successfully

## Usage Examples

### Channel (Unicast - N to 1)

```cpp
#include <ipc/ipc.h>

// Sender
ipc::Channel ipc{"channel", ipc::SENDER};
ipc.write("Hello, World!");

// Receiver
ipc::Channel ipc{"channel", ipc::RECEIVER};
ipc.set_callback(std::make_shared<MyCallback>());
ipc.read();
```

### Route (Broadcast - N to N)

```cpp
#include <ipc/ipc.h>

// Sender
ipc::Route ipc{"route", ipc::SENDER};
ipc.write("Hello, World!");

// Receiver
ipc::Route ipc{"route", ipc::RECEIVER};
ipc.set_callback(std::make_shared<MyCallback>());
ipc.read();
```

### Custom Callback

```cpp
class MyCallback : public ipc::Callback {
public:
    void message_arrived(const ipc::Buffer* buf, const ipc::ErrorCode& cause) override {
        std::string msg(static_cast<const char*>(buf->data()), buf->size());
        std::cout << "Received: " << msg << std::endl;
    }
};
```

## Error Codes

| Error Code | Description |
|------------|-------------|
| `IPC_ERR_SUCCESS` | Success |
| `IPC_ERR_NOINIT` | Not initialized |
| `IPC_ERR_NOMEM` | Out of memory |
| `IPC_ERR_INVAL` | Invalid argument |
| `IPC_ERR_NO_CONN` | No connection |
| `IPC_ERR_CONN_REFUSED` | Connection refused |
| `IPC_ERR_NOT_FOUND` | Resource not found |
| `IPC_ERR_CONN_LOST` | Connection lost |
| `IPC_ERR_NOT_SUPPORTED` | Not supported |
| `IPC_ERR_UNKNOWN` | Unknown error |

## Timeout

- `ipc::TimeOut::DEFAULT_TIMEOUT`: 100ms
- `ipc::TimeOut::INVALID_TIMEOUT`: Infinite wait

## Build

```bash
mkdir build && cd build
cmake ..
make
```

## License

MIT License