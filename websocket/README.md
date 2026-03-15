# MapleBBS WebSocket Interface

This document explains how to set up and run the WebSocket interface for MapleBBS.

## Overview

The WebSocket interface is provided by a Node.js bridge (`websocket/wsd.js`) that:
1. Listens for WebSocket connections on port 8888.
2. Spawns a BBS process for each connection.
3. Pipes data between the WebSocket and the BBS process.

## Prerequisites

- Node.js (v24+ recommended)
- `ws` library (`npm install ws`)
- `script` command (usually available on Linux/BSD)

## Running the WebSocket Bridge

To set up and start the bridge:

```bash
cd websocket
npm install  # Install dependencies
node wsd.js  # Start the bridge
```

You can change the port and the path to the BBS binary in `websocket/wsd.js`.

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
