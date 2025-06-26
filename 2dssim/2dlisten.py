import socket

def listen_for_udp(port=5000):
    """Listen for incoming UDP packets."""
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind(('0.0.0.0', port))
    print(f"Listening for UDP packets on port {port}...")
    
    try:
        while True:
            data, addr = sock.recvfrom(4096)
            print(f"Received {len(data)} bytes from {addr}")
    except KeyboardInterrupt:
        print("Stopped listening")
    finally:
        sock.close()

# Start listening
listen_for_udp(5000)