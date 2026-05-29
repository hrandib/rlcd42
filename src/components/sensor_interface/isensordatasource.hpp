/*
 * BSD 3-Clause License
 *
 * Copyright (c) 2026, Dmytro Shestakov
 */

#pragma once

#include <algorithm>
#include <cstddef>
#include <functional>
#include <string>
#include <string_view>
#include <vector>

class ISensorDataSource
{
protected:
    using string = std::string;
    using string_view = std::string_view;

    struct SensorDataItem
    {
        string name;
        string unit;
        string format;
        float value;
    };
    using SensorDataArray = std::vector<SensorDataItem>;

    string source;
    SensorDataArray data_;

    void update_item(string_view name, float value)
    {
        auto it = std::ranges::find_if(data_, [name](const SensorDataItem& item) {
            return item.name == name;
        });
        if(it != data_.end()) {
            it->value = value;
        }
    }

    void update_item_by_index(size_t index, float value)
    {
        if(index < data_.size()) {
            data_[index].value = value;
        }
    }
public:
    using sensor_callback_t = std::function<void(const SensorDataArray&)>;

    ISensorDataSource(string_view source) : source(source), data_()
    { }

    void store_item(string_view name, string_view unit, string_view format = "", float value = 0.0f)
    {
        data_.emplace_back(std::string(name), std::string(unit), std::string(format), value);
    }

    string_view get_source() const
    {
        return source;
    }

    virtual const SensorDataArray& read_values() const = 0;
    virtual void on_update(sensor_callback_t callback) const {
        // The target class may not use asynchronous updates, so this can be a no-op by default. Derived classes that
        // support async updates can override this method to allow setting a callback
    };

    virtual ~ISensorDataSource() = default;
};
