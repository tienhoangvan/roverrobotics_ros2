#include "rclcpp/rclcpp.hpp"
#include "serial/serial.h"
#include "atomic"
#include <chrono>
#include "openrover_core_msgs/msg/raw_data.hpp"
#include "openrover_core_msgs/msg/raw_motor_command.hpp"
#include "openrover_core_msgs/msg/raw_command.hpp"
using namespace openrover_core_msgs;
using namespace std::chrono_literals;

using duration = std::chrono::nanoseconds;

/// Responsible for managing the serial connection and communicating with the rover.
class OpenRoverSerial : public rclcpp::Node {
public:
	OpenRoverSerial(std::string port);
	static std::vector<std::string> list_ftdi_ports();
protected:
	std::atomic<std::array<uint8_t, 3>> motor_efforts_u8;

	/// Should send messages at this frequency, even if no data is requested.
	duration keepalive_period = 100ms;

	/// If no motor speed is commanded for this long, kill the motors
	duration kill_motors_timeout = 333ms;

	/// How often to poll for a uart message
	duration uart_poll_period = 1ms;

	rclcpp::Publisher<openrover_core_msgs::msg::RawData>::SharedPtr pub_raw_data;
	rclcpp::Subscription<openrover_core_msgs::msg::RawMotorCommand>::SharedPtr sub_motor_efforts;
	rclcpp::Subscription<openrover_core_msgs::msg::RawCommand>::SharedPtr sub_raw_commands;

	void on_raw_command(openrover_core_msgs::msg::RawCommand::SharedPtr);
	void read_callback();
	void keepalive_callback();
	void on_kill_motors();
	void on_motor_speed_commanded(openrover_core_msgs::msg::RawMotorCommand::SharedPtr msg);
	rclcpp::TimerBase::SharedPtr keepalive_timer;
	rclcpp::TimerBase::SharedPtr read_timer;
	rclcpp::TimerBase::SharedPtr kill_motors_timer;

	std::unique_ptr<serial::Serial> serial;
};

class OpenRoverError : public std::runtime_error {
public:
	OpenRoverError(const char* msg) : std::runtime_error(msg) {};
	OpenRoverError(const std::string msg) : std::runtime_error(msg) {};
};