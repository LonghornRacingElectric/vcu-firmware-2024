#!/usr/bin/env python3

import argparse
import csv
import struct
import sys
from pathlib import Path


FILE_MAGIC = b"VBLG"
FILE_VERSION = 1
HEADER_STRUCT = struct.Struct("<4sHHHHHH6BH8x")

CSV_HEADER = (
    "Time,Segment 1 Max,Segment 1 Min,Segment 1 Mean,Segment 2 Max,Segment 2 Min,Segment 2 Mean,"
    "Segment 3 Max,Segment 3 Min,Segment 3 Mean,Segment 4 Max,Segment 4 Min,Segment 4 Mean,"
    "Segment Unique Max,Segment Unique Min,Segment Unique Mean,Inverter Enabled,"
    "Inverter Torque Request,OCV Estimate,Power Limit,Power Limit Feedback P,Power Limit Feedback I,"
    "Power Limit Feedback D,Power Limit Feedback Torque,PRNDL State,Ready To Drive Buzzer,"
    "Brake Light,Enable Drag Reduction,Pump Output,Radiator Output,Battery Fans Output,"
    "Vehicle Displacement X,Vehicle Displacement Y,Vehicle Displacement Z,Vehicle Velocity X,"
    "Vehicle Velocity Y,Vehicle Velocity Z,Vehicle Acceleration X,Vehicle Acceleration Y,"
    "Vehicle Acceleration Z,HV Battery,LV Battery,Dash Speed,APPS Telemetry,BSE Telemetry,"
    "Steering Wheel Telemetry,APPS Fault,BSE Fault,STOMPP Fault,Steering Fault,Voltage,Current,"
    "State of Charge,Pack Voltage Mean,Pack Voltage Minimum,Pack Voltage Maximum,Pack Voltage Range,"
    "Pack Temp Mean,Pack Temp Minimum,Pack Temp Maximum,Pack Temp Range,IMD,AMS,Contactor Status,"
    "Cell Voltage Mean,Cell Voltage Max,Cell Voltage Min,Cell Temps Mean,Cell Temps Max,"
    "Cell Temps Min,Volumetric Flow Rate,Water Temp Inverter,Water Temp Motor,Water Temp Radiator,"
    "Radiator Fan RPM Percentage,LV Voltage,LV State of Charge,LV Current,Voltage Input into DC,"
    "Current Input into DC,RPM,Feedback Speed,Average Module Temp,Inverter Coolant Temp,"
    "Inverter Hot Spot Temp,Motor Temp,Motor Angle,Delta Resolver,Phase A Current,Phase B Current,"
    "Phase C Current,ID Feedback,IQ Feedback,BC Voltage,AB Voltage,Output Voltage,ID Command,"
    "IQ Command,Inverter Frequency,Actual Torque,Torque Command,Fault Vector,State Vector,"
    "BMS Limiting Regen Torque,Motor Temp Derate Limiting,Motor Hot Spot Limiting,BMS Active,"
    "BMS Limiting Motor Torque,Max Speed Limiting,Inverter Hot Spot Limiting,Low Speed Limiting,"
    "Coolant Derating Limiting,Stall Burst Limiting,APPS 1 Voltage,APPS 2 Voltage,BSE 1 Voltage,"
    "BSE 2 Voltage,Steer Voltage,Suspension 1 Voltage,Suspension 2 Voltage,Front Left Wheel Speed,"
    "Front Right Wheel Speed,Back Left Wheel Speed,Back Right Wheel Speed,"
    "Front Left Wheel Magnetic Field,VCU Acceleration X,VCU Acceleration Y,VCU Acceleration Z,"
    "HVC Acceleration X,HVC Acceleration Y,HVC Acceleration Z,PDU Acceleration X,"
    "PDU Acceleration Y,PDU Acceleration Z,Front Left Acceleration X,Front Left Acceleration Y,"
    "Front Left Acceleration Z,Front Right Acceleration X,Front Right Acceleration Y,"
    "Front Right Acceleration Z,Back Left Acceleration X,Back Left Acceleration Y,"
    "Back Left Acceleration Z,Back Right Acceleration X,Back Right Acceleration Y,"
    "Back Right Acceleration Z,VCU Gyro X,VCU Gyro Y,VCU Gyro Z,HVC Gyro X,HVC Gyro Y,HVC Gyro Z,"
    "PDU Gyro X,PDU Gyro Y,PDU Gyro Z,GPS Latitude,GPS Longitude,GPS Speed,GPS Heading,GPS Hour,"
    "GPS Minute,GPS Seconds,GPS Year,GPS Month,GPS Day,GPS Milliseconds"
)

CSV_COLUMNS = CSV_HEADER.split(",")

