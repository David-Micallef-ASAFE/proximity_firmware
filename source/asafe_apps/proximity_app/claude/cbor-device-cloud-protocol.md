# Device ↔ Cloud CBOR Protocol

| | |
|---|---|
| **Status** | Draft |
| **Version** | 0.21 |
| **Last updated** | 2026-10-26 |
| **Owner** | David Micallef |

---

## 1. Scope

This document defines the application-layer message format exchanged between a device and the cloud system. Messages are encoded as [CBOR (RFC 8949)](https://www.rfc-editor.org/rfc/rfc8949).

The device communicates over **Wirepas**. §9 defines which Wirepas endpoints carry these messages.

**Out of scope:** everything else about how messages are moved (Wirepas routing, gateway, MQTT/backend, etc.), transport-level security, and retries/acknowledgement at the transport layer.

## 2. Constraints

| Constraint | Value |
|---|---|
| Maximum message size (either direction) | **100 bytes** (the complete encoded CBOR item) |
| Encoding | One CBOR **map** per message, with **integer keys** |
| Byte order | Big-endian (as defined by CBOR) |

A sender **must not** emit a message larger than 100 bytes. If a set of values doesn't fit, the sender splits it across several messages. A receiver **must** reject any message larger than 100 bytes with status `MSG_TOO_LARGE`.

## 3. Encoding Rules

1. Each message is a single, definite-length CBOR map (major type 5). Indefinite-length items are not allowed.
2. All map keys are CBOR integers.
   - **Negative keys (−1 … −24)** are reserved for protocol control fields (see §4).
   - **Positive keys (1 … n)** are data keys defined in the Key Database (§6).
   - Key `0` is reserved and must not be used.
3. Senders should use **deterministic encoding** (RFC 8949 §4.2): shortest-form integers and lengths, and keys ordered by their encoded bytes.
4. Keys in the range −24 … 23 encode in **1 byte**, so the most frequently used keys should stay in that range. Keys 24 … 255 take 2 bytes.
5. Fixed-length binary values (encryption keys, etc.) are encoded as a CBOR **byte string** (major type 2), *not* as a CBOR array of integers. A 16-byte key costs 17 bytes as a byte string, and up to 33 bytes as an array.
6. A receiver must ignore unknown control keys in the range −24 … −1 that it doesn't support. For unknown positive data keys it must return `UNKNOWN_KEY` (see §5).

### 3.1 CBOR Type Reference

| Type name used in this doc | CBOR major type | Encoded size |
|---|---|---|
| Unsigned Integer | 0 | 1–9 bytes (value ≤ 23 → 1 byte; ≤ 0xFF → 2; ≤ 0xFFFF → 3; ≤ 0xFFFFFFFF → 5) |
| Integer | 0 or 1 | as above |
| Boolean | 7 (simple values 20/21) | 1 byte (`F4` = false, `F5` = true) |
| Byte String (N) | 2 | header (1 byte if N ≤ 23, 2 bytes if N ≤ 255) + N |
| Array | 4 | header + elements |
| Map | 5 | header + key/value pairs |
| Null | 7 (simple value 22) | 1 byte (`F6`) |

## 4. Control Fields (Reserved Negative Keys)

| Key | Name | CBOR Type | Required | Description |
|---|---|---|---|---|
| −1 | Operation | Unsigned Integer | Always | Message operation (see §4.1) |
| −2 | Sequence | Unsigned Integer | Always | Transaction ID set by the requester and echoed in the response. Wraps at 0xFFFF. |
| −3 | Status | Unsigned Integer | In `RESPONSE` | Overall result (see §5) |
| −4 | Key List | Array of Unsigned Integer | In `READ` | Data keys being requested |
| −5 | Key Errors | Map {key → status} | Optional in `RESPONSE` | Status per key, included when one or more keys failed |
| −6 | Protocol Version | Unsigned Integer | In `NOTIFY`; optional in other messages | Protocol version the sender is using (see §4.3) |
| −7 … −24 | *Reserved* | — | — | Reserved for future control fields |

### 4.1 Operations

| Value | Operation | Direction | Description |
|---|---|---|---|
| 1 | `READ` | Cloud → Device | Request the values of the keys in `−4` |
| 2 | `WRITE` | Cloud → Device | Set the values of the data keys in the map |
| 3 | `RESPONSE` | Device → Cloud | Reply to a `READ` or `WRITE`. Carries the `−3` status, plus the values that were read. |
| 4 | `NOTIFY` | Device → Cloud | Unsolicited report of one or more data keys (e.g. on boot or change) |

### 4.2 Semantics

- **READ:** The device returns every requested key it can read. Keys it can't read (write-only or unknown) are left out of the value set and listed in `−5`.
- **WRITE:** A write is **all-or-nothing** per message. If any key fails validation, nothing is applied, the status is non-zero, and the failing keys are listed in `−5`.
- **Write-only keys** (e.g. network/provisioning keys) are never returned by the device. Reading one returns `ACCESS_DENIED` for that key.
- **Read-only keys** included in a `WRITE` return `ACCESS_DENIED`.
- **Applying Wirepas settings (keys 1–6):** after a successful `WRITE` to any of keys 1–6, the device:
  1. Stores the new values and sends the `RESPONSE` with status `OK`. Here `OK` means "accepted and stored", not yet in effect.
  2. Waits **1 minute**.
  3. Resets automatically and comes back up with the new settings.

  From then on, the cloud must reach the device using the new settings (e.g. a new network address or channel).
  Another successful write to keys 1–6 during the 1-minute wait restarts the timer, so several messages can be applied with a single reset.
- **NOTIFY** does not require a response. The device picks its own sequence numbers for notifications.

### 4.3 Protocol Versioning

- The current protocol version is **1**. This is separate from the document version in the header table.
- The version number goes up only for **breaking** changes, such as a change to the meaning or type of an existing key, control field or status code. Adding new keys, operations or status codes is not breaking.
- The device **must** include `−6` in every `NOTIFY`, so the cloud learns the version the device supports.
- Other messages may include `−6`. When a request leaves it out, the device assumes version 1.
- If a request carries a version the device doesn't support, the device makes no changes and replies with status `UNSUPPORTED_VERSION`. That `RESPONSE` includes `−6` set to the version the device does support.
- Including `−6` adds 2 bytes to a message.

## 5. Status Codes

| Value | Name | Meaning |
|---|---|---|
| 0 | `OK` | Success |
| 1 | `UNKNOWN_KEY` | Data key not defined or not supported by this device |
| 2 | `ACCESS_DENIED` | Read of a write-only key, or write of a read-only key |
| 3 | `INVALID_TYPE` | Value has the wrong CBOR type or length |
| 4 | `OUT_OF_RANGE` | Value is outside the permitted range |
| 5 | `MSG_TOO_LARGE` | Message larger than 100 bytes |
| 6 | `MALFORMED` | Not valid CBOR, not a map, or a required control field is missing |
| 7 | `UNSUPPORTED_OP` | Unknown operation in `−1` |
| 8 | `BUSY` | Device can't process the request right now. Retry later. |
| 9 | `PARTIAL` | Some keys succeeded and some failed (READ only). See `−5`. |
| 10 | `UNSUPPORTED_VERSION` | Protocol version in `−6` isn't supported. The reply carries the supported version in `−6`. |
| 11 – 255 | *Reserved* | |

## 6. Key Database

This is the master list of data keys, and it will be updated as the protocol grows.
**Access:** `R` = Read-only, `W` = Write-only, `R/W` = Read/Write.

| Key | Name | Context | CBOR Type | Access | Notes |
|---|---|---|---|---|---|
| 1 | Network Address | Wirepas | Unsigned Integer | R/W | Wirepas network address (24-bit) |
| 2 | Network Channel | Wirepas | Unsigned Integer | R/W | Range **0 – 39** |
| 3 | Node Role | Wirepas | Unsigned Integer | R/W | See §6.1 |
| 4 | Network Keys In Use | Wirepas | Boolean | R/W | `true` = network encryption/authentication keys are set and active |
| 5 | Network Key – Cipher Key | Wirepas | Byte String (16) | W | 128-bit cipher key. Must be exactly 16 bytes. |
| 6 | Network Key – Auth Key | Wirepas | Byte String (16) | W | 128-bit authentication key. Must be exactly 16 bytes. |
| 7 | Provisioning UID | Provisioning | Unsigned Integer | R/W | Unique ID used during provisioning. Range **0 – 0xFFFFFFFF** (32-bit). |
| 8 | Provisioning Key | Provisioning | Byte String (32) | W | 256-bit provisioning key. Must be exactly 32 bytes. |
| 9 | Heartbeat | Proximity V1 | Unsigned Integer | R/W | Heartbeat interval in **seconds**. Range **60 – 86400** (1 minute to 1 day). |
| 10 | Anchor Slot | Proximity V1 | Unsigned Integer | R/W | Slot assigned to the anchor. Range **0 – 22**. |
| 11 | Tag – Stationary Update Period | Proximity V1 | Unsigned Integer | R/W | Update period while the tag is stationary, in **seconds**. Range **1 – 86400** (1 second to 1 day). |
| 12 | Tag – Motion Update Period | Proximity V1 | Unsigned Integer | R/W | Update period while the tag is in motion, in **seconds**. Range **1 – 86400** (1 second to 1 day). |
| 13 | Reset Device Settings | Wirepas | Null | W | **Command** ("Unprovisioning"). The value must be `null`. See §6.2. |
| 14 | Retrieve | Device | Unsigned Integer | W | **Command**. Starts the device retrieval procedure. The value is how long the procedure runs, in **seconds**. Range **10 – 900** (10 seconds to 15 minutes). See §6.2. |
| 15 | Mode | Device | Unsigned Integer | R/W | Device operating mode. Valid values **0 – 1**. See §6.3. |
| 16 | Ping | Device | Null | W | **Command**. Confirms that the device is listening. The value must be `null`. See §6.2. |
| 17 | UWB Operation Enabled | Device | Boolean | R/W | `true` = UWB operation is enabled, `false` = disabled. |
| 18 | Device Management Enabled | Device | Boolean | R/W | `true` = device management is enabled, `false` = disabled. |

### 6.1 Node Role Values (Key 3)

The value is a base role combined (bitwise OR) with optional flags, following the Wirepas stack role encoding. *Check these values against the Wirepas SDK version in use.*

| Bits | Value | Meaning |
|---|---|---|
| Base role | `0x01` | Sink |
| | `0x02` | Headnode (router) |
| | `0x03` | Subnode (non-router) |
| Flag | `0x10` | Low-latency |
| Flag | `0x80` | Autorole |

Example: `0x82` = Headnode + Autorole.

### 6.2 Command Keys

A **command key** triggers an action on the device instead of setting a value. Command keys:

- are sent only in a `WRITE`;
- carry the value CBOR `null` (`F6`, 1 byte) when they take no parameter, because a CBOR map entry always needs a value. Any other value is rejected with `INVALID_TYPE`. A command that does take a parameter (e.g. key 14) carries it as its value, with the type given in the Key Database;
- are write-only. Reading one returns `ACCESS_DENIED`.

#### Key 13 — Reset Device Settings ("Unprovisioning")

When the device receives `{13: null}`, it:

1. Replies with a `RESPONSE` carrying status `OK`.
2. Waits **1 minute**, the same delay as for Wirepas setting changes in §4.2.
3. Resets all its Wirepas settings (keys 1–6) to **NULL**, i.e. unset, and restarts.

Unprovisioning clears **only** the Wirepas settings (keys 1–6). The provisioning data (keys 7 and 8, Provisioning UID and Provisioning Key) is **kept**, so the device can be provisioned again later.

Once unprovisioned, the device is no longer on the Wirepas network and can't be reached through this protocol until it is provisioned again.

Key 13 must be the only data key in its `WRITE`. If other data keys come with it, the whole write is rejected with `INVALID_TYPE` and nothing is applied.

#### Key 14 — Retrieve

When the device receives `{14: <period>}`, it **immediately** starts a procedure that helps someone physically find and retrieve it, then replies with a `RESPONSE` carrying status `OK`. Here `OK` means the procedure has started.

- `<period>` is an Unsigned Integer in **seconds**. The procedure **ends automatically** on the device once this period has passed. No stop command is needed.
- Valid range: **10 – 900** seconds (10 seconds to 15 minutes). A value outside the range is rejected with `OUT_OF_RANGE`. The only exception is the stop value `0` (below).
- If Retrieve arrives while the procedure is already running, the procedure restarts with the new period.
- A period of `0` stops a running procedure straight away.
- How the procedure works is **out of scope** for this document. Each device uses whatever hardware it has on board, such as flashing an LED or sounding a buzzer.
- A device with no suitable hardware replies with `UNKNOWN_KEY` for key 14.

#### Key 16 — Ping

Ping checks that the device is reachable and listening on the network.

- When the device receives `{16: null}`, it replies **immediately** with a `RESPONSE` carrying status `OK`. That reply is the acknowledgement.
- Ping **changes nothing** on the device: no settings, no configuration and no mode.
- If no `RESPONSE` comes back, the cloud can treat the device as unreachable.

### 6.3 Mode Values (Key 15)

| Value | Mode |
|---|---|
| 0 | Inventory |
| 1 | Run |
| 2 – 255 | *Reserved for future modes* |

- The only accepted values for now are `0` and `1`. Any other value is rejected with `OUT_OF_RANGE`.
- More modes may be added later. Adding a mode is not a breaking change (§4.3), so it doesn't change the protocol version.

## 7. Examples

All examples were verified with a CBOR encoder. Diagnostic notation is shown first, followed by the hex encoding.

### 7.1 Read Wirepas network configuration

**Request (Cloud → Device) — 10 bytes**
```
{-1: 1, -2: 7, -4: [1, 2, 3]}
A3 20 01 21 07 23 83 01 02 03
```

**Response (Device → Cloud) — 18 bytes**
```
{-1: 3, -2: 7, -3: 0, 1: 0x123456, 2: 18, 3: 0x82}
A6 20 03 21 07 22 00 01 1A 00 12 34 56 02 12 03 18 82
```

### 7.2 Write network keys

**Request (Cloud → Device) — 43 bytes**
```
{-1: 2, -2: 8, 4: true,
  5: h'000102030405060708090A0B0C0D0E0F',
  6: h'101112131415161718191A1B1C1D1E1F'}
A5 20 02 21 08 04 F5
05 50 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F
06 50 10 11 12 13 14 15 16 17 18 19 1A 1B 1C 1D 1E 1F
```

**Response — 7 bytes**
```
{-1: 3, -2: 8, -3: 0}
A3 20 03 21 08 22 00
```

### 7.3 Read including a write-only key (partial failure)

**Request — 9 bytes**
```
{-1: 1, -2: 9, -4: [5, 7]}
A3 20 01 21 09 23 82 05 07
```

**Response — 17 bytes.** Key 7 is returned; key 5 is reported as `ACCESS_DENIED`.
```
{-1: 3, -2: 9, -3: 9, 7: 305419896, -5: {5: 2}}
A5 20 03 21 09 22 09 07 1A 12 34 56 78 24 A1 05 02
```

### 7.4 Write provisioning key

**Request — 40 bytes**
```
{-1: 2, -2: 10, 8: h'000102…1F'}
A3 20 02 21 0A 08 58 20 <32 bytes>
```

### 7.5 Reset Device Settings (Unprovisioning)

**Request — 7 bytes**
```
{-1: 2, -2: 11, 13: null}
A3 20 02 21 0B 0D F6
```

**Response — 7 bytes.** The device unprovisions 1 minute later.
```
{-1: 3, -2: 11, -3: 0}
A3 20 03 21 0B 22 00
```

### 7.6 Retrieve

**Request — 8 bytes.** Run the retrieval procedure for 60 seconds.
```
{-1: 2, -2: 12, 14: 60}
A3 20 02 21 0C 0E 18 3C
```

**Response — 7 bytes.** The retrieval procedure has started and ends automatically after 60 seconds.
```
{-1: 3, -2: 12, -3: 0}
A3 20 03 21 0C 22 00
```

### 7.7 Ping

**Request — 7 bytes**
```
{-1: 2, -2: 13, 16: null}
A3 20 02 21 0D 10 F6
```

**Response — 7 bytes.** The device is listening.
```
{-1: 3, -2: 13, -3: 0}
A3 20 03 21 0D 22 00
```

### 7.8 Write and read UWB Operation / Device Management

**Request — 9 bytes.** Enable UWB operation and disable device management.
```
{-1: 2, -2: 14, 17: true, 18: false}
A4 20 02 21 0E 11 F5 12 F4
```

**Response — 7 bytes**
```
{-1: 3, -2: 14, -3: 0}
A3 20 03 21 0E 22 00
```

**Read request — 9 bytes**
```
{-1: 1, -2: 15, -4: [17, 18]}
A3 20 01 21 0F 23 82 11 12
```

**Read response — 11 bytes**
```
{-1: 3, -2: 15, -3: 0, 17: true, 18: false}
A5 20 03 21 0F 22 00 11 F5 12 F4
```

## 8. Size Budget

The fixed overhead of a request (map header + Operation + Sequence) is **5–6 bytes**, which leaves about **94 bytes** for data.

| Item | Bytes |
|---|---|
| Map header (≤ 23 entries) | 1 |
| Operation (`−1`) | 2 |
| Sequence (`−2`), value ≤ 23 / ≤ 255 / ≤ 65535 | 2 / 3 / 4 |
| Status (`−3`) | 2 |
| Protocol Version (`−6`) | 2 |
| Boolean data key | 2 |
| Unsigned Integer data key (24-bit value) | 6 |
| Byte String (16) data key | 18 |
| Byte String (32) data key | 35 |

Worst case for the current table: writing keys 5, 6 and 8 together comes to 1 + 2 + 4 + 18 + 18 + 35 = **78 bytes**, which fits.

## 9. Wirepas Transport Binding

The rules below cover how protocol messages are carried in Wirepas data packets.

### 9.1 Endpoints

| Direction | Source Endpoint | Destination Endpoint |
|---|---|---|
| Cloud → Device (`READ`, `WRITE`) | **Any** (0–255) | **10** |
| Device → Cloud (`RESPONSE`, `NOTIFY`) | Device's configured endpoint | **2** |

- The device **must** accept and process packets on **Destination Endpoint 10**, whatever the Source Endpoint.
- All device-to-cloud messages are sent to **Destination Endpoint 2**. The Source Endpoint is whatever the device is configured to use.
- Packets on any other destination endpoint are not part of this protocol. The device ignores them, or passes them to another application if one is registered on that endpoint.

### 9.2 Payload

- Each Wirepas packet payload holds **exactly one** CBOR message (§3). Nothing comes before or after it: no extra header and no padding.
- The 100-byte limit (§2) applies to the Wirepas payload.
- If a packet on endpoint 10 doesn't decode as a valid message, the device replies with a `RESPONSE` carrying status `MALFORMED`. If it can't recover the sequence number, it sets `−2` to 0.

### 9.3 Addressing

- The device sends all device-to-cloud messages, both `RESPONSE` and `NOTIFY`, to **any sink**.

## 10. Security Considerations

Keys 5, 6 and 8 carry secret key material in clear CBOR. This protocol assumes the transport provides confidentiality and integrity. If it can't guarantee that, an application-layer protection scheme (e.g. COSE_Encrypt0) must be added. COSE adds overhead that has to fit within the 100-byte budget.

## 11. Open Questions

- [ ] Mode (key 15): does a mode change take effect immediately, or after a delay or reset (as for Wirepas settings)? What mode does a new or unprovisioned device start in?

## 12. Change Log

| Version | Date | Change |
|---|---|---|
| 0.1 | 2026-10-22 | Initial draft. Keys 1–10 defined (10 is a placeholder). |
| 0.2 | 2026-10-22 | Added Heartbeat (9). Anchor Slot moved to 10. Added Tag Stationary Update Period (11) and Tag Stationary Motion Update Period (12). |
| 0.3 | 2026-10-22 | Added Wirepas Transport Binding (§9): device accepts messages on Destination Endpoint 10 from any Source Endpoint. |
| 0.4 | 2026-10-22 | Key 12 renamed to Tag – Motion Update Period (name typo fixed). |
| 0.5 | 2026-10-22 | Wirepas settings (keys 1–6) are applied by an automatic reset 1 minute after a successful write (§4.2). |
| 0.6 | 2026-10-22 | Added Protocol Version control field `−6` (§4.3) and status `UNSUPPORTED_VERSION` (10). |
| 0.7 | 2026-10-22 | Valid ranges defined: Network Channel 0–39, Provisioning UID 0–0xFFFFFFFF, Anchor Slot 0–22. |
| 0.8 | 2026-10-22 | Device → Cloud messages are sent to Destination Endpoint 2, from the device's configured Source Endpoint (§9.1). |
| 0.9 | 2026-10-22 | Units (seconds) and ranges defined for Heartbeat (60–86400) and Tag update periods 11, 12 (1–86400). |
| 0.10 | 2026-10-22 | Confirmed: another write to keys 1–6 during the 1-minute wait restarts the timer (§4.2). |
| 0.11 | 2026-10-22 | Addressing finalised: `RESPONSE` and `NOTIFY` are both sent to any sink (§9.3). |
| 0.12 | 2026-10-22 | Added key 13 Reset Device Settings ("Unprovisioning") command, command-key rules (§6.2), and example 7.5. |
| 0.13 | 2026-10-22 | Added key 14 Retrieve command (§6.2) and example 7.6. |
| 0.14 | 2026-10-22 | Clarified: Unprovisioning (key 13) clears only Wirepas settings (keys 1–6); provisioning data (keys 7, 8) is kept. |
| 0.15 | 2026-10-22 | Retrieve (key 14) now takes a period in seconds (Unsigned Integer), after which the procedure ends automatically. Example 7.6 updated. |
| 0.16 | 2026-10-22 | Confirmed: key 13 must be sent on its own in a `WRITE`, and it takes effect after a 1-minute delay. Key 13's context confirmed as Wirepas. |
| 0.17 | 2026-10-22 | Retrieve (key 14) period range set to 10–900 seconds. |
| 0.18 | 2026-10-22 | Confirmed: a new Retrieve restarts a running procedure, and a period of `0` stops it. |
| 0.19 | 2026-10-22 | Added key 15 Mode (Device, R/W): 0 = Inventory, 1 = Run (§6.3). |
| 0.20 | 2026-10-22 | Added key 16 Ping command (§6.2) and example 7.7. |
| 0.21 | 2026-10-26 | Added keys 17 UWB Operation Enabled and 18 Device Management Enabled (Device, Boolean, R/W), and example 7.8. |
