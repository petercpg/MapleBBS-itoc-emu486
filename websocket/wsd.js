const WebSocket = require('ws');
const net = require('net');

// Configuration: Set the target Telnet service
const TARGET_HOST = process.env.BBS_HOST || '127.0.0.1';
const TARGET_PORT = process.env.BBS_PORT || 23;
const WS_PORT = process.env.WS_PORT || 8888;

const server = new WebSocket.Server({ port: WS_PORT });

console.log(`WebSocket BBS Bridge listening on port ${WS_PORT}`);
console.log(`Proxying to Telnet: ${TARGET_HOST}:${TARGET_PORT}`);

server.on('connection', (ws) => {
    console.log('New WebSocket connection');

    // Connect to the local BBS Telnet service
    const client = new net.Socket();

    client.connect(TARGET_PORT, TARGET_HOST, () => {
        console.log('Connected to BBS Telnet service');

        // Send PROXY v1 header to pass real client IP
        // Format: PROXY TCP4 <src_ip> <dst_ip> <src_port> <dst_port>\r\n
        const remoteAddr = ws._socket.remoteAddress.replace(/^::ffff:/, '');
        const remotePort = ws._socket.remotePort;
        const localAddr = ws._socket.localAddress.replace(/^::ffff:/, '');
        const localPort = ws._socket.localPort;

        const proxyHeader = `PROXY TCP4 ${remoteAddr} ${localAddr} ${remotePort} ${localPort}\r\n`;
        client.write(proxyHeader);
    });

    ws.on('message', (message) => {
        // Handle buffer or string
        const data = Buffer.isBuffer(message) ? message : Buffer.from(message);
        client.write(data);
    });

    client.on('data', (data) => {
        ws.send(data);
    });

    client.on('error', (err) => {
        console.error('Telnet connection error:', err);
        ws.close();
    });

    client.on('close', () => {
        console.log('BBS Telnet service closed connection');
        ws.close();
    });

    ws.on('close', () => {
        console.log('Client disconnected from WebSocket');
        client.destroy();
    });

    ws.on('error', (err) => {
        console.error('WebSocket error:', err);
        client.destroy();
    });
});
