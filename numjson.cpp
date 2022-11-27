#include <cstring>
#include <iostream>
#include <pybind11/pybind11.h>

#ifdef STANDALONE
// for fuzzing
#include <pybind11/embed.h>
#endif

namespace py = pybind11;
struct __attribute__ ((visibility("hidden"))) res {
    py::object obj;
    const char * next;
};

struct res_string_view {
    std::string_view obj;
    const char * next;
};

const char * skip_whitespace(const char * s) {
    while (*s == ' ' || *s == '\n' || *s == '\r' || *s == '\t') {
        s += 1;
    }
    return s;
}

[[ noreturn ]] void error(const char * s) {
    throw py::value_error(s);
}

void __assert(bool x, const char * msg) {
    if (!x) {
        error(msg);
    }
}

#define _assert(x) __assert((x), #x)

res_string_view internal_read_string(const char * s) {
    _assert(s[0] == '"');
    s = s + 1;
    int len = 0;
    while (s[len] != '"') {
        if (s[len] == 0) {
            error("unexpected string end");
        }
        len += 1;
    }
    return { std::string_view(s, len), &s[len + 1] };
}

res read_value(const char * s);

res read_object(const char * s) {
    _assert(s[0] == '{');
    py::dict out;
    s = skip_whitespace(s + 1);
    while (*s != '}') {
        if (*s == 0) {
            error("unclosed object");
        }
        auto key = internal_read_string(s);
        s = skip_whitespace(key.next);
        if (*s != ':') {
            error("expected colon in object definition");
        }
        s = skip_whitespace(s + 1);
        auto value = read_value(s);
        std::string stupid_extra_copy(key.obj);
        out[stupid_extra_copy.c_str()] = value.obj;
        s = value.next;
        s = skip_whitespace(s);
        if (*s != '}' && *s != ',') {
            error("broken object");
        }
        if (*s == ',') {
            s = skip_whitespace(s + 1);
        }
    }
    return res { out, s + 1};
}

res read_array(const char * s) {
    _assert(s[0] == '[');
    s = skip_whitespace(s + 1);
    py::list out;
    while (*s != ']') {
        if (*s == 0) {
            error("unclosed array");
        }
        auto value = read_value(s);
        out.append(value.obj);
        s = skip_whitespace(value.next);
        if (*s != ']' && *s != ',') {
            error("broken array");
        }
        if (*s == ',') {
            s = skip_whitespace(s + 1);
        }
    }
    return res { out, s + 1};
}

res read_true(const char * s) {
    if (strncmp(s, "true", strlen("true")) == 0) {
        return res { py::bool_(true), s + strlen("true") };
    } else {
        error("expected true");
    }
}

res read_false(const char * s) {
    if (strncmp(s, "false", strlen("false")) == 0) {
        return res { py::bool_(false), s + strlen("false") };
    } else {
        error("expected false");
    }
}

res read_number(const char * s) {
    char * e;
    double x = strtod(s, &e);
    if (x == (long)x) {
        return res { py::int_((long)x), e };
    }
    return res { py::float_(x), e };
}

res read_string(const char * s) {
    res_string_view i = internal_read_string(s);
    return res { py::bytes(i.obj), i.next };
}

res read_value(const char * s) {
    s = skip_whitespace(s);
    switch (*s) {
        case 0:
            break;
        case '{':
            return read_object(s);
        case '[':
            return read_array(s);
        case 't':
            return read_true(s);
        case 'f':
            return read_false(s);
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
        case '-':
            return read_number(s);
        case '"':
            return read_string(s);
        default:
            break;
    }
    error("expected value");
}

py::object parse_value(const char * s) {
    res r = read_value(s);
    _assert(r.next && skip_whitespace(r.next)[0] == 0);
    return r.obj;
}

py::object parse_(std::string & x) {
    return parse_value(x.c_str());
}

#ifdef STANDALONE
int main() {
    py::scoped_interpreter guard{};
    std::string input;
    std::cin >> input;
    try {
        py::object res = parse_(input);
        py::print(res);
    } catch (pybind11::value_error) {
        py::print("<error>");
    }
}
#else

PYBIND11_MODULE(numjson, m) {
    m.doc() = "numjson module";
    m.def("parse", &parse_, "main entrypoint");
}

#endif
