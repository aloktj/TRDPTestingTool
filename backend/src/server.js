const express = require('express');
const multer = require('multer');
const fs = require('fs');
const path = require('path');
const { randomUUID } = require('crypto');
const TrdpConfigLoader = require('./trdpConfigLoader');

const app = express();
const port = process.env.PORT || 3001;

const configsDir = path.join(__dirname, '..', 'configs');
const metadataPath = path.join(configsDir, 'metadata.json');

function ensureDataDirectory() {
  if (!fs.existsSync(configsDir)) {
    fs.mkdirSync(configsDir, { recursive: true });
  }

  if (!fs.existsSync(metadataPath)) {
    fs.writeFileSync(metadataPath, '[]', 'utf8');
  }
}

async function loadMetadata() {
  try {
    const data = await fs.promises.readFile(metadataPath, 'utf8');
    if (!data.trim()) {
      return [];
    }
    return JSON.parse(data);
  } catch (error) {
    console.error('Failed to read metadata file', error);
    return [];
  }
}

async function saveMetadata(entries) {
  await fs.promises.writeFile(metadataPath, JSON.stringify(entries, null, 2), 'utf8');
}

ensureDataDirectory();

const upload = multer({
  storage: multer.memoryStorage(),
  fileFilter: (_req, file, cb) => {
    const isXml =
      file.mimetype === 'text/xml' ||
      file.mimetype === 'application/xml' ||
      file.originalname.toLowerCase().endsWith('.xml');

    if (!isXml) {
      return cb(new Error('Only XML files are allowed.'));
    }
    cb(null, true);
  },
  limits: { fileSize: 5 * 1024 * 1024 },
});

app.get('/api/configs', async (_req, res) => {
  try {
    const metadata = await loadMetadata();
    res.json(metadata.map(({ id, filename, uploadedAt }) => ({ id, filename, uploadedAt })));
  } catch (error) {
    console.error('Failed to load configs', error);
    res.status(500).json({ message: 'Failed to read configuration list.' });
  }
});

app.get('/api/configs/:id/summary', async (req, res) => {
  const { id } = req.params;

  try {
    const metadata = await loadMetadata();
    const entry = metadata.find((item) => item.id === id);
    if (!entry) {
      return res.status(404).json({ message: 'Configuration not found.' });
    }

    const filePath = path.join(configsDir, entry.storedName || entry.filename);
    const loader = new TrdpConfigLoader();
    const summary = await loader.loadSummary(filePath);

    res.json(summary);
  } catch (error) {
    console.error('Failed to load configuration summary', error);
    res.status(500).json({ message: 'Failed to parse configuration file.' });
  }
});

app.post('/api/configs/upload', upload.single('file'), async (req, res) => {
  if (!req.file) {
    return res.status(400).json({ message: 'No file uploaded. Please attach an XML file.' });
  }

  const { originalname, buffer } = req.file;
  const id = randomUUID();
  const extension = path.extname(originalname) || '.xml';
  const storedName = `${id}${extension}`;
  const filePath = path.join(configsDir, storedName);
  const uploadedAt = new Date().toISOString();

  try {
    await fs.promises.writeFile(filePath, buffer);
    const metadata = await loadMetadata();
    metadata.push({ id, filename: originalname, storedName, uploadedAt });
    await saveMetadata(metadata);

    res.status(201).json({ id, filename: originalname, uploadedAt });
  } catch (error) {
    console.error('Failed to save uploaded file', error);
    res.status(500).json({ message: 'Failed to store configuration file.' });
  }
});

app.use((err, _req, res, _next) => {
  if (err) {
    console.error('Unhandled error', err);
    return res.status(400).json({ message: err.message || 'Request failed.' });
  }
});

app.listen(port, () => {
  console.log(`Config service listening on port ${port}`);
});

module.exports = app;
