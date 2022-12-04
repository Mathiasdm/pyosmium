#ifndef PYOSMIUM_CAST_H
#define PYOSMIUM_CAST_H

#include <datetime.h>
#include <chrono>

#include <pybind11/pybind11.h>
#include <osmium/osm.hpp>

namespace pybind11 { namespace detail {
    template <> struct type_caster<osmium::Timestamp> {
    public:
        using type = osmium::Timestamp;

        bool load(handle src, bool) {
            // Lazy initialise the PyDateTime import
            if (!PyDateTimeAPI) { PyDateTime_IMPORT; }

            if (!src) {
                return false;
            }

            if (pybind11::isinstance<pybind11::str>(src)) {
                value = osmium::Timestamp(src.cast<std::string>());
                return true;
            }

            if (!PyDateTime_Check(src.ptr())) {
                return false;
            }

            auto ts = src.attr("timestamp")();
            value = (unsigned) ts.cast<double>();

            return true;
        }

        static handle cast(type const &src, return_value_policy, handle)
        {
            using namespace std::chrono;
            // Lazy initialise the PyDateTime import
            if (!PyDateTimeAPI) { PyDateTime_IMPORT; }

            std::time_t tt = src.seconds_since_epoch();
            std::tm localtime = *std::gmtime(&tt);
            handle pydate = PyDateTime_FromDateAndTime(localtime.tm_year + 1900,
                                                       localtime.tm_mon + 1,
                                                       localtime.tm_mday,
                                                       localtime.tm_hour,
                                                       localtime.tm_min,
                                                       localtime.tm_sec,
                                                       0);

            static auto utc = module::import("datetime").attr("timezone").attr("utc");
            using namespace literals;
            handle with_utc = pydate.attr("replace")("tzinfo"_a=utc).inc_ref();
            pydate.dec_ref();
            return with_utc;
        }

        PYBIND11_TYPE_CASTER(type, _("datetime.datetime"));
    };
}} // namespace pybind11::detail

namespace pyosmium {

template <typename T>
T const *try_cast(pybind11::object o) {
    if (!pybind11::hasattr(o, "_data")) {
        return nullptr;
    }

    auto inner = o.attr("_data");

    if (pybind11::isinstance<T>(inner)) {
        return inner.cast<T const *>();
    }

    return nullptr;
}

template <typename T>
T const &cast(pybind11::object o) {
    return o.attr("_data").cast<T const &>();
}


template <typename T>
T const *try_cast_list(pybind11::object o) {
    if (!pybind11::hasattr(o, "_list")) {
        return nullptr;
    }

    auto inner = o.attr("_list");

    if (pybind11::isinstance<T>(inner)) {
        return inner.cast<T const *>();
    }

    return nullptr;
}


template <typename T>
T const &cast_list(pybind11::object o) {
    return o.attr("_list").cast<T const &>();
}




}

#endif // PYOSMIUM_CAST_H
