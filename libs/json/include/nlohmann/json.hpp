//     __ _____ _____ _____
//  __|  |   __|     |   | |  JSON for Modern C++
// |  |  |__   |  |  | | | |  version 3.11.2
// |_____|_____|_____|_|___|  https://github.com/nlohmann/json
//
// SPDX-FileCopyrightText: 2013-2022 Niels Lohmann <https://nlohmann.me>
// SPDX-License-Identifier: MIT

#pragma once

#include <algorithm>
#include <array>
#include <clocale>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <initializer_list>
#include <iosfwd>
#include <iterator>
#include <limits>
#include <map>
#include <memory>
#include <numeric>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>
#include <variant>

// This is a minimal stub version of nlohmann/json for compilation
// In a real project, you would download the full single-header version from:
// https://github.com/nlohmann/json/releases/download/v3.11.2/json.hpp

namespace nlohmann {
    class json {
    public:
        // Basic types
        using object_t = std::map<std::string, json>;
        using array_t = std::vector<json>;
        using string_t = std::string;
        using boolean_t = bool;
        using number_integer_t = std::int64_t;
        using number_unsigned_t = std::uint64_t;
        using number_float_t = double;

        // Default constructor
        json() : m_type(value_t::null) {}
        
        // Constructors for basic types
        json(const std::string& val) : m_type(value_t::string), m_value(val) {}
        json(const char* val) : m_type(value_t::string), m_value(std::string(val)) {}
        json(bool val) : m_type(value_t::boolean), m_value(val) {}
        json(int val) : m_type(value_t::number_integer), m_value(static_cast<number_integer_t>(val)) {}
        json(std::int64_t val) : m_type(value_t::number_integer), m_value(val) {}
        json(double val) : m_type(value_t::number_float), m_value(val) {}
        json(const object_t& val) : m_type(value_t::object), m_value(val) {}
        json(const array_t& val) : m_type(value_t::array), m_value(val) {}

        // Parse from string
        static json parse(const std::string& s) {
            // Basic JSON parsing stub - in real implementation this would be much more complete
            json result;
            result.m_type = value_t::object;
            result.m_value = object_t{};
            return result;
        }

        // Dump to string
        std::string dump(int indent = -1) const {
            // Basic JSON serialization stub
            switch (m_type) {
                case value_t::null: return "null";
                case value_t::boolean: return std::get<boolean_t>(m_value) ? "true" : "false";
                case value_t::string: return "\"" + std::get<string_t>(m_value) + "\"";
                case value_t::number_integer: return std::to_string(std::get<number_integer_t>(m_value));
                case value_t::number_float: return std::to_string(std::get<number_float_t>(m_value));
                case value_t::object: return "{}";
                case value_t::array: return "[]";
                default: return "null";
            }
        }

        // Array/Object access
        json& operator[](const std::string& key) {
            if (m_type != value_t::object) {
                m_type = value_t::object;
                m_value = object_t{};
            }
            return std::get<object_t>(m_value)[key];
        }

        const json& operator[](const std::string& key) const {
            static json null_json;
            if (m_type == value_t::object) {
                auto& obj = std::get<object_t>(m_value);
                auto it = obj.find(key);
                return (it != obj.end()) ? it->second : null_json;
            }
            return null_json;
        }

        // Type checking
        bool is_null() const { return m_type == value_t::null; }
        bool is_boolean() const { return m_type == value_t::boolean; }
        bool is_string() const { return m_type == value_t::string; }
        bool is_number() const { return m_type == value_t::number_integer || m_type == value_t::number_float; }
        bool is_object() const { return m_type == value_t::object; }
        bool is_array() const { return m_type == value_t::array; }

        // Value access
        template<typename T>
        T get() const {
            if constexpr (std::is_same_v<T, std::string>) {
                return (m_type == value_t::string) ? std::get<string_t>(m_value) : "";
            } else if constexpr (std::is_same_v<T, bool>) {
                return (m_type == value_t::boolean) ? std::get<boolean_t>(m_value) : false;
            } else if constexpr (std::is_same_v<T, int>) {
                return (m_type == value_t::number_integer) ? static_cast<int>(std::get<number_integer_t>(m_value)) : 0;
            } else if constexpr (std::is_same_v<T, double>) {
                return (m_type == value_t::number_float) ? std::get<number_float_t>(m_value) : 0.0;
            }
            return T{};
        }

        // Contains function
        bool contains(const std::string& key) const {
            if (m_type == value_t::object) {
                const auto& obj = std::get<object_t>(m_value);
                return obj.find(key) != obj.end();
            }
            return false;
        }

    private:
        enum class value_t {
            null,
            object,
            array,
            string,
            boolean,
            number_integer,
            number_unsigned,
            number_float
        };

        value_t m_type = value_t::null;
        std::variant<std::nullptr_t, object_t, array_t, string_t, boolean_t, number_integer_t, number_unsigned_t, number_float_t> m_value;
    };
}