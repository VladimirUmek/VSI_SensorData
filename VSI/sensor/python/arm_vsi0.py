# Copyright (c) 2021 Arm Limited. All rights reserved.

# Virtual Streaming Interface instance 0 Python script: Sensor Interface

##@addtogroup arm_vsi0_py_sensor
#  @{
#
##@package arm_vsi0_sensor
#Documentation for VSI Sensor Data module.
#
#More details.

import math
import logging
import os

## Set verbosity level
verbosity = logging.DEBUG
#verbosity = logging.ERROR

# [debugging] Verbosity settings
level = { 10: "DEBUG",  20: "INFO",  30: "WARNING",  40: "ERROR" }
logging.basicConfig(format='Py: VSI0: [%(levelname)s]\t%(message)s', level = verbosity)
logging.info("Verbosity level is set to " + level[verbosity])

# Input File Name
FILE_NAME_SENSOR = "..\\sensor_samples0.csv"

CSV_Col = {
    'Timestamp' : -1,
    'Temp'      : -1,
    'Hum'       : -1,
    'Press'     : -1,
    'Light'     : -1,
    'AccX'      : -1,
    'AccY'      : -1,
    'AccZ'      : -1,
    'GyroX'     : -1,
    'GyroY'     : -1,
    'GyroZ'     : -1,
    'MagX'      : -1,
    'MagY'      : -1,
    'MagZ'      : -1
}

# IRQ registers
IRQ_Status = 0

# Timer registers
Timer_Control  = 0
Timer_Interval = 0

# Timer callback event count
Timer_Event = 0

# Timer Control register definitions
Timer_Control_Run_Msk      = 1<<0
Timer_Control_Periodic_Msk = 1<<1
Timer_Control_Trig_IRQ_Msk = 1<<2
Timer_Control_Trig_DMA_Msk = 1<<3

# DMA registers
DMA_Control = 0

# DMA Control register definitions
DMA_Control_Enable_Msk    = 1<<0
DMA_Control_Direction_Msk = 1<<1
DMA_Control_Direction_P2M = 0<<1
DMA_Control_Direction_M2P = 1<<1

# Define number of sensors
SENSOR_COUNT = 6

# Sensor ID definitions (constants)
SID_TEMP  = 0
SID_HUM   = 1
SID_PRESS = 2
SID_ACC   = 3
SID_GYRO  = 4
SID_MAG   = 5

# USER REGISTER MAPPING
# =====================
IDX_STATUS        = 0
IDX_INTERVAL      = 1
IDX_SELECT        = 2
IDX_ENABLE        = 3
IDX_SCALE         = 4
IDX_ODR           = 5
IDX_FIFO_CNT      = 6
IDX_FIFO          = 7

# Status Register
# ===============
STATUS = 0

# Bit definitions
BIT_STATUS_FIFO_NE_TEMP  = 1 << SID_TEMP
BIT_STATUS_FIFO_NE_HUM   = 1 << SID_HUM
BIT_STATUS_FIFO_NE_PRESS = 1 << SID_PRESS
BIT_STATUS_FIFO_NE_ACC   = 1 << SID_ACC
BIT_STATUS_FIFO_NE_GYRO  = 1 << SID_GYRO
BIT_STATUS_FIFO_NE_MAG   = 1 << SID_MAG

# INTERVAL Register
# ===============
INTERVAL = 0

# Sensor Select Register
# ======================
SELECT = 0

# Enable Register
# ===============
# Enable (1) or Disable (0) selected sensor
ENABLE = []

# Scale Register
# ==============
SCALE = []

# Output data rate (in microseconds (Hz))
# =======================================
ODR = []

# Output data rate counter
# ========================
ODR_CNT = []

# FIFO Count Register
# ===================
# Number of samples (available to read) in FIFO (per sensor)
FIFO_CNT = []

# FIFO Register
# =============
# FIFO data buffers (per sensor)
FIFO = []

# FIFO Timestamp Register
# =======================
# FIFO data timestamp (per sensor)
FIFO_TS = []

