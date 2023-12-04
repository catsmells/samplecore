const express = require('express');
const http = require('http');
const socketIo = require('socket.io');
const app = express();
const server = http.createServer(app);
const io = socketIo(server);
app.get('/', (req, res) => {
  res.sendFile(__dirname + '/index.html');
});
let connectedNodes = 0;
let totalData = 0;
io.on('connection', (socket) => {
  connectedNodes++;
  io.emit('update', { nodes: connectedNodes, data: totalData });
  socket.on('data', (data) => {
    totalData += data.length;
    io.emit('update', { nodes: connectedNodes, data: totalData });
  });
  socket.on('disconnect', () => {
    connectedNodes--;
    io.emit('update', { nodes: connectedNodes, data: totalData });
  });
});
server.listen(9001, () => {
  console.log('UI listening on port 9001');
});
