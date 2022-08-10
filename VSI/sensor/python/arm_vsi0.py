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

## Set verbosity level
verbosity = logging.DEBUG
#verbosity = logging.ERROR

# [debugging] Verbosity settings
level = { 10: "DEBUG",  20: "INFO",  30: "WARNING",  40: "ERROR" }
logging.basicConfig(format='Py: VSI0: [%(levelname)s]\t%(message)s', level = verbosity)
logging.info("Verbosity level is set to " + level[verbosity])

# File names
#FILE_NAME_SENSOR = "sensor_samples.csv"
FILE_NAME_SENSOR = "..\\..\\..\\sensor_samples.csv"

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

#Define number of sensors
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
IDX_CTRL          = 0
IDX_STATUS        = 1
IDX_CLK_DIV       = 2
IDX_SELECT        = 3
IDX_SCALE         = 4
IDX_ODR           = 5
IDX_FIFO_CNT      = 6
IDX_FIFO          = 7
IDX_FIFO_MOTION_X = 8
IDX_FIFO_MOTION_Y = 9
IDX_FIFO_MOTION_Z = 10
# Reserved registers
IDX_ODR_CNT       = 11
IDX_FIFO_TS       = 12

# Control Register
# ================
CTRL = 0

# Bit definitions
BIT_CTRL_EN_TEMP  = 1 << SID_TEMP
BIT_CTRL_EN_HUM   = 1 << SID_HUM
BIT_CTRL_EN_PRESS = 1 << SID_PRESS
BIT_CTRL_EN_ACC   = 1 << SID_ACC
BIT_CTRL_EN_GYRO  = 1 << SID_GYRO
BIT_CTRL_EN_MAG   = 1 << SID_MAG

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

# Sensor Select Register
# ======================
SELECT = 0

# Measurement scale (default)
# ===========================
# ACC   Units: G
# GYRO  Units: dps
# MAG   Units: Gauss?
# TEMP  Units: deg Celsius
# HUM   Units: %
# PRESS Units: mBar
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


## Open CSV file containing sensor data
def openDataFile(file_name):
    logging.info("openDataFile({}) called".format(file_name))

    f = open(file_name)

    # Read file header and determine column numbers for particular sensor value
    components = f.readline().split(",")
    
    # Keep characters only (remove white spaces, newline, ...)
    for i in range(len(components)):
        components[i] = components[i].strip()

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

def enTEMP(enable):
    val = True
    if enable:
        # Enable sensor
        if CSV_Col['Temp'] != -1:
            # Load ODR register if timestamp is provided via input file
            if CSV_Col['Timestamp'] != -1:
                t0 = FIFO_TS[SID_TEMP][0]
                t1 = FIFO_TS[SID_TEMP][1]
                ODR[SID_TEMP] = int(t1) - int(t0)
        else:
            val = False
    else:
        # Disable sensor
        ODR[SID_TEMP] = 0
        FIFO_CNT[SID_TEMP] = 0

    return val

def enHUM(enable):
    val = True
    if enable:
        if CSV_Col['Hum'] != -1:
            # Load ODR register if timestamp is provided via input file
            if CSV_Col['Timestamp'] != -1:
                t0 = FIFO_TS[SID_HUM][0]
                t1 = FIFO_TS[SID_HUM][1]
                ODR[SID_HUM] = int(t1) - int(t0)
        else:
            val = False
    else:
        # Disable sensor
        ODR[SID_HUM] = 0
        FIFO_CNT[SID_HUM] = 0

    return val

def enPRESS(enable):
    val = True
    if enable:
        if CSV_Col['Press'] != -1:
            # Load ODR register if timestamp is provided via input file
            if CSV_Col['Timestamp'] != -1:
                t0 = FIFO_TS[SID_PRESS][0]
                t1 = FIFO_TS[SID_PRESS][1]
                ODR[SID_PRESS] = int(t1) - int(t0)
        else:
            val = False
    else:
        # Disable sensor
        ODR[SID_PRESS] = 0
        FIFO_CNT[SID_PRESS] = 0
    return val

def enACC(enable):
    val = True
    if enable:
        if CSV_Col['AccX'] != -1 and CSV_Col['AccY'] != -1 and CSV_Col['AccZ'] != -1:
            # Load ODR register if timestamp is provided via input file
            if CSV_Col['Timestamp'] != -1:
                t0 = FIFO_TS[SID_ACC][0]
                t1 = FIFO_TS[SID_ACC][3]
                ODR[SID_ACC] = int(t1) - int(t0)
        else:
            val = False
    else:
        # Disable sensor
        ODR[SID_ACC] = 0
        FIFO_CNT[SID_ACC] = 0
    return val