## Create and initialize user registers
def CreateUserRegisters():

    # Create lists of registers (per sensor)
    for i in range(SENSOR_COUNT):
        ENABLE.append(list())
        SCALE.append(list())
        ODR.append(list())
        ODR_CNT.append(list())
        FIFO_CNT.append(list())
        FIFO.append(list())
        FIFO_TS.append(list())

    # Initialize registers
    for i in range(SENSOR_COUNT):
        ENABLE[i]   = 0
        SCALE[i]    = 100000
        ODR[i]      = 0
        ODR_CNT[i]  = 0
        FIFO_CNT[i] = 0

## Open CSV file containing sensor data
def openDataFile(file_name):
    global FIFO, FIFO_TS, ODR
    logging.info("openDataFile({}) called".format(file_name))

    f = open(file_name)

    # Read file header and determine column numbers for particular sensor value
    components = f.readline().split(",")
    
    # Keep characters only (remove white spaces, newline, ...)
    for i in range(len(components)):
        components[i] = components[i].strip()

    # Display CSV file header
    logging.debug("Header: {}".format(components))

    # Update key values according to the input file format
    for k, v in CSV_Col.items():
        for i in range(len(components)):
            if k == components[i]:
                CSV_Col.update({k : i})
                break

    # CSV_Col now contains updated column number for particular sensor value
    # Value -1 means that sensor data is not present
    #logging.debug("{}".format(CSV_Col))

    # Read line by line and parse data into FIFOs
    for line in f:
        data = line.split(",")

        # Keep characters only (remove white spaces, newline, ...)
        for i in range(len(data)):
            data[i] = data[i].strip()
        
        #logging.debug("Data: {}".format(data))

        if CSV_Col['Timestamp'] != -1:
            Ts = data[CSV_Col['Timestamp']]
        else:
            Ts = ""

        # Fill up the FIFOs
        if CSV_Col['Temp'] != -1 and data[CSV_Col['Temp']] != "":
            FIFO[SID_TEMP].append(data[CSV_Col['Temp']])
            FIFO_TS[SID_TEMP].append(Ts)
        if CSV_Col['Hum'] != -1 and data[CSV_Col['Hum']] != "":
            FIFO[SID_HUM].append(data[CSV_Col['Hum']])
            FIFO_TS[SID_HUM].append(Ts)
        if CSV_Col['Press'] != -1 and data[CSV_Col['Press']] != "":
            FIFO[SID_PRESS].append(data[CSV_Col['Press']])
            FIFO_TS[SID_PRESS].append(Ts)
        if CSV_Col['AccX'] != -1 and data[CSV_Col['AccX']] != "":
            FIFO[SID_ACC].append(data[CSV_Col['AccX']])
            FIFO_TS[SID_ACC].append(Ts)
        if CSV_Col['AccY'] != -1 and data[CSV_Col['AccY']] != "":
            FIFO[SID_ACC].append(data[CSV_Col['AccY']])
            FIFO_TS[SID_ACC].append(Ts)
        if CSV_Col['AccZ'] != -1 and data[CSV_Col['AccZ']] != "":
            FIFO[SID_ACC].append(data[CSV_Col['AccZ']])
            FIFO_TS[SID_ACC].append(Ts)
        if CSV_Col['GyroX'] != -1 and data[CSV_Col['GyroX']] != "":
            FIFO[SID_GYRO].append(data[CSV_Col['GyroX']])
            FIFO_TS[SID_GYRO].append(Ts)
        if CSV_Col['GyroY'] != -1 and data[CSV_Col['GyroY']] != "":
            FIFO[SID_GYRO].append(data[CSV_Col['GyroY']])
            FIFO_TS[SID_GYRO].append(Ts)
        if CSV_Col['GyroZ'] != -1 and data[CSV_Col['GyroZ']] != "":
            FIFO[SID_GYRO].append(data[CSV_Col['GyroZ']])
            FIFO_TS[SID_GYRO].append(Ts)
        if CSV_Col['MagX'] != -1 and data[CSV_Col['MagX']] != "":
            FIFO[SID_MAG].append(data[CSV_Col['MagX']])
            FIFO_TS[SID_MAG].append(Ts)
        if CSV_Col['MagY'] != -1 and data[CSV_Col['MagY']] != "":
            FIFO[SID_MAG].append(data[CSV_Col['MagY']])
            FIFO_TS[SID_MAG].append(Ts)
        if CSV_Col['MagZ'] != -1 and data[CSV_Col['MagZ']] != "":
            FIFO[SID_MAG].append(data[CSV_Col['MagZ']])
            FIFO_TS[SID_MAG].append(Ts)

    f.close()

    if CSV_Col['Timestamp'] != -1:
        # Timestamp is provided, determine initial ODR
        for sid in range(SENSOR_COUNT):
            if len(FIFO_TS[sid]) > 0:
                if (sid == SID_TEMP) or (sid == SID_HUM) or (sid == SID_PRESS):                
                    t0 = FIFO_TS[sid][0]
                    t1 = FIFO_TS[sid][1]
                else:
                    t0 = FIFO_TS[sid][0]
                    t1 = FIFO_TS[sid][3]

            ODR[sid] = int(t1) - int(t0)

