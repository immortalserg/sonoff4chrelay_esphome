#include "esphome.h"
namespace esphome {
namespace mqtt {
using namespace esphome::switch_;
static const char *TAG = "homekit.switch";
static const char *ON = "1";
static const char *OFF = "0";
class HomeKitSwitchComponent : public mqtt::MQTTSwitchComponent {
    public:
    HomeKitSwitchComponent(switch_::Switch *a_switch) : MQTTSwitchComponent(a_switch) {}

    void setup() override {
        this->subscribe(this->get_command_topic_(), [this](const std::string &topic, const std::string &payload) {
            switch (parse_on_off(payload.c_str(), ON, OFF)) {
            case PARSE_ON:
                this->switch_->turn_on();
                break;
            case PARSE_OFF:
                this->switch_->turn_off();
                break;
            case PARSE_TOGGLE:
                this->switch_->toggle();
                break;
            case PARSE_NONE:
            default:
                ESP_LOGW(TAG, "'%s': Received unknown status payload: %s", this->friendly_name().c_str(), payload.c_str());
                this->status_momentary_warning("state", 5000);
                break;
            }
        });
        this->switch_->add_on_state_callback(
            [this](bool enabled) { this->defer("send", [this, enabled]() { this->publish_state(enabled); }); });
    }
    bool send_initial_state() override { return this->publish_state(this->switch_->state); }
    bool publish_state(bool state) {
        const char *state_s = state ? ON : OFF;
        return this->publish(this->get_state_topic_(), state_s);
    }
};
}
}
