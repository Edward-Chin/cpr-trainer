#!/usr/bin/env python3
"""Save DistanceCapture or StrokeTest serial CSV without extra dependencies."""
import csv
from datetime import datetime
from pathlib import Path
import re
import sys

RAW_HEADER = 'timestamp_ms,echo_duration_us,distance_mm,valid'
STROKE_HEADER = 'timestamp_ms,gap_mm,in_range,event,down_mm,up_mm,bpm,hz,count'
NUMBER = r'(?:\d+(?:\.\d+)?)?'
FORMATS = {
    RAW_HEADER: re.compile(r'\d+,\d+,' + NUMBER + r',[01]'),
    STROKE_HEADER: re.compile(r'\d+,' + NUMBER + r',[01],(?:down|up)?,' +
                              ','.join([NUMBER] * 4) + r',\d+'),
}


def parse_row(line):
    for header, pattern in FORMATS.items():
        if pattern.fullmatch(line):
            fields = line.split(',')
            if header == RAW_HEADER:
                if fields[3] == '1' and (not fields[2] or int(fields[1]) == 0):
                    return None
            elif fields[2] == '1' and not fields[1]:
                return None
            return header, fields
    return None


def main():
    log_dir = Path(__file__).resolve().parents[1] / 'logs'
    log_dir.mkdir(exist_ok=True)
    path = log_dir / ('distance-' + datetime.now().strftime('%Y%m%d-%H%M%S-%f') + '.csv')
    count = 0
    selected_header = None
    failed = False
    print(f'Saving to {path}\nAccepting DistanceCapture or StrokeTest. Ctrl+C stops.',
          file=sys.stderr, flush=True)
    with path.open('x', newline='', buffering=1) as output:
        writer = csv.writer(output)
        try:
            for line in sys.stdin:
                line = line.strip()
                if not line:
                    continue
                if line.startswith('#'):
                    print(line, file=sys.stderr)
                    continue
                if line in FORMATS:
                    header, fields = line, None
                else:
                    parsed = parse_row(line)
                    if parsed is None:
                        print(f'Ignored non-CSV or malformed serial output: {line}', file=sys.stderr)
                        continue
                    header, fields = parsed
                # Infer from a complete row if the initial USB header was missed.
                if selected_header is None:
                    selected_header = header
                    writer.writerow(header.split(','))
                    print(f'Detected columns: {header}', file=sys.stderr, flush=True)
                elif selected_header != header:
                    print('Sketch format changed. Stop and restart capture for a new file.', file=sys.stderr)
                    failed = True
                    break
                if fields is None:
                    continue
                writer.writerow(fields)
                count += 1
                if count % 50 == 0:
                    print(f'{count} samples saved; latest: {line}', file=sys.stderr, flush=True)
        except KeyboardInterrupt:
            pass
    print(f'Saved {count} samples to {path}', file=sys.stderr)
    if not count:
        print('No samples received. Check the uploaded sketch and port.', file=sys.stderr)
    return 1 if failed or not count else 0


if __name__ == '__main__':
    sys.exit(main())
