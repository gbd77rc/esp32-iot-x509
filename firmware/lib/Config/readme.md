# Configuration

This library controls how the loading, saving and assigning JSON elements.  It will be a single instance class, as we create it automatically after defining it.  The instance name `Configuration`.

Each function has been commented.

## Example of use

    class InfoClass : public BaseConfigInfoClass
    {
        public:
            InfoClass() : BaseConfigInfoClass("Info") {}
            void load(JsonObjectConst obj)
            {
                this->_isEnabled = obj.containsKey("enabled") ? obj["enabled"].as<bool>() : true;
            }

            void save(JsonObject obj)
            {
                auto json = obj.createNestedObject(this->_sectionName);
                json["enabled"] = this->_isEnabled;
            }

            void LedInfoClass::toJson(JsonObject ob)
            {
                 auto json = ob.createNestedObject("counting");  // This element name can be anything as it not be used to save/load
                 json["enabled"] = this->_isEnabled ? "Yes" : "No"
                 json["count"] = this->_count;
            }

            void setEnabled(bool flag)
            {
                if (flag != this->_isEnabled)
                {
                    this->_isEnabled = flag;
                    this->_changed = true;
                }
            }

        private:
            bool _isEnabled;
            int  _count;
    }

    InfoClass Info;

    Configuration.begin("/config.json");
    Configuration.add(&Info);
    Configuration.load();

    Info.setEnabled(false);

    if (Configuration.shouldSave())
    {
        Configuration.save();
    }

This example shows how to create a class and then register it with the `Configuration` instance.
