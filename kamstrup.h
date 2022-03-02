#include "esphome.h"

#define EMPTY ""
#define ECHO "E"
#define FACTORY_RESET "&F"
#define IPR "+IPR"
#define DUMP "+DUMP"
#define READ_SMS "+CMGR"
#define SEND_SMS "+CMGS"
#define DELETE_SMS "+CMGD"
#define NEW_MESSAGE_INDICATION "+CNMI"


enum AT_COMMAND { UNKNOWN, AT, AT_FACTORY_SETTINGS, AT_ECHO, AT_IPR, AT_READ_SMS, AT_SEND_SMS, AT_DELETE_SMS, AT_NEW_MESSAGE_INDICATION };
enum MACHINE_STATE { READ_AT_COMMAND, READ_SMS_BODY };


class RikaComponent : public PollingComponent, public UARTDevice {
 public:
  RikaComponent(UARTComponent *parent) : PollingComponent(15000), UARTDevice(parent) {
    this->machine_state = READ_AT_COMMAND;
  }

  MACHINE_STATE machine_state;
  AT_COMMAND at_command;

  void setup() override {
  }

  int readline(int readch, char *buffer, int len)
  {
    static int pos = 0;
    int rpos;

    if (readch > 0) {
      switch (readch) {
        case '\n': // Ignore new-lines
          break;
        case '\r': // Return on CR
          rpos = pos;
          pos = 0;  // Reset position index ready for next time
          return rpos;
        default:
          if (pos < len-1) {
            buffer[pos++] = readch;
            buffer[pos] = 0;
          }
      }
    }
    // No end of line has been found, so return -1.
    return -1;
  }

  void loop() override {
    // Do we have an SMS ready to read? If yes, read & process it
    const int max_line_length = 80;
    static char buffer[max_line_length];
    switch (this->machine_state) {
      case READ_AT_COMMAND: {
        String at_command_text = "";
        while (available()) {
          if(readline(read(), buffer, max_line_length) > 0) {
	      ESP_LOGI("rika", "%s", buffer);
	  }
	}
	return;
        AT_COMMAND command = parse_at_command(at_command_text);
        process_at_command(command);
        break;
        }
      case READ_SMS_BODY: {
        break;
        }
    }
  }

  void update() override {
  }

  // Parses Command:
  // Takes a string containing hopefully an AT command, and finds it equivalent in
  // the command enum that we have defined.
  // Also remove the "AT" part of the commands (ie: the first 2 bytes of the string).
  AT_COMMAND parse_at_command(String at_command) {

    // We remove the "AT" part of the AT command (ex: AT+IPR)
    at_command.remove(0, 2);
    if (at_command.equals(EMPTY)) {
      return AT;
    }

    if (at_command.startsWith(IPR)) {
      return AT_IPR;
    }

    if (at_command.startsWith(READ_SMS)) {
      return AT_READ_SMS;
    }

    if (at_command.startsWith(DELETE_SMS)) {
      return AT_DELETE_SMS;
    }

    if (at_command.startsWith(FACTORY_RESET)) {
      return AT_FACTORY_SETTINGS;
    }

    if (at_command.startsWith(ECHO)) {
      return AT_ECHO;
    }

    if (at_command.startsWith(SEND_SMS)) {
      return AT_SEND_SMS;
    }

    if (at_command.startsWith(NEW_MESSAGE_INDICATION)) {
      return AT_NEW_MESSAGE_INDICATION;
    }

    return UNKNOWN;
  }

  // process the command and sends the correct answer
  // here, we just update our sensors without publishing
  // state. States will get published when HA polls us.
  void process_at_command(AT_COMMAND command) {
    String payload;

    switch (this->machine_state) {
      case READ_SMS_BODY: {
	break;
        }

      case READ_AT_COMMAND: {
        switch (command) {
          case AT:
          case AT_FACTORY_SETTINGS:
          case AT_ECHO:
          case AT_IPR:
	  case AT_NEW_MESSAGE_INDICATION:
          case AT_DELETE_SMS:
            break;

          case AT_READ_SMS:
            payload = format_status_sms();
            send_payload(payload);
            break;

          case AT_SEND_SMS:
            this->machine_state = READ_SMS_BODY;
            break;

          case UNKNOWN:
            send_ok();
            break;
        }
	break;
        }
    }
  }

  void send_ok() {
    write(char(13));
    write(char(10));
    //print("OK");
    write(char(13));
    write(char(10));
  }
  
  void send_payload(String payload) {
    //print(payload);
    send_ok();
  }

  String format_send_sms_ok() {
    return String("AT+CMGS: 1");
  }
  
  String format_status_sms() {
    return String("AT+CMGR: \"REC READ\",\"+4520202020\",,\"07/04/20,10:08:02+32\"\r\n1234 ?\r\n");
  }
  
};
