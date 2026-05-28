/*
 * BSD 3-Clause License
 *
 * Copyright (c) 2026, Dmytro Shestakov
 */

#ifndef SENSOR_DESC_HPP
#define SENSOR_DESC_HPP

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
        float value;
    };
    using SensorDataArray = std::vector<SensorDataItem>;

    string source;
    SensorDataArray data_;
public:
    using sensor_callback_t = std::function<void(const SensorDataArray&)>;

    ISensorDataSource(string_view source) : source(source), data_()
    { }

    string_view get_source() const
    {
        return source;
    }

    virtual const SensorDataArray& read_values() const
    {
        return data_;
    }
    virtual void on_update(sensor_callback_t callback) const {
        // Default implementation does nothing, can be overridden by derived classes
    };

    virtual ~ISensorDataSource() = default;
};

#endif // SENSOR_DESC_HPP
