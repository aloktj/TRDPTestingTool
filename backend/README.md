# TRDPTestingTool Backend

This backend provides basic configuration upload and listing APIs used by the simulator frontend.

## Endpoints

- `POST /api/configs/upload`
  - Multipart form upload with the field name `file`.
  - Accepts XML files up to 5 MB.
  - Saves the file under `configs/` with a generated ID and records metadata in `configs/metadata.json`.

- `GET /api/configs`
  - Returns `{ id, filename, uploadedAt }` for every uploaded configuration.

- `GET /api/configs/{id}/summary`
  - Parses the stored XML to return device, interface, and dataset summaries.

- `POST /api/configs/{id}/activate`
  - Loads the stored XML into the TRDP engine and restarts it with the new configuration.
  - Marks the chosen configuration as active in `configs/metadata.json`.

Run the service locally with:

```
npm start
```

The server listens on port `3001` by default. Set `PORT` to override.
