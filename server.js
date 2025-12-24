// server.js
const http = require("http");
const fs = require("fs");
const path = require("path");
const WebSocket = require("ws");

const server = http.createServer((req, res) => {
    let file = req.url === "/" ? "index.html" : req.url.substring(1);
    let filePath = path.join(__dirname, file);

    if (!fs.existsSync(filePath)) {
        res.writeHead(404);
        return res.end("Not Found");
    }

    res.writeHead(200);
    fs.createReadStream(filePath).pipe(res);
});

const wss = new WebSocket.Server({ server });

wss.on("connection", ws => {
    console.log("WebSocket client connected");

    ws.on("message", msg => {
        console.log("MSG:", msg.toString());

        // Broadcast to all clients
        wss.clients.forEach(client => {
            if (client.readyState === WebSocket.OPEN) {
                client.send(msg.toString());
            }
        });
    });

    ws.on("close", () => console.log("Client disconnected"));
});

server.listen(8080, () => {
    console.log("Server running at http://localhost:8080");
});
