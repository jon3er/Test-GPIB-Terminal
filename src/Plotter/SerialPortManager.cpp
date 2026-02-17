#include "SerialPortManager.h"

SerialPortManager::SerialPortManager()
    : m_serial(m_ioContext), 
      m_workGuard(boost::asio::make_work_guard(m_ioContext))
{}

SerialPortManager::~SerialPortManager() {
    ClosePort();

    m_ioContext.stop();
    
    if (m_ioThread.joinable()) {
        m_ioThread.join();
    }
}

std::vector<std::string> SerialPortManager::ScanPorts() {
    std::vector<std::string> ports;
#ifdef _WIN32
    // Windows: Check COM1-COM256
    for (int i = 1; i <= 256; ++i) {
        std::string portName = "COM" + std::to_string(i);
        try {
            boost::asio::serial_port test_port(m_ioContext);
            test_port.open(portName);
            test_port.close();
            ports.push_back(portName);
        } catch (...) {
            // Port doesn't exist, continue
        }
    }
#else
    // Linux/Unix: Check /dev directory
    for (const auto& entry : std::filesystem::directory_iterator("/dev")) {
        std::string path = entry.path().string();
        if (path.find("ttyS") != std::string::npos ||
            path.find("ttyUSB") != std::string::npos ||
            path.find("ttyACM") != std::string::npos) {
            ports.push_back(path);
        }
    }
#endif
    return ports;
}

bool SerialPortManager::OpenPort(const std::string& portName, unsigned int baudRate) {
    try {
        if (m_serial.is_open()) m_serial.close();

        m_serial.open(portName);

        m_serial.set_option(boost::asio::serial_port_base::baud_rate(baudRate));
        m_serial.set_option(boost::asio::serial_port_base::character_size(8));
        m_serial.set_option(boost::asio::serial_port_base::parity(
            boost::asio::serial_port_base::parity::none));
        m_serial.set_option(boost::asio::serial_port_base::stop_bits(
            boost::asio::serial_port_base::stop_bits::one));
        m_serial.set_option(boost::asio::serial_port_base::flow_control(
            boost::asio::serial_port_base::flow_control::none));

#ifdef _WIN32
        // Windows: Set DTR via DCB (Device Control Block)
        auto handle = m_serial.native_handle();
        DCB dcb;
        GetCommState(handle, &dcb);
        dcb.fDtrControl = DTR_CONTROL_ENABLE;   // set DTR
        dcb.fRtsControl = RTS_CONTROL_DISABLE;  // clear RTS
        SetCommState(handle, &dcb);
#else
        // Linux/Unix: Set DTR/RTS via ioctl
        int fd = m_serial.native_handle();
        int flags;
        ioctl(fd, TIOCMGET, &flags);
        flags |= TIOCM_DTR;   // set DTR
        flags &= ~TIOCM_RTS;  // clear RTS
        ioctl(fd, TIOCMSET, &flags);
#endif


        std::cout << "Opened serial port: " << portName << " at " << baudRate << " baud\n";

        Write("\r\n\r\n");

        // CRITICAL: Wait for Arduino to reset and flush any garbage
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));

        return true;
    } catch (const boost::system::system_error& e) {
        std::cerr << "Error opening port " << portName << ": " << e.what() << "\n";
        return false;
    }
}

void SerialPortManager::ClosePort() {
    if (m_serial.is_open()) m_serial.close();
}

bool SerialPortManager::IsOpen() const {
    return m_serial.is_open();
}

bool SerialPortManager::Write(const std::string& data) {
	return this->Write(boost::asio::buffer(data));
}

bool SerialPortManager::Write(const boost::asio::const_buffer& buffer) {
    if (!m_serial.is_open()) return false;

    try {
        boost::asio::write(m_serial, buffer);
        return true;
    } catch (const boost::system::system_error& e) {
        std::cerr << "Write error: " << e.what() << "\n";
        return false;
    }
}

void SerialPortManager::StartAsyncRead(std::function<void(const std::string&)> onLineRead) {
    m_onLineRead = onLineRead;
    DoRead();

    if (!m_ioThread.joinable()) {
        m_ioThread = std::thread([this]() {
            m_ioContext.run();
        });
    }
}

void SerialPortManager::DoRead() {
    m_serial.async_read_some(boost::asio::buffer(&m_readChar, 1),
        [this](const boost::system::error_code& ec, std::size_t bytes_transferred) {
            if (!ec) {
                if (m_readChar == '\n') {
                    if (m_onLineRead) m_onLineRead(m_inputBuffer);
                    m_inputBuffer.clear();
                } else if (m_readChar != '\r') {
                    m_inputBuffer += m_readChar;
                }
                DoRead(); // Wait for the next character
            }
        });
}
