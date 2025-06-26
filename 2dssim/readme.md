# 2DS UDP Simulator

## Usage

`python 2dsend.py /path/to/file.F2DS [--ip IP_ADDRESS] [--port PORT] [--delay SECONDS] [--data-only]`
For testing, it is set up to send the file back to your IP on port 5000. You can run the 2dlisten.py to ensure it's reading the 4096 byte data package.
`python 2dlisten.py`



## Arguments
- **filename**: Path to the OAP file to stream
- **--ip**: Target IP address (default: `127.0.0.1`)
- **--port**: Target port number (default: `5000`)
- **--delay**: Delay between packets in seconds (default: `0.2`)
- **--data-only**: Send only the 4096-byte data portion instead of the full 4114-byte record