def enTEMP(enable):
    if enable and CSV_Col['Temp'] == -1:
        # Sensor samples not available
        enable = 0

    return enable

def enHUM(enable):
    if enable and CSV_Col['Hum'] == -1:
        # Sensor samples not available
        enable = 0

    return enable

def enPRESS(enable):
    if enable and CSV_Col['Press'] == -1:
        # Sensor samples not available
        enable = 0

    return enable

def enACC(enable):
    if enable and (CSV_Col['AccX'] == -1 and CSV_Col['AccY'] == -1 and CSV_Col['AccZ'] == -1):
        # Sensor samples not available
        enable = 0

    return enable

def enGYRO(enable):
    if enable and (CSV_Col['GyroX'] == -1 and CSV_Col['GyroY'] == -1 and CSV_Col['GyroZ'] == -1):
        # Sensor samples not available
        enable = 0

    return enable

def enMAG(enable):
    if enable and (CSV_Col['MagX'] == -1 and CSV_Col['MagY'] == -1 and CSV_Col['MagZ'] == -1):
        # Sensor samples not available
        enable = 0

    return enable

# Handle Timer Interval Expiration
def IntervalHandler():
    global ODR_CNT, FIFO_CNT
    logging.debug("IntervalHandler() called")

    for sid in range(SENSOR_COUNT):
        if ENABLE[sid]:
            # Sensor is enabled, check its interval counter
            if ODR_CNT[sid] > 0:
                # Decrease interval down-counter
                ODR_CNT[sid] -= INTERVAL
            
            if ODR_CNT[sid] == 0:
                # Increase number of samples in FIFO
                FIFO_CNT[sid] += int(INTERVAL / ODR[sid])

                if sid >= SID_ACC:
                    # Motion sensor sample consists of 3 FIFO entries
                    FIFO_CNT[sid] *= 3

                # Interval expired, reload down-counter
                ODR_CNT[sid] = ODR[sid]
            
            logging.debug("SID={}, ODR_CNT={}. FIFO_CNT={}".format(sid, ODR_CNT[sid], FIFO_CNT[sid]))

## Calculate the timer overflow interval to service enabled sensors
# Function calculates greatest common divisor among ODR registers
def CalculateInterval():
    odr = list()
    
    for sid in range(SENSOR_COUNT):
        if ODR[sid] > 0:
            odr.append(ODR[sid])

    i = 1
    while i < len(odr): 
        odr[0] = math.gcd(odr[0], odr[i])
        i += 1

    value = odr[0]

    return value

# USER REGISTER HANDLING
# ======================

## Read ENABLE register (user register)
#  @return value value read (32-bit)
def rdENABLE():
    global SELECT, ENABLE

    sid = SELECT
    value = ENABLE[sid]

    logging.debug("Read ENABLE[{}]: {}".format(sid, value))
    return value