def enGYRO(enable):
    val = True
    if enable:
        if CSV_Col['GyroX'] != -1 and CSV_Col['GyroY'] != -1 and CSV_Col['GyroZ'] != -1:
            # Load ODR register if timestamp is provided via input file
            if CSV_Col['Timestamp'] != -1:
                t0 = FIFO_TS[SID_GYRO][0]
                t1 = FIFO_TS[SID_GYRO][3]
                ODR[SID_GYRO] = int(t1) - int(t0)
        else:
            val = False
    else:
        # Disable sensor
        ODR[SID_GYRO] = 0
        FIFO_CNT[SID_GYRO] = 0
    return val

def enMAG(enable):
    val = True
    if enable:
        if CSV_Col['MagX'] != -1 and CSV_Col['MagY'] != -1 and CSV_Col['MagZ'] != -1:
            # Load ODR register if timestamp is provided via input file
            if CSV_Col['Timestamp'] != -1:
                t0 = FIFO_TS[SID_MAG][0]
                t1 = FIFO_TS[SID_MAG][3]
                ODR[SID_MAG] = int(t1) - int(t0)
        else:
            val = False
    else:
        # Disable sensor
        ODR[SID_MAG] = 0
        FIFO_CNT[SID_MAG] = 0
    return val


# USER REGISTER HANDLING
# ======================

## Read CTRL register (user register)
#  @return value value read (32-bit)
def rdCTRL():
    #global CTRL
    logging.info("rdCTRL() called")

    value = CTRL

    logging.debug("rdCTRL() = {}".format(value))
    return value

## Write CTRL register (user register)
#  @param value value to write (32-bit)
def wrCTRL(value):
    global CTRL
    logging.debug("wrCTRL({}) called".format(value))

    if ((CTRL ^ value) & BIT_CTRL_EN_TEMP) != 0:
        if enTEMP((value & BIT_CTRL_EN_TEMP) != 0) == False:
            # Cannot enable, clear enable bit
            value &= ~BIT_CTRL_EN_TEMP

    if ((CTRL ^ value) & BIT_CTRL_EN_HUM) != 0:
        if enHUM((value & BIT_CTRL_EN_HUM) != 0) == False:
            # Cannot enable, clear enable bit
            value &= ~BIT_CTRL_EN_HUM

    if ((CTRL ^ value) & BIT_CTRL_EN_PRESS) != 0:
        if enPRESS((value & BIT_CTRL_EN_PRESS) != 0) == False:
            # Cannot enable, clear enable bit
            value &= ~BIT_CTRL_EN_PRESS

    if ((CTRL ^ value) & BIT_CTRL_EN_ACC) != 0:
        if enACC((value & BIT_CTRL_EN_ACC) != 0) == False:
            # Cannot enable, clear enable bit
            value &= ~BIT_CTRL_EN_ACC

    if ((CTRL ^ value) & BIT_CTRL_EN_GYRO) != 0:
        if enGYRO((value & BIT_CTRL_EN_GYRO) != 0) == False:
            # Cannot enable, clear enable bit
            value &= ~BIT_CTRL_EN_GYRO

    if ((CTRL ^ value) & BIT_CTRL_EN_MAG) != 0:
        if enMAG((value & BIT_CTRL_EN_MAG) != 0) == False:
            # Cannot enable, clear enable bit
            value &= ~BIT_CTRL_EN_MAG

    CTRL = value

## Read STATUS register (user register)
#  @return value value read (32-bit)
def rdSTATUS():
    global STATUS
    logging.info("rdSTATUS() called")

    value = 0

    for sid in range(len(FIFO)):
        value = (len(FIFO[sid]) > 0) << sid

    value = STATUS

    logging.debug("rdSTATUS() = {}".format(value))
    return value

## Read CLK_DIV register (user register)
#  @return value value read (32-bit)
def rdCLK_DIV():
    logging.info("rdCLK_DIV() called")

    odr = [1000000]
    
    for sid in range(len(ODR)):
        if ODR[sid] > 0:
            odr.append(ODR[sid])

    i = 1
    while i < len(odr): 
        odr[0] = math.gcd(odr[0], odr[i])
        i += 1

    value = odr[0]

    logging.debug("rdCLK_DIV() = {}".format(value))
    return value

## Read SELECT register (user register)
#  @return value value read (32-bit)
def rdSELECT(value):
    global SELECT
    logging.debug("rdSELECT() called")

    value = SELECT

    logging.debug("rdSELECT() = {}".format(value))
    return value

## Write SELECT register (user register)
#  @param value value to write (32-bit)
def wrSELECT(value):
    global SELECT
    logging.debug("wrSELECT({}) called".format(value))

    SELECT = value

## Read SCALE register (user register)
#  @return value value read (32-bit)
def rdSCALE():
    global SCALE
    logging.debug("rdSCALE() called")

    value = SCALE[SELECT]

    logging.debug("rdSCALE() = {}".format(value))
    return value

