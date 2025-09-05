import serial
import csv

# Ajuste a porta conforme seu PC
# Windows: "COM3", "COM4", etc.
# Linux/Mac: "/dev/ttyUSB0" ou "/dev/ttyACM0"
SERIAL_PORT = "COM3"
BAUD_RATE = 115200
MAX_LINES = 100  # número de linhas que queremos salvar

# Conecta ao Serial
ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)

# Nome do CSV que será salvo
CSV_FILENAME = "dataset_features.csv"

lines_saved = 0
csv_lines = []

print("Aguardando dados do Serial...")

# Lê o cabeçalho primeiro
while True:
    header_line = ser.readline().decode().strip()
    if header_line:
        print("Cabeçalho:", header_line)
        csv_lines.append(header_line.split(','))  # salva cabeçalho
        break

# Lê linhas do Serial
while lines_saved < MAX_LINES:
    line = ser.readline().decode().strip()
    if line:
        values = line.split(',')
        csv_lines.append(values)
        lines_saved += 1
        print(f"Linha {lines_saved}: {line}")

# Salva CSV
with open(CSV_FILENAME, 'w', newline='') as f:
    writer = csv.writer(f)
    writer.writerows(csv_lines)

print(f"Arquivo '{CSV_FILENAME}' salvo com sucesso com {MAX_LINES} linhas!")
ser.close()
