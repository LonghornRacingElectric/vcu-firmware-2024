# VBL Telemetry Format v1

This firmware now writes telemetry as `.vbl` binary files instead of CSV.

The authoritative on-device layout lives in:
- [nvm_telemetry_format.h](/Users/acloran/Documents/GitHub/vcu-firmware-2024/Core/Inc/nvm_telemetry_format.h)

The reference decoder lives in:
- [decode_telemetry_vbl.py](/Users/acloran/Documents/GitHub/vcu-firmware-2024/Scripts/decode_telemetry_vbl.py)

If you want another Codex session to add decoding support to your existing CSV-to-MoTeC tool, pass it both files above plus this note.

## File Layout

Each file is:
1. A 32-byte file header
2. Zero or more fixed-size telemetry records

There is no footer. Record count is derived from:
- `(file_size - header_size) / record_size`

## Header Layout

The header is little-endian and matches this packed C struct:

```c
struct TelemetryFileHeaderV1 {
  char magic[4];          // "VBLG"
  uint16_t version;       // 1
  uint16_t headerSize;    // 32
  uint16_t recordSize;    // sizeof(TelemetryRecordV1)
  uint16_t flags;         // bit0=little-endian, bit1=IEEE754 float/double
  uint16_t samplePeriodMs;// 0 means "sample every nvm_periodic call"
  uint16_t syncIntervalMs;// periodic f_sync cadence used by firmware
  uint8_t startYear;      // GPS year, 0-99
  uint8_t startMonth;
  uint8_t startDay;
  uint8_t startHour;
  uint8_t startMinute;
  uint8_t startSecond;
  uint16_t startMillis;
  uint8_t reserved[8];
};
```

The Python decoder uses this exact `struct` format string for the header:

```python
"<4sHHHHHH6BH8x"
```

## Record Layout

Each record matches the packed `TelemetryRecordV1` struct in [nvm_telemetry_format.h](/Users/acloran/Documents/GitHub/vcu-firmware-2024/Core/Inc/nvm_telemetry_format.h).

Important decoding notes:
- The file is little-endian.
- `float` fields are 32-bit IEEE754.
- `double` fields are 64-bit IEEE754.
- `uint64_t` fields are unsigned 64-bit integers.
- Boolean-like values are stored as `uint8_t` and should be interpreted as `0` or `1`.
- The first field is `uptimeMs` as a `uint32_t`.
  The reference decoder converts it back to the legacy CSV `Time` column by dividing by `1000.0`.
- `feedbackSpeed` currently contains the same value as `rpm`, because that is what the old CSV path emitted.

## CSV Compatibility

The reference decoder writes the same CSV column names and order that the old logger used. That means your existing CSV-to-MoTeC tool can stay focused on CSV semantics if you prefer:

1. Decode `.vbl` to CSV with [decode_telemetry_vbl.py](/Users/acloran/Documents/GitHub/vcu-firmware-2024/Scripts/decode_telemetry_vbl.py)
2. Feed the resulting CSV into your existing converter

If you want to decode `.vbl` directly inside your converter, mirror:
- the 32-byte header format
- the `TelemetryRecordV1` field order
- the `FIELD_SPECS` array in [decode_telemetry_vbl.py](/Users/acloran/Documents/GitHub/vcu-firmware-2024/Scripts/decode_telemetry_vbl.py)

## Example

```bash
python3 Scripts/decode_telemetry_vbl.py Log__2026_04_09__12_34_56.vbl
```

That writes:

```text
Log__2026_04_09__12_34_56.csv
```