## Write SCALE register (user register)
#  @param value value to write (32-bit)
def wrSCALE(value):
    global SCALE
    logging.debug("wrSCALE({}) called".format(value))

    SCALE[SELECT] = value

## Read ODR register (user register)
#  @return value value read (32-bit)
def rdODR():
    global ODR
    logging.debug("rdODR() called")

    value = ODR[SELECT]

    logging.debug("rdODR() = {}".format(value))

    return value

## Write ODR register (user register)
#  @param value value to write (32-bit)
def wrODR(value):
    global ODR
    logging.debug("wrODR({}) called".format(value))

    ODR[SELECT] = value

## Read FIFO_CNT register (user register)
#  @return value value read (32-bit)
def rdFIFO_CNT():
    global FIFO
    logging.debug("rdFIFO_CNT() called")

    #value = len(FIFO[SELECT])
    value = FIFO_CNT[SELECT]

    logging.debug("rdFIFO_CNT() = {}".format(value))
    return value

## Read FIFO register (user register)
#  @return value value read (32-bit)
def rdFIFO():
    global FIFO, FIFO_TS
    logging.debug("rdFIFO() called")

    FIFO_TS[SELECT].pop(0)
    value = FIFO[SELECT].pop(0)

    logging.debug("rdFIFO() = {}".format(value))
    return int(value)

## Read FIFO_MOTION_X register (user register)
#  @return value value read (32-bit)
def rdFIFO_MOTION_X():
    logging.debug("rdFIFO_MOTION_X() called")

    value = rdFIFO()

    logging.debug("rdFIFO_MOTION_X() = {}".format(value))
    return value

## Read FIFO_MOTION_Y register (user register)
#  @return value value read (32-bit)
def rdFIFO_MOTION_Y():
    logging.debug("rdFIFO_MOTION_Y() called")

    value = rdFIFO()

    logging.debug("rdFIFO_MOTION_Y() = {}".format(value))
    return value

## Read FIFO_MOTION_Z register (user register)
#  @return value value read (32-bit)
def rdFIFO_MOTION_Z():
    logging.debug("rdFIFO_MOTION_Z() called")

    value = rdFIFO()

    logging.debug("rdFIFO_MOTION_Z() = {}".format(value))
    return value


# VSI IMPLEMENTATION
# ==================

## Initialize
def init():
    logging.info("init() called")

    # Create sensor registers
    for i in range(SENSOR_COUNT):
        # Create FIFO registers (per sensor data FIFO and timestamp FIFO)
        FIFO.append(list())
        FIFO_TS.append(list())
        FIFO_CNT.append(list())
    
        # Create ODR registers
        ODR.append(list())
        ODR_CNT.append(list())

    # Initialize registers
    for i in range(SENSOR_COUNT):
        ODR[i] = 0
        ODR_CNT[i] = 0
   
    openDataFile(FILE_NAME_SENSOR)


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

    for sid in range(len(ODR)):
        # Check if sensor is enabled and its output data rate is set
        if (CTRL & (1 << sid)) == 1 and ODR[sid] > 0:

            if ODR_CNT[sid] == 0:
                # Reload ODR down-counter
                ODR_CNT[sid] = ODR[sid]
                # Increment number of samples in FIFO
                # FIFO_CNT[sid] += 1
                # Increase number of samples in FIFO
                FIFO_CNT[sid] += Timer_Interval / ODR[sid]
            else:
                # Decrease ODR counter
                ODR_CNT[sid] -= Timer_Interval
    

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

    if   index == IDX_CTRL:
        value = rdCTRL()
    elif index == IDX_STATUS:
        value = rdSTATUS()
    elif index == IDX_CLK_DIV:
        value = rdCLK_DIV()
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
    elif index == IDX_FIFO_MOTION_X:
        value = rdFIFO_MOTION_X()
    elif index == IDX_FIFO_MOTION_Y:
        value = rdFIFO_MOTION_Y()
    elif index == IDX_FIFO_MOTION_Z:
        value = rdFIFO_MOTION_Z()
    else:
        value = 0

    return value


## Write user registers (the VSI User Registers)
#  @param index user register index (zero based)
#  @param value value to write (32-bit)
#  @return value value written (32-bit)
def wrRegs(index, value):
    logging.info("wrRegs(index={}, value={}) called".format(index, value))

    if   index == IDX_CTRL:
        wrCTRL(value)
    elif index == IDX_SELECT:
        wrSELECT(value)
    elif index == IDX_SCALE:
        wrSCALE(value)
    elif index == IDX_ODR:
        wrODR(value)

    return value

## @}

def main():
    init()
    enTEMP(True)
    rdCLK_DIV()

if __name__ == '__main__': main()
