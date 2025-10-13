import * as http from 'http';
import * as fs from 'fs';
import * as path from 'path';

// const http = require('http');
// const fs = require('fs');
// const path = require('path');

const hostname = '0.0.0.0';
const port = 3000;

// Resolve the path to the HTML file
const filePath = path.join(__dirname, 'public', 'index.html');

const server = http.createServer(
  (req: http.IncomingMessage, res: http.ServerResponse) => {
    // Read the HTML file asynchronously
    fs.readFile(filePath, (err: NodeJS.ErrnoException | null, data: Buffer) => {
      if (err) {
        console.error('Error reading file:', err);
        res.statusCode = 500;
        res.setHeader('Content-Type', 'text/plain');
        res.end('Internal Server Error');
        return;
      }

      // Send the HTML content
      res.statusCode = 200;
      res.setHeader('Content-Type', 'text/html');
      res.end(data);
    });
  }
);

server.listen(port, hostname, () => {
  console.log(`Server running at http://${hostname}:${port}/`);
});