## Write ENABLE register (user register)
#  @param value value to write (32-bit)
def wrENABLE(value):
    global SELECT, ENABLE

    sid = SELECT

    if   sid == SID_TEMP:
        value = enTEMP(value)
    elif sid == SID_HUM:
        value = enHUM(value)
    elif sid == SID_PRESS:
        value = enPRESS(value)
    elif sid == SID_ACC:
        value = enACC(value)
    elif sid == SID_GYRO:
        value = enGYRO(value)
    elif sid == SID_MAG:
        value = enMAG(value)

    logging.debug("Write ENABLE[{}] = {}".format(sid, value))

    ENABLE[sid] = value

## Read STATUS register (user register)
#  @return value value read (32-bit)
def rdSTATUS():
    global STATUS, FIFO_CNT

    value = 0

    for sid in range(SENSOR_COUNT):
        if FIFO_CNT[sid] > 0:
            value |= 1 << sid

    STATUS = value

    logging.debug("Read STATUS: {}".format(value))
    return value

## Read INTERVAL register (user register)
#  @return value value read (32-bit)
def rdINTERVAL():
    global INTERVAL

    value = INTERVAL

    logging.debug("Read INTERVAL: {}".format(value))
    return value

## Read SELECT register (user register)
#  @return value value read (32-bit)
def rdSELECT(value):
    global SELECT

    value = SELECT

    logging.debug("Read SELECT: {}".format(value))
    return value

## Write SELECT register (user register)
#  @param value value to write (32-bit)
def wrSELECT(value):
    global SELECT
    logging.debug("Write SELECT = {}".format(value))

    SELECT = value

## Read SCALE register (user register)
#  @return value value read (32-bit)
def rdSCALE():
    global SELECT, SCALE

    sid = SELECT
    value = SCALE[sid]

    logging.debug("Read SCALE[{}]: {}".format(sid, value))
    return value

## Write SCALE register (user register)
#  @param value value to write (32-bit)
def wrSCALE(value):
    global SELECT, SCALE

    sid = SELECT
    logging.debug("Write SCALE[{}] = {}".format(sid, value))

    SCALE[sid] = value

## Read ODR register (user register)
#  @return value value read (32-bit)
def rdODR():
    global SELECT, ODR

    sid = SELECT
    value = ODR[sid]

    logging.debug("Read ODR[{}]: {}".format(sid, value))

    return value

## Write ODR register (user register)
#  @param value value to write (32-bit)
def wrODR(value):
    global SELECT, ODR

    sid = SELECT
    logging.debug("Write ODR[{}] = {}".format(sid, value))

    ODR[sid] = value

## Read FIFO_CNT register (user register)
#  @return value value read (32-bit)
def rdFIFO_CNT():
    global SELECT, FIFO_CNT

    sid = SELECT
    value = FIFO_CNT[sid]

    logging.debug("Read FIFO_CNT[{}]: {}".format(sid, value))
    return value

## Read FIFO register (user register)
#  @return value value read (32-bit)
def rdFIFO():
    global SELECT, FIFO, FIFO_TS, FIFO_CNT

    sid = SELECT

    if len(FIFO[sid]) > 0 and FIFO_CNT[sid] > 0:
        # Pop sample from FIFOs (Timestamp + Sample value)
        FIFO_TS[sid].pop(0)
        val = FIFO[sid].pop(0)
        # Decrement virtual FIFO counter
        FIFO_CNT[sid] -= 1

        if len(FIFO[sid]) == 0:
            # No more samples in sensor FIFO, disable sensor
            ENABLE[sid] = 0
    else:
        val = 0

    scaled_float = float(val) * SCALE[sid]

    value = int(scaled_float)
    value &= 0xffffffff

    logging.debug("Read FIFO[{}]: {} ({})".format(sid, value, val))
    return value

# VSI IMPLEMENTATION
# ==================

## Initialize
def init():
    global INTERVAL
    logging.info("init() called")
    logging.debug("Current working directory: {}".format(os.getcwd()))

    CreateUserRegisters()
   
    openDataFile(FILE_NAME_SENSOR)

    # Initialize timer interval register
    INTERVAL = CalculateInterval()


