const WebSocket = require('ws');
const { spawn } = require('child_process');
const server = new WebSocket.Server({ port: 8888 });

console.log('WebSocket BBS Bridge listening on port 8888');

server.on('connection', (ws) => {
    console.log('New connection');

    // Default to /home/bbs/bin/bbs or common locations
    // We use 'script' to provide a PTY-like environment for the BBS process
    // This allows it to work with ANSI colors and interactive menus
    // Path should be relative to the project root where the script is usually run
    const bbs = spawn('script', ['-qec', './bin/bbs', '/dev/null'], {
        env: { ...process.env, TERM: 'xterm-256color' }
    });

    ws.on('message', (message) => {
        // Handle buffer or string
        const data = Buffer.isBuffer(message) ? message : Buffer.from(message);
        bbs.stdin.write(data);
    });

    bbs.stdout.on('data', (data) => {
        ws.send(data);
    });

    bbs.stderr.on('data', (data) => {
        ws.send(data);
    });

    bbs.on('close', (code) => {
        console.log(`BBS process exited with code ${code}`);
        ws.close();
    });

    ws.on('close', () => {
        console.log('Client disconnected');
        bbs.kill();
    });

    ws.on('error', (err) => {
        console.error('WebSocket error:', err);
        bbs.kill();
    });
});
