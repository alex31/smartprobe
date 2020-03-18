#include <fcntl.h> 
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <string>
#include <array>

// g++ --std=c++17 -Wall -Wextra gps2smartprobe.cpp -o gps2smartprobe

static inline constexpr int getLinuxBaudRate(const uint32_t bauds)
{
  switch (bauds) {
  case 50       : return B50     ;    
  case 75       : return B75     ;     
  case 110      : return B110    ;     
  case 134      : return B134    ;    
  case 150      : return B150    ;    
  case 200      : return B200    ;    
  case 300      : return B300    ;     
  case 600      : return B600    ;   
  case 1200     : return B1200   ;   
  case 1800     : return B1800   ;  
  case 2400     : return B2400   ;  
  case 4800     : return B4800   ;  
  case 9600     : return B9600   ;  
  case 19200    : return B19200  ;  
  case 38400    : return B38400  ; 
  case 57600    : return B57600  ; 
  case 115200   : return B115200 ;
  case 230400   : return B230400 ; 
  case 460800   : return B460800 ;
  case 500000   : return B500000 ;
  case 576000   : return B576000 ;
  case 921600   : return B921600 ;
  case 1000000  : return B1000000;
  case 1152000  : return B1152000;
  case 1500000  : return B1500000;
  case 2000000  : return B2000000;
  case 2500000  : return B2500000;
  case 3000000  : return B3000000;
  case 3500000  : return B3500000;
  case 4000000  : return B4000000;
  default :
    std::cerr << "baud rate " << bauds << " not standard, aborting\n";
    abort();
    return 0;
  }
}




int set_interface_attribs(int fd, int speed)
{
    struct termios tty;

    if (tcgetattr(fd, &tty) < 0) {
        printf("Error from tcgetattr: %s\n", strerror(errno));
        return -1;
    }

    cfsetospeed(&tty, (speed_t)speed);
    cfsetispeed(&tty, (speed_t)speed);

    tty.c_cflag |= (CLOCAL | CREAD);    /* ignore modem controls */
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;         /* 8-bit characters */
    tty.c_cflag &= ~PARENB;     /* no parity bit */
    tty.c_cflag &= ~CSTOPB;     /* only need 1 stop bit */
    tty.c_cflag &= ~CRTSCTS;    /* no hardware flowcontrol */

    /* setup for non-canonical mode */
    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
    tty.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
    tty.c_oflag &= ~OPOST;

    /* fetch bytes as they become available */
    tty.c_cc[VMIN] = 255;
    tty.c_cc[VTIME] = 255;

    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        printf("Error from tcsetattr: %s\n", strerror(errno));
        return -1;
    }
    return 0;
}


int init_file(const std::string& portName)
{
  int fd = open(portName.c_str(), O_RDWR | O_NOCTTY | O_SYNC);
  if (fd < 0) {
    std::cerr << "Error opening " <<  portName << std::endl;
  }
  return fd;
}

int init_uart(const std::string& portName, const uint32_t baudRate)
{
  int fd = init_file(portName);

  if (fd >= 0) {
    /*baudrate 115200, 8 bits, no parity, 1 stop bit */
    if (set_interface_attribs(fd, getLinuxBaudRate(baudRate)) < 0)
      return fd;
    
    fcntl(fd, F_SETFL, 0);
  }
  
  return fd;
}


int main(int argc, char* argv[])
{
  if (argc != 4) {
    std::cerr << "need 3 arguments : gps smartprobe and speed" << std::endl;
    exit(-1);
  }
  const std::string gpsDev = argv[1];
  const std::string smartprobeDev = argv[2];
  const uint32_t baud = atoi(argv[3]);

  int gpsFd = -1;
  if (gpsDev.find("/dev/") != std::string::npos) {
    gpsFd = init_uart(gpsDev, baud);
  } else {
    gpsFd = init_file(gpsDev);
  }
  
  if (gpsFd < 0) {
    std::cout << "device " << gpsDev << "not found or unsupported baudrate " << baud << std::endl;
  }
  int smartprobeFd = init_uart(smartprobeDev, baud);
  if (smartprobeFd < 0) {
    std::cout << "device " << smartprobeDev << "not found or unsupported baudrate " << baud << std::endl;
  }

  while(true) {
    std::array<std::byte, 8> buf;
    int retv = read(gpsFd, buf.data(), buf.size());
    if (retv == static_cast<int>(buf.size())) {
      write(smartprobeFd, buf.data(), buf.size());
      write(STDOUT_FILENO, ".", 1);
      fsync(STDOUT_FILENO);
    }
  }
  
}


