## Read interrupt request (the VSI IRQ Status Register)
#  @return value value read (32-bit)
def rdIRQ():
    global IRQ_Status
    logging.info("rdIRQ() called")

    value = IRQ_Status
    logging.debug("Read interrupt request: {}".format(value))

    return value


## Write interrupt request (the VSI IRQ Status Register)
#  @param value value to write (32-bit)
#  @return value value written (32-bit)
def wrIRQ(value):
    global IRQ_Status
    logging.info("wrIRQ(value={}) called".format(value))

    IRQ_Status = value
    logging.debug("Write interrupt request: {}".format(value))

    return value


## Write Timer registers (the VSI Timer Registers)
#  @param index Timer register index (zero based)
#  @param value value to write (32-bit)
#  @return value value written (32-bit)
def wrTimer(index, value):
    global Timer_Control, Timer_Interval
    logging.info("wrTimer(index={}, value={}) called".format(index, value))

    if   index == 0:
        Timer_Control = value
        logging.debug("Write Timer_Control: {}".format(value))
    elif index == 1:
        Timer_Interval = value
        logging.debug("Write Timer_Interval: {}".format(value))

    return value


## Timer event (called at Timer Overflow)
def timerEvent():
    global Timer_Event
    logging.info("timerEvent() called")

    Timer_Event += 1

    IntervalHandler()


## Write DMA registers (the VSI DMA Registers)
#  @param index DMA register index (zero based)
#  @param value value to write (32-bit)
#  @return value value written (32-bit)
def wrDMA(index, value):
    global DMA_Control
    logging.info("wrDMA(index={}, value={}) called".format(index, value))

    if   index == 0:
        DMA_Control = value
        logging.debug("Write DMA_Control: {}".format(value))

    return value


## Read data from peripheral for DMA P2M transfer (VSI DMA)
#  @param size size of data to read (in bytes, multiple of 4)
#  @return data data read (bytearray)
def rdDataDMA(size):
    logging.info("rdDataDMA(size={}) called".format(size))

    return bytearray()


## Write data to peripheral for DMA M2P transfer (VSI DMA)
#  @param data data to write (bytearray)
#  @param size size of data to write (in bytes, multiple of 4)
def wrDataDMA(data, size):
    logging.info("wrDataDMA(data={}, size={}) called".format(data, size))


## Read user registers (the VSI User Registers)
#  @param index user register index (zero based)
#  @return value value read (32-bit)
def rdRegs(index):
    logging.info("rdRegs(index={}) called".format(index))

    if   index == IDX_ENABLE:
        value = rdENABLE()
    elif index == IDX_STATUS:
        value = rdSTATUS()
    elif index == IDX_INTERVAL:
        value = rdINTERVAL()
    elif index == IDX_SELECT:
        value = rdSELECT()
    elif index == IDX_SCALE:
        value = rdSCALE()
    elif index == IDX_ODR:
        value = rdODR()
    elif index == IDX_FIFO_CNT:
        value = rdFIFO_CNT()
    elif index == IDX_FIFO:
        value = rdFIFO()
    else:
        value = 0

    return value


## Write user registers (the VSI User Registers)
#  @param index user register index (zero based)
#  @param value value to write (32-bit)
#  @return value value written (32-bit)
def wrRegs(index, value):
    logging.info("wrRegs(index={}, value={}) called".format(index, value))

    if   index == IDX_ENABLE:
        wrENABLE(value)
    elif index == IDX_SELECT:
        wrSELECT(value)
    elif index == IDX_SCALE:
        wrSCALE(value)
    elif index == IDX_ODR:
        wrODR(value)

    return value

## @}

def main():
    global SELECT, FIFO_CNT
    init()
    enTEMP(1)
    enACC(1)

    SELECT = 3
    FIFO_CNT[SELECT] = 7 * 3
    
    while FIFO_CNT[SELECT]:
        rdFIFO()

if __name__ == '__main__': main()
