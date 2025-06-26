import struct
from datetime import datetime
import socket
import time
import struct
import argparse
import os
import sys

class P2dRec:
    """Class to represent a 4114-byte record with metadata."""
    HEADER_SIZE = 16  # Size of the header in bytes
    RECORD_SIZE = 4096  # Size of the record data in bytes
    CHECKSUM_SIZE = 2  # Size of the checksum in bytes
    TOTAL_SIZE = HEADER_SIZE + RECORD_SIZE + CHECKSUM_SIZE  # 4114 bytes total

    def __init__(self, record_data):
        """
        Initialize the P2dRec object by parsing the record header, data, and checksum.
        :param record_data: A 4,114-byte record.
        """
        self.year = None
        self.month = None
        self.day_of_week = None
        self.day = None
        self.hour = None
        self.minute = None
        self.second = None
        self.millisecond = None
        self.binary_data = None
        self.checksum = None
        self.timestamp = None
        
        self._parse_record(record_data)

    def _parse_record(self, record_data):
        """
        Parse the record with timestamp, data, and checksum.
        :param record_data: A 4,114-byte record.
        """
        if len(record_data) < self.TOTAL_SIZE:
            raise ValueError(f"Record data is too small: {len(record_data)} bytes, expected {self.TOTAL_SIZE}")
        
        # Parse the 8 timestamp fields (16 bytes total) using LITTLE ENDIAN format
        timestamp_data = struct.unpack('<8H', record_data[:self.HEADER_SIZE])
        
        # Assign timestamp fields
        (self.year, self.month, self.day_of_week, self.day, 
         self.hour, self.minute, self.second, self.millisecond) = timestamp_data
        
        # For debugging purposes
        print(f"Timestamp values: {timestamp_data}")
        
        # Create datetime object
        try:
            # Keep millisecond within valid range (0-999)
            ms = min(self.millisecond, 999)
            
            self.timestamp = datetime(
                year=self.year,
                month=self.month,
                day=self.day,
                hour=self.hour, 
                minute=self.minute, 
                second=self.second, 
                microsecond=ms * 1000
            )
        except ValueError as e:
            # Handle invalid date values
            self.timestamp = None
            print(f"Warning: Invalid timestamp values: {timestamp_data}")
            print(f"Error details: {e}")
        
        # Extract binary data (4096 bytes)
        data_start = self.HEADER_SIZE
        data_end = data_start + self.RECORD_SIZE
        raw_data = record_data[data_start:data_end]
        self.binary_data = ''.join(format(byte, '08b') for byte in raw_data)
        
        # Extract checksum (2 bytes)
        checksum_bytes = record_data[data_end:data_end + self.CHECKSUM_SIZE]
        self.checksum = struct.unpack('<H', checksum_bytes)[0]

def read_oap_file(filename,target_ip='127.0.0.1', target_port=5000, delay=0.2, send_full_record=False):
    """Read and decode an OAP file."""
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    records_sent = 0
    with open(filename, 'rb') as file:
        # Read the entire file content
        binary_data = file.read()

        # Read and decode each 4114-byte record
        record_size = P2dRec.TOTAL_SIZE  # 4114 bytes total
        for i in range(0, len(binary_data), record_size):
            record_data = binary_data[i:i + record_size]
            if len(record_data) < record_size:
                print(f"Skipping incomplete record at index {i}.")
                continue
                
            try:
                if send_full_record:
                        # Send the entire record (timestamp + data + checksum)
                        print(f"Sending full record {i//record_size + 1} to {target_ip}:{target_port}")
                        sock.sendto(record_data, (target_ip, target_port))
                else:
                    record = P2dRec(record_data)
                    # Convert binary string to bytes for UDP transmission
                    bytes_data = bytearray()
                    for j in range(0, len(record.binary_data), 8):
                        byte_str = record.binary_data[j:j+8]
                        if len(byte_str) == 8:  # Ensure we have a full byte
                            byte_val = int(byte_str, 2)
                            bytes_data.append(byte_val)
                    
                    # Ensure we have exactly 4096 bytes
                    if len(bytes_data) != 4096:
                        bytes_data = bytes_data[:4096].ljust(4096, b'\0')
                    
                    # Send the data via UDP
                    print(f"Sending record {i//record_size + 1} to {target_ip}:{target_port}")
                    sock.sendto(bytes_data, (target_ip, target_port))
                records_sent += 1
                
                # Add delay between packets
            
                time.sleep(delay)
            except Exception as e:
                print(f"Error processing record at index {i}: {e}")
    print(f"Finished streaming {records_sent} records")
    
def parse_args():
    """Parse command line arguments."""
    parser = argparse.ArgumentParser(description='Stream OAP data via UDP')
    parser.add_argument('filename', help='Path to the OAP file')
    parser.add_argument('--ip', default='127.0.0.1', help='Target IP address (default: 127.0.0.1)')
    parser.add_argument('--port', type=int, default=5000, help='Target port (default: 5000)')
    parser.add_argument('--delay', type=float, default=0.2, help='Delay between packets in seconds (default: 0.2)')
    parser.add_argument('--fullrecord', action='store_true', help='Send only the 4096-byte data portion (default: send data only)')
    return parser.parse_args()

def main():
    """Main entry point for the script."""
    args = parse_args()
    
    # Verify file exists
    if not os.path.isfile(args.filename):
        print(f"Error: File '{args.filename}' not found")
        sys.exit(1)
    
    print(f"Streaming OAP data from {args.filename} to {args.ip}:{args.port}")
    records_sent = read_oap_file(
        args.filename,
        target_ip=args.ip,
        target_port=args.port,
        delay=args.delay,
        send_full_record=args.fullrecord
        
    )
    
    print(f"Program completed, sent {records_sent} records")

if __name__ == "__main__":
    main()