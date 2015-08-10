/**\file
 *
 * \copyright
 * Copyright (c) 2015, Magnus Achim Deininger <magnus@ef.gy>
 * \copyright
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * \copyright
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * \copyright
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * \see Documentation: https://ef.gy/documentation/prometheus-client-cpp
 * \see Source Code: https://github.com/jyujin/prometheus-client-cpp
 * \see Licence Terms: https://github.com/jyujin/prometheus-client-cpp/COPYING
 */

#if !defined(PROMETHEUS_HTTP_H)
#define PROMETHEUS_HTTP_H

#include <ef.gy/http.h>
#include <prometheus/metric.h>

namespace prometheus {
std::string text(const collector::base &c) {
  std::string reply;
  if (c.type != "") {
    reply += "# TYPE " + c.name + " " + c.type + "\n";
  }
  if (c.help != "") {
    reply += "# HELP " + c.name + " " + c.help + "\n";
  }
  reply += c.name;
  if (c.label.size() > 0) {
    bool first = true;
    reply += "{";
    for (const auto &l : c.label) {
      if (first) {
        first = false;
      } else {
        reply += ",";
      }
      reply += l.first + "=\"" + l.second + "\"";
    }
    reply += "{";
  }
  reply += " " + c.value() + "\n";
  return reply;
}

template <class transport>
static bool http(typename efgy::net::http::server<transport>::session &session,
                 collector::registry &reg) {
  std::string reply = "";

  for (const auto &c : reg.collectors) {
    reply += text(*c);
  }

  session.reply(200, "Content-Type: text/plain; version=0.0.4\n", reply);

  return true;
}

template <class transport>
static bool
commonHTTP(typename efgy::net::http::server<transport>::session &session,
           std::smatch &) {
  return http<transport>(session, collector::registry::common());
}

}

#endif
