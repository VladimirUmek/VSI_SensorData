from encodings import utf_8
import socket
import numpy

#Configuration: Enabled sensors
env_sensors = ["temp", "hum", "press"]
motion_sensors = ["acc_x", "acc_y", "acc_z", "gyro_x", "gyro_y", "gyro_z", "mag_x", "mag_y", "mag_z"]

#Get local IP
UDP_IP = socket.gethostbyname(socket.gethostname())
print(f'{UDP_IP}')
UDP_PORT = 5000
#Create UDP socket (port 5000)
sock = socket.socket(socket.AF_INET,    # Internet
                     socket.SOCK_DGRAM) # UDP
sock.bind((UDP_IP, UDP_PORT))

#Open .csv file to save data
try:
  f = open("samples.csv", "w+")
except:
  print(f'Can nott open samples.csv')

count = 0
#Current implementation is limited and only receives 128 samples from each sensor
while count < 128:
  count += 1
  data, addr = sock.recvfrom(1024)
  
  if  count == 1:
  # first line is command line
    data = data.decode("utf-8")
    data = data.replace(" ", "")
    f.write(data)
    data = data.replace(",\r", "")    
    data = data.replace(",\n", "")
    data = data.replace("\r", "")    
    data = data.replace("\n", "")
    sensor_list = data.split(",")
  else:
  #following lines are data
    i = 0
    float_data = numpy.fromstring(data, dtype='<f4')
    int_data   = numpy.fromstring(data, dtype='<i4')
    for sens in sensor_list:      
      if sens == "timestamp":
        txt = f'{int_data[i]},'
        f.write(txt)        
      else :
        txt = f'{float_data[i]},'
        f.write(txt)
      i += 1
    f.write('\n')

    output = f'{count}    IP: {addr}    message: {data}'
    print(output)

#Close socket
sock.shutdown(0)
sock.close()

#Close .csv file
try:
  f.close()
except:
  output('Can not save/close samples.csv')

