/* Prometheus metric types.
 *
 * See also:
 * * Documentation: https://ef.gy/documentation/prometheus-client-cpp
 * * Source Code: https://github.com/ef-gy/prometheus-client-cpp
 * * Licence: https://github.com/ef-gy/prometheus-client-cpp/blob/master/COPYING
 *
 * @copyright
 * This file is part of a Prometheus CPP client, which is released as open
 * source under the terms of an MIT/X11-style licence, described in the COPYING
 * file.
 */

#if !defined(PROMETHEUS_METRIC_H)
#define PROMETHEUS_METRIC_H

#include <prometheus/collector.h>
#include <sstream>

namespace prometheus {
namespace metric {
template <typename T = long long>
class counter : public collector::base {
 public:
  counter(const std::string &pName,
          const std::vector<std::string> &pLabels = std::vector<std::string>(),
          collector::registry<collector::base> &reg =
              collector::registry<collector::base>::common(),
          const std::map<std::string, std::string> &pLabel =
              std::map<std::string, std::string>())
      : collector::base(pName, "counter", pLabels, reg, pLabel), val(0) {}

  virtual std::string value(void) const {
    std::ostringstream oss("");
    oss << val;
    return oss.str();
  }

  counter &inc(const T &v = 1) {
    val += v >= 0 ? v : 0;
    return *this;
  }

  counter &set(const T &v) {
    val = v > val ? v : val;
    return *this;
  }

  counter &labels(const std::vector<std::string> &labelValues) {
    const auto newLabels = applyLabels(labelValues);
    const auto ls = labelString(newLabels);
    if (!child[ls]) {
      child[ls] = new counter(name, labelNames, *this, newLabels);
    }
    return *((counter *)child[ls]);
  }

 protected:
  T val;
};

template <typename T = long long>
class gauge : public collector::base {
 public:
  gauge(const std::string &pName,
        const std::vector<std::string> &pLabels = std::vector<std::string>(),
        collector::registry<collector::base> &reg =
            collector::registry<collector::base>::common(),
        const std::map<std::string, std::string> &pLabel =
            std::map<std::string, std::string>())
      : collector::base(pName, "gauge", pLabels, reg, pLabel), val(0) {}

  virtual std::string value(void) const {
    std::ostringstream oss("");
    oss << val;
    return oss.str();
  }

  gauge &inc(const T &v = 1) {
    val += v;
    return *this;
  }

  gauge &dec(const T &v = 1) {
    val -= v;
    return *this;
  }

  gauge &set(const T &v) {
    val = v;
    return *this;
  }

  gauge &setToCurrentTime(void) {
    val = std::time(0);
    return *this;
  }

  gauge &labels(const std::vector<std::string> &labelValues) {
    const auto newLabels = applyLabels(labelValues);
    const auto ls = labelString(newLabels);
    if (!child[ls]) {
      child[ls] = new gauge(name, labelNames, *this, newLabels);
    }
    return *((gauge *)child[ls]);
  }

 protected:
  T val;
};

template <typename T = long long>
class histogram : public collector::base {
 public:
  histogram(const std::string &pName, const std::vector<std::string> &pLabels =
                                          std::vector<std::string>(),
            collector::registry<collector::base> &reg =
                collector::registry<collector::base>::common(),
            const std::map<std::string, std::string> &pLabel =
                std::map<std::string, std::string>())
      : collector::base(pName, "histogram", pLabels, reg, pLabel),
        count(pName, pLabels, *this, pLabel),
        sum(pName, pLabels, *this, pLabel),
        inf(pName, pLabels, *this, pLabel) {
    inf.label["le"] = "+Inf";
  }

  histogram &labels(const std::vector<std::string> &labelValues) {
    const auto newLabels = applyLabels(labelValues);
    const auto ls = labelString(newLabels);
    if (!child[ls]) {
      child[ls] = new histogram(name, labelNames, *this, newLabels);
    }
    return *((histogram *)child[ls]);
  }

  counter<T> &bucket(const T &val) const {
    for (const auto &b : buckets) {
      if (val <= b.first()) {
        return *b.second();
      }
    }

    return inf;
  }

  histogram &observe(const T &val) {
    bucket(val).inc();
    count.inc(val);
    sum.inc();
    return *this;
  }

 protected:
  counter<T> count;
  gauge<T> sum;

  std::map<T, counter<T> *> buckets;
  counter<T> inf;
};
}

static bool setDefaultMetrics(collector::registry<collector::base> &reg) {
  (new metric::gauge<long long>("process_start_time_seconds", {}, reg))
      ->setToCurrentTime();

  return true;
}
}

#endif
