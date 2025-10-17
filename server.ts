import * as http from 'http';
import * as fs from 'fs';
import * as fsp from 'fs/promises';
import * as path from 'path';
const Busboy = require('busboy');

const htmlFile = 'index.html';
const dataManagerAddon = require(path.join(
  __dirname,
  'packages/data_manager_addon/build/Release/data_manager_addon.node'
));
const hostname = '0.0.0.0';
const port = 3000;
const uploadDir = path.join(__dirname, 'data');

const MIME_TYPES: { [key: string]: string } = {
  '.html': 'text/html',
  '.css': 'text/css',
  '.js': 'text/javascript',
  '.png': 'image/png',
  '.jpg': 'image/jpeg',
  '.gif': 'image/gif',
};

const server = http.createServer(
  async (req: http.IncomingMessage, res: http.ServerResponse) => {
    const sendJson = (statusCode: number, data: any) => {
      res.statusCode = statusCode;
      res.setHeader('Content-Type', 'application/json');
      res.end(JSON.stringify(data));
    };

    const sendFile = async (filePath: string) => {
      try {
        const ext = path.extname(filePath);
        const contentType = MIME_TYPES[ext] || 'application/octet-stream';
        const data = await fsp.readFile(filePath);
        res.statusCode = 200;
        res.setHeader('Content-Type', contentType);
        res.end(data);
      } catch (err) {
        console.error('Error reading file:', err);
        sendJson(500, { message: 'Internal Server Error' });
      }
    };

    const url = new URL(req.url || '/', `http://${req.headers.host}`);

    if (req.method === 'GET') {
      const filePath = path.join(
        __dirname,
        'public',
        url.pathname === '/' ? htmlFile : url.pathname
      );
      await sendFile(filePath);
    } else if (req.method === 'POST' && url.pathname === '/api/create') {
      let body = '';
      req.on('data', (chunk) => (body += chunk.toString()));
      req.on('end', async () => {
        try {
          let records: number;
          try {
            const parsedBody = JSON.parse(body);
            records = parseInt(parsedBody.recordCount, 10);
          } catch (e) {
            sendJson(400, { message: 'Invalid JSON payload.' });
            return;
          }

          if (isNaN(records) || records <= 0) {
            sendJson(400, {
              message: 'Please provide a valid number of records.',
            });
            return;
          }

          dataManagerAddon.generateCsv(records);
          sendJson(200, {
            message: `Successfully created ${records} records in output.csv.`,
          });
        } catch (error: any) {
          console.error('Error in /api/create:', error);
          sendJson(500, {
            message: 'Error processing create request.',
            error: error.message,
          });
        }
      });
    } else if (req.method === 'POST' && url.pathname === '/api/upload') {
      if (!fs.existsSync(uploadDir)) {
        fs.mkdirSync(uploadDir);
      }

      const busboy = Busboy({ headers: req.headers });
      let filePath: string;

      busboy.on(
        'file',
        (fieldname: string, file: NodeJS.ReadableStream, info: any) => {
          // Extract filename from the info object
          const filename = info.filename;

          // Check that filename is a string before proceeding
          if (typeof filename !== 'string') {
            console.error('Busboy received invalid filename.');
            file.resume(); // Resume the stream to prevent it from hanging
            return;
          }

          filePath = path.join(uploadDir, filename);
          const saveTo = fs.createWriteStream(filePath);
          file.pipe(saveTo);
        }
      );

      busboy.on('finish', async () => {
        try {
          const dbPath = path.join(__dirname, 'data.db');
          await fsp.unlink(dbPath).catch(() => {});

          if (filePath) {
            dataManagerAddon.importCsv(filePath);
            sendJson(200, {
              message: `Successfully uploaded and imported new data.`,
            });
          } else {
            sendJson(400, { message: 'No file uploaded.' });
          }
        } catch (error: any) {
          console.error('Error in /api/upload:', error);
          sendJson(500, {
            message: 'Error processing upload request.',
            error: error.message,
          });
        } finally {
          if (filePath) {
            await fsp
              .unlink(filePath)
              .catch((err) => console.error('Error deleting temp file:', err));
          }
        }
      });

      req.pipe(busboy);
    } else {
      res.statusCode = 404;
      res.setHeader('Content-Type', 'text/plain');
      res.end('Not Found');
    }
  }
);

server.listen(port, hostname, () => {
  console.log(`Server running at http://${hostname}:${port}/`);
});
