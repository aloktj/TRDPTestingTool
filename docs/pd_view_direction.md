# PD View reorientation toward telegram instances

## Clarifying dataset vs. telegram responsibilities
- **Datasets** (see `src/model/dataset_model.h`) are immutable schemas: field order, field types, and array extents. They never hold runtime values and are shared by many telegrams.
- **Telegrams** (see `src/model/sim_config.h`) are the runtime instances bound to a specific `data-set-id`/`comId`, sources, destinations, and cycle/timeout parameters. Each telegram must own its own TX/RX payload buffers so that multiple telegrams using the same dataset do not overwrite one another.

## Determining PD direction (incoming vs outgoing)
We already know the simulator host address from each interface (`InterfaceConfig::hostIp`). Every telegram lists concrete endpoints via `sources` and `destinations` with `uriHost` strings. That lets us infer direction without user input:

1. **Local sender (TX)**: when `interface.hostIp` matches any `telegram.sources[*].uriHost` (or the source is empty and we fall back to the interface host), the local node is the producer. The PD view should expose writable TX payload fields and a "Send/apply" action for that telegram.
2. **Local subscriber (RX)**: when `interface.hostIp` matches any `telegram.destinations[*].uriHost`, the local node is the consumer. The PD view should surface a read-only RX payload pane that updates when `PdEndpointRuntime::handleSubscription` fires.
3. **Both sides / loopback**: if the host appears in both source and destination lists, we should render both panes and keep TX and RX buffers independent.
4. **Fallbacks**: if neither side explicitly names the host, we can default to TX when the telegram is configured for publishing (e.g., `createEndpoint` or publisher buttons exist) and to RX when a subscription handler is registered, but the intent is to rely on explicit endpoint IPs to avoid guessing.

## PD view/engine changes to align with the model
- Store TX and RX payload buffers per **telegram runtime** (not the dataset) inside `PdEndpointRuntime`, with helpers to map dataset fields to byte offsets for editing and display.
- Drive the PD view from telegram instances, showing:
  - TX editor (enabled only when direction identifies local sender).
  - RX monitor (visible when local subscriber) that mirrors the latest payload received in `handleSubscription`.
  - Dataset schema sidebar sourced from the immutable dataset definition for context only.
- Use the existing interface `hostIp` and telegram endpoint hosts to automatically label each telegram row as **Outgoing**, **Incoming**, or **Loopback** so users immediately know which panes are interactive.

## Implementation sketch (next steps)
1. Extend `PdEndpointRuntime` to maintain per-telegram TX/RX buffers and expose accessors for the UI.
2. Introduce a small helper that evaluates `(interface.hostIp, telegram.sources, telegram.destinations)` to classify direction; reuse it in both runtime setup and PD view rendering.
3. Rework the PD view rows to render TX/RX panes based on that classification while keeping dataset details read-only.
4. Ensure payload writes are atomic (protect with the existing mutex) so TX edits cannot race with RX updates.
