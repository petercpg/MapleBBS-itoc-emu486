# MapleBBS WebSocket Interface

This document explains how to set up and run the WebSocket interface for MapleBBS.

## Overview

The WebSocket interface is provided by a Node.js bridge (`websocket/wsd.js`) that:
1. Listens for WebSocket connections on port 8888.
2. Connects to the local BBS Telnet service (port 23).
3. Pipes data between the WebSocket and the Telnet session, passing the original client IP via the PROXY protocol.

## Prerequisites

- Node.js (v24+ recommended)
- `ws` library (`npm install ws`)

## Configuration

The bridge is configured via environment variables. If you are using Systemd, you should modify the service file:

### Modifying Ports and IP
1. Edit `/etc/systemd/system/maplebbs-ws.service`:
   - `WS_PORT`: The port the WebSocket server listens on (default: `8888`).
   - `BBS_HOST`: The IP of the BBS Telnet service (default: `127.0.0.1`).
   - `BBS_PORT`: The port of the BBS Telnet service (default: `23`).
2. Reload and restart:
   ```bash
   sudo systemctl daemon-reload
   sudo systemctl restart maplebbs-ws
   ```

## Running the WebSocket Bridge

### Manual Start
```bash
cd websocket
npm install
node wsd.js
```

### Systemd Service (Recommended)
A service file is provided at `systemd/maplebbs-ws.service`. To install it:
```bash
# Edit the WorkingDirectory path in the file first!
sudo cp systemd/maplebbs-ws.service /etc/systemd/system/
sudo systemctl daemon-reload
sudo systemctl enable maplebbs-ws
sudo systemctl start maplebbs-ws
```

## Nginx Configuration (WSS)

To use WebSocket over SSL (WSS), you should use Nginx as a reverse proxy:

```nginx
server {
    listen 443 ssl;
    server_name bbs.example.com;

    ssl_certificate /path/to/cert.pem;
    ssl_certificate_key /path/to/key.pem;

    location /ws {
        proxy_pass http://localhost:8888;
        proxy_http_version 1.1;
        proxy_set_header Upgrade $http_upgrade;
        proxy_set_header Connection "Upgrade";
        proxy_set_header Host $host;
    }
}
```

## Client Connection

You can connect to the BBS using any WebSocket-capable client. For example, using `wscat`:

```bash
wscat -c ws://localhost:8888
```

Or using an in-browser BBS client like PttChrome or similar.