FIELD_SPECS = [
    ("uptime_ms", "I"),
    ("segment_1_max", "f"),
    ("segment_1_min", "f"),
    ("segment_1_mean", "f"),
    ("segment_2_max", "f"),
    ("segment_2_min", "f"),
    ("segment_2_mean", "f"),
    ("segment_3_max", "f"),
    ("segment_3_min", "f"),
    ("segment_3_mean", "f"),
    ("segment_4_max", "f"),
    ("segment_4_min", "f"),
    ("segment_4_mean", "f"),
    ("segment_unique_max", "f"),
    ("segment_unique_min", "f"),
    ("segment_unique_mean", "f"),
    ("enable_inverter", "B"),
    ("inverter_torque_request", "f"),
    ("ocv_estimate", "f"),
    ("power_limit", "f"),
    ("power_limit_feedback_p", "f"),
    ("power_limit_feedback_i", "f"),
    ("power_limit_feedback_d", "f"),
    ("power_limit_feedback_torque", "f"),
    ("prndl_state", "B"),
    ("ready_to_drive_buzzer", "B"),
    ("brake_light", "f"),
    ("enable_drag_reduction", "B"),
    ("pump_output", "f"),
    ("radiator_output", "f"),
    ("battery_fans_output", "f"),
    ("vehicle_displacement_x", "f"),
    ("vehicle_displacement_y", "f"),
    ("vehicle_displacement_z", "f"),
    ("vehicle_velocity_x", "f"),
    ("vehicle_velocity_y", "f"),
    ("vehicle_velocity_z", "f"),
    ("vehicle_acceleration_x", "f"),
    ("vehicle_acceleration_y", "f"),
    ("vehicle_acceleration_z", "f"),
    ("hv_battery", "f"),
    ("lv_battery", "f"),
    ("dash_speed", "f"),
    ("apps_telemetry", "f"),
    ("bse_telemetry", "f"),
    ("steering_wheel_telemetry", "f"),
    ("apps_fault", "B"),
    ("bse_fault", "B"),
    ("stompp_fault", "B"),
    ("steering_fault", "B"),
    ("voltage", "f"),
    ("current", "f"),
    ("state_of_charge", "f"),
    ("pack_voltage_mean", "f"),
    ("pack_voltage_minimum", "f"),
    ("pack_voltage_maximum", "f"),
    ("pack_voltage_range", "f"),
    ("pack_temp_mean", "f"),
    ("pack_temp_minimum", "f"),
    ("pack_temp_maximum", "f"),
    ("pack_temp_range", "f"),
    ("imd", "B"),
    ("ams", "B"),
    ("contactor_status", "B"),
    ("cell_voltage_mean", "f"),
    ("cell_voltage_max", "f"),
    ("cell_voltage_min", "f"),
    ("cell_temps_mean", "f"),
    ("cell_temps_max", "f"),
    ("cell_temps_min", "f"),
    ("volumetric_flow_rate", "f"),
    ("water_temp_inverter", "f"),
    ("water_temp_motor", "f"),
    ("water_temp_radiator", "f"),
    ("radiator_fan_rpm_percentage", "f"),
    ("lv_voltage", "f"),
    ("lv_state_of_charge", "f"),
    ("lv_current", "f"),
    ("voltage_input_into_dc", "f"),
    ("current_input_into_dc", "f"),
    ("rpm", "f"),
    ("feedback_speed", "f"),
    ("average_module_temp", "f"),
    ("inverter_coolant_temp", "f"),
    ("inverter_hot_spot_temp", "f"),
    ("motor_temp", "f"),
    ("motor_angle", "f"),
    ("delta_resolver", "f"),
    ("phase_a_current", "f"),
    ("phase_b_current", "f"),
    ("phase_c_current", "f"),
    ("id_feedback", "f"),
    ("iq_feedback", "f"),
    ("bc_voltage", "f"),
    ("ab_voltage", "f"),
    ("output_voltage", "f"),
    ("id_command", "f"),
    ("iq_command", "f"),
    ("inverter_frequency", "f"),
    ("actual_torque", "f"),
    ("torque_command", "f"),
    ("fault_vector", "Q"),
    ("state_vector", "Q"),
    ("bms_limiting_regen_torque", "B"),
    ("motor_temp_derate_limiting", "B"),
    ("motor_hot_spot_limiting", "B"),
    ("bms_active", "B"),
    ("bms_limiting_motor_torque", "B"),
    ("max_speed_limiting", "B"),
    ("inverter_hot_spot_limiting", "B"),
    ("low_speed_limiting", "B"),
    ("coolant_derating_limiting", "B"),
    ("stall_burst_limiting", "B"),
    ("apps_1_voltage", "f"),
    ("apps_2_voltage", "f"),
    ("bse_1_voltage", "f"),
    ("bse_2_voltage", "f"),
    ("steer_voltage", "f"),
    ("suspension_1_voltage", "f"),
    ("suspension_2_voltage", "f"),
    ("front_left_wheel_speed", "f"),
    ("front_right_wheel_speed", "f"),
    ("back_left_wheel_speed", "f"),
    ("back_right_wheel_speed", "f"),
    ("front_left_wheel_magnetic_field", "f"),
    ("vcu_acceleration_x", "f"),
    ("vcu_acceleration_y", "f"),
    ("vcu_acceleration_z", "f"),
    ("hvc_acceleration_x", "f"),
    ("hvc_acceleration_y", "f"),
    ("hvc_acceleration_z", "f"),
    ("pdu_acceleration_x", "f"),
    ("pdu_acceleration_y", "f"),
    ("pdu_acceleration_z", "f"),
    ("front_left_acceleration_x", "f"),
    ("front_left_acceleration_y", "f"),
    ("front_left_acceleration_z", "f"),
    ("front_right_acceleration_x", "f"),
    ("front_right_acceleration_y", "f"),
    ("front_right_acceleration_z", "f"),
    ("back_left_acceleration_x", "f"),
    ("back_left_acceleration_y", "f"),
    ("back_left_acceleration_z", "f"),
    ("back_right_acceleration_x", "f"),
    ("back_right_acceleration_y", "f"),
    ("back_right_acceleration_z", "f"),
    ("vcu_gyro_x", "f"),
    ("vcu_gyro_y", "f"),
    ("vcu_gyro_z", "f"),
    ("hvc_gyro_x", "f"),
    ("hvc_gyro_y", "f"),
    ("hvc_gyro_z", "f"),
    ("pdu_gyro_x", "f"),
    ("pdu_gyro_y", "f"),
    ("pdu_gyro_z", "f"),
    ("gps_latitude", "d"),
    ("gps_longitude", "d"),
    ("gps_speed", "f"),
    ("gps_heading", "f"),
    ("gps_hour", "B"),
    ("gps_minute", "B"),
    ("gps_seconds", "B"),
    ("gps_year", "B"),
    ("gps_month", "B"),
    ("gps_day", "B"),
    ("gps_milliseconds", "H"),
]

