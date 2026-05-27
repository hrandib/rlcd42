/*
 * BSD 3-Clause License
 *
 * Copyright (c) 2026, Dmytro Shestakov
 */

#ifndef SENSOR_DESC_HPP
#define SENSOR_DESC_HPP

#include <cstddef>

class SensorDescriptor
{
private:
    const char* name;
    const char* unit;
    const char* source;
public:
    using sensor_callback_t = void (*)(float value);

    SensorDescriptor(const char* name, const char* unit, const char* source) : name(name), unit(unit), source(source)
    { }

    const char* get_name() const
    {
        return name;
    }
    const char* get_unit() const
    {
        return unit;
    }
    const char* get_source() const
    {
        return source;
    }

    virtual float read_value() const = 0;
    virtual void on_update(sensor_callback_t callback) const {
        // Default implementation does nothing, can be overridden by derived classes
    };

    virtual ~SensorDescriptor() = default;
};

#endif // SENSOR_DESC_HPP