RECORD_STRUCT = struct.Struct("<" + "".join(fmt for _, fmt in FIELD_SPECS))


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Decode a .vbl telemetry file into CSV.")
    parser.add_argument("input", type=Path, help="Path to the .vbl input file")
    parser.add_argument(
        "-o",
        "--output",
        type=Path,
        help="Path to the CSV output file. Defaults to the input path with a .csv suffix.",
    )
    return parser.parse_args()


def decode_header(raw_header: bytes) -> dict:
    (
        magic,
        version,
        header_size,
        record_size,
        flags,
        sample_period_ms,
        sync_interval_ms,
        start_year,
        start_month,
        start_day,
        start_hour,
        start_minute,
        start_second,
        start_millis,
    ) = HEADER_STRUCT.unpack(raw_header)

    return {
        "magic": magic,
        "version": version,
        "header_size": header_size,
        "record_size": record_size,
        "flags": flags,
        "sample_period_ms": sample_period_ms,
        "sync_interval_ms": sync_interval_ms,
        "start_year": start_year,
        "start_month": start_month,
        "start_day": start_day,
        "start_hour": start_hour,
        "start_minute": start_minute,
        "start_second": start_second,
        "start_millis": start_millis,
    }


def format_row(record_values: tuple) -> list:
    row = [record_values[0] / 1000.0]
    row.extend(record_values[1:])
    return row


def main() -> int:
    args = parse_args()

    output_path = args.output or args.input.with_suffix(".csv")

    if len(CSV_COLUMNS) != len(FIELD_SPECS):
      print("Decoder configuration mismatch: CSV header count does not match field count.", file=sys.stderr)
      return 1

    if HEADER_STRUCT.size != 32:
        print("Decoder configuration mismatch: header size must remain 32 bytes.", file=sys.stderr)
        return 1

    if not args.input.exists():
        print(f"Input file does not exist: {args.input}", file=sys.stderr)
        return 1

    with args.input.open("rb") as input_file:
        raw_header = input_file.read(HEADER_STRUCT.size)
        if len(raw_header) != HEADER_STRUCT.size:
            print("Input file is too small to contain a telemetry header.", file=sys.stderr)
            return 1

        header = decode_header(raw_header)

        if header["magic"] != FILE_MAGIC:
            print(f"Unexpected file magic: {header['magic']!r}", file=sys.stderr)
            return 1

        if header["version"] != FILE_VERSION:
            print(f"Unsupported telemetry version: {header['version']}", file=sys.stderr)
            return 1

        if header["header_size"] != HEADER_STRUCT.size:
            print(f"Unexpected header size: {header['header_size']}", file=sys.stderr)
            return 1

        if header["record_size"] != RECORD_STRUCT.size:
            print(
                f"Unexpected record size: file says {header['record_size']}, decoder expects {RECORD_STRUCT.size}",
                file=sys.stderr,
            )
            return 1

        raw_records = input_file.read()

    complete_record_bytes = len(raw_records) - (len(raw_records) % RECORD_STRUCT.size)
    if complete_record_bytes != len(raw_records):
        print(
            "Warning: ignoring trailing partial telemetry record at end of file.",
            file=sys.stderr,
        )

    with output_path.open("w", newline="") as output_file:
        writer = csv.writer(output_file)
        writer.writerow(CSV_COLUMNS)

        for offset in range(0, complete_record_bytes, RECORD_STRUCT.size):
            record_values = RECORD_STRUCT.unpack_from(raw_records, offset)
            writer.writerow(format_row(record_values))

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